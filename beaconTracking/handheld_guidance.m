% handheld_guidance.m
% Simulate a handheld receiver guiding a person to a beacon using only
% unsigned power readings from two orthogonal antennas (parallel & perp).
% Logic:
%  1) Reverse‐lock (drop–count vs rolling‐avg, cooldown, one U‐turn held),
%  2) Steering via ratio = Bpar/Bperp: if ratio ≥ last_ratio keep turn_dir,
%     else flip turn_dir, then apply fixed‐angle turn.

%% === Constants & Grid Setup ===
mu0    = 4*pi*1e-7;           % Permeability (H/m)
m      = 1e-2;                % Beacon moment (A·m^2)
range  = 40;                  % Domain half‐width (m)
res    = 0.5;                 % Grid resolution (m)

[x,y] = meshgrid(-range:res:range, -range:res:range);
r     = sqrt(x.^2 + y.^2); r(r==0)=eps;
theta     = atan2(y,x);
Br    = mu0*m./(4*pi*r.^3).*(2*cos(theta));
Bt    = mu0*m./(4*pi*r.^3).*sin(theta);
BxF   = Br.*cos(theta) - Bt.*sin(theta);
ByF   = Br.*sin(theta) + Bt.*cos(theta);

%% === Parameters & State Init ===
params.hist_size     = 10;     % history length
params.min_valid_mag = 0.05;   % seed & ignore below
params.drop_steps    = 5;      % drops to trigger reverse
params.reverse_cd    = 20;     % steps in reverse before unlock
params.max_turn      = pi/6;   % fixed turn size (30°)
params.step_size     = 0.5;    % move per step

% Rolling‐history
state.history.buf = params.min_valid_mag * ones(params.hist_size,1);
state.history.idx = 1;
state.sum_history = params.min_valid_mag * params.hist_size;

% Reverse‐lock counters
state.fwd_drops    = 0;
state.reverse_lock = false;
state.cd_timer     = 0;

% Steering
state.turn_dir    = +1;        % +1=left, –1=right
state.last_ratio  = 1;         % seed ratio

%% === Initial Position & Heading ===
pos     = (rand(1,2)*2-1)*(range-5);
while norm(pos)<10
    pos = (rand(1,2)*2-1)*(range-5);
end
heading = rand(1,2); heading = heading/norm(heading);

% Seed history & ratio
Bx0 = interp2(x,y,BxF,pos(1),pos(2),'linear',0);
By0 = interp2(x,y,ByF,pos(1),pos(2),'linear',0);
mag0 = sqrt(Bx0^2 + By0^2);
[state, ~] = hist_update(state, mag0, params);
state.last_ratio = 1;  % arbitrary

%% === Logging & Figure ===
steps     = 300;
pos_log   = nan(steps,2);
Bpar_log  = nan(steps,1);
Bperp_log = nan(steps,1);
mag_log   = nan(steps,1);

figure('Position',[100 100 1400 500]);
% Flux & path
subplot(1,3,1);
magF = sqrt(BxF.^2 + ByF.^2);
quiver(x,y, BxF./magF, ByF./magF, .5,'k'); hold on;
p_dot = plot(NaN,NaN,'ro','MarkerFaceColor','r');
h_head= quiver(NaN,NaN,0,0,2,'b','LineWidth',2);
plot(0,0,'bs','MarkerSize',10,'LineWidth',2);
axis equal; title('Flux & Path'); xlabel('X'); ylabel('Y');
% Signals
subplot(1,3,2);
hP = plot(NaN,NaN,'-r','LineWidth',1.5); hold on;
hQ = plot(NaN,NaN,'-b','LineWidth',1.5);
hM = plot(NaN,NaN,'-k','LineWidth',2);
legend('B_{||}','B_{\perp}','|B|','Location','NW');
title('Signals'); xlabel('Step'); ylabel('Mag');
% Suggestion arrow
subplot(1,3,3);
h_sug = quiver(0,0,0,0,'LineWidth',2,'MaxHeadSize',2,'AutoScale','off');
axis([-1 1 -1 1]); axis off; title('Suggestion');

%% === Main Simulation Loop ===
for k = 1:steps
    % 1) Sample two‐antenna powers
    Bx = interp2(x,y,BxF,pos(1),pos(2),'linear',0);
    By = interp2(x,y,ByF,pos(1),pos(2),'linear',0);
    Bpar = abs(dot([Bx,By],heading));
    perp = [-heading(2),heading(1)];
    Bperp = abs(dot([Bx,By],perp));
    mag = sqrt(Bpar^2 + Bperp^2);

    % 2) Update history & get previous average
    [state, avg_prev] = hist_update(state, mag, params);

    % 3) Reverse‐lock logic
    if state.cd_timer > 0
        state.cd_timer = state.cd_timer - 1;
    elseif ~state.reverse_lock
        if mag < avg_prev
            state.fwd_drops = state.fwd_drops + 1;
        else
            state.fwd_drops = 0;
        end
        if state.fwd_drops >= params.drop_steps
            state.reverse_lock = true;
            state.cd_timer     = params.reverse_cd;
            state.fwd_drops    = 0;
        end
    else
        % exit reverse mode when cooldown expires
        state.reverse_lock = false;
    end

    % 4) Steering via ratio if not reversing, else U‐turn
    if state.reverse_lock
        turn = pi;
    else
        ratio = Bpar / max(eps, Bperp);
        if ratio < state.last_ratio
            state.turn_dir = -state.turn_dir;
        end
        state.last_ratio = ratio;
        turn = state.turn_dir * params.max_turn;
    end

    % 5) Apply turn & move
    heading = rotate_vector(heading, turn);
    heading = heading / norm(heading);
    pos = pos + params.step_size * heading;
    pos = max(min(pos,range),-range);

    % 6) Log & plot
    pos_log(k,:)  = pos;
    Bpar_log(k)   = Bpar;
    Bperp_log(k)  = Bperp;
    mag_log(k)    = mag;

    subplot(1,3,1);
    set(p_dot,'XData',pos(1),'YData',pos(2));
    set(h_head,'XData',pos(1),'YData',pos(2), ...
               'UData',heading(1),'VData',heading(2));
    plot(pos_log(1:k,1),pos_log(1:k,2),'r-');

    subplot(1,3,2);
    set(hP,'XData',1:k,'YData',Bpar_log(1:k));
    set(hQ,'XData',1:k,'YData',Bperp_log(1:k));
    set(hM,'XData',1:k,'YData',mag_log(1:k));

    subplot(1,3,3);
    if state.reverse_lock
        vec = [0,-1];
    elseif state.turn_dir > 0
        vec = [-1,0];
    else
        vec = [1,0];
    end
    set(h_sug,'UData',vec(1),'VData',vec(2));

    drawnow; pause(0.03);

    % 7) Stop when close to beacon (within 1 m)
    if norm(pos) < 1
        disp("Beacon reached!");
        break;
    end
end

%% === Helper: rolling‐history update ===
function [st, avg_prev] = hist_update(st, mag, p)
    avg_prev = st.sum_history / p.hist_size;
    old = st.history.buf(st.history.idx);
    st.history.buf(st.history.idx) = mag;
    st.sum_history = st.sum_history - old + mag;
    st.history.idx = st.history.idx + 1;
    if st.history.idx > p.hist_size
        st.history.idx = 1;
    end
end

%% === Helper: rotate vector by theta ===
function v2 = rotate_vector(v, theta)
    R  = [cos(theta), -sin(theta); sin(theta), cos(theta)];
    v2 = (R * v')';
end
