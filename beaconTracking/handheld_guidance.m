% handheld_guidance.m
% Simulate a handheld receiver guiding a person to a beacon using only
% the unsigned power readings from two orthogonal antennas:

%% === Constants & Grid Setup ===
mu0    = 4*pi*1e-7;           
m      = 1e-2;                
range  = 40;                  
res    = 0.5;                 

[x,y] = meshgrid(-range:res:range, -range:res:range);
r     = sqrt(x.^2 + y.^2); r(r==0)=eps;
theta     = atan2(y,x);
Br    = mu0*m./(4*pi*r.^3).*(2*cos(theta));
Bt    = mu0*m./(4*pi*r.^3).*sin(theta);
BxF   = Br.*cos(theta) - Bt.*sin(theta);
ByF   = Br.*sin(theta) + Bt.*cos(theta);

%% === Parameters & State Init ===
params.hist_size     = 10;
params.min_valid_mag = 0.05;
params.drop_steps    = 5;
params.reverse_cd    = 20;
params.fwd_thresh    = pi/12;
params.max_turn      = pi/6;
params.step_size     = 0.5;

% Rolling‐history buffer (seed below)
state.history.buf = zeros(params.hist_size,1);
state.history.idx = 1;
state.sum_history = 0;

% Reverse‐lock & drop counters
state.fwd_drops     = 0;
state.reverse_lock  = false;
state.cd_timer      = 0;
state.did_reverse   = false;

% Steering state
state.turn_dir      = +1;
state.last_ratio    = 1;

%% === Initial Position & Heading ===
pos     = (rand(1,2)*2-1)*(range-5);
while norm(pos) < 10
    pos = (rand(1,2)*2-1)*(range-5);
end
heading = rand(1,2); heading = heading/norm(heading);

% Seed history with first magnitude
Bx0 = interp2(x,y,BxF,pos(1),pos(2),'linear',0);
By0 = interp2(x,y,ByF,pos(1),pos(2),'linear',0);
mag0 = sqrt(Bx0^2 + By0^2);
state.history.buf(:) = mag0;
state.sum_history    = mag0 * params.hist_size;

%% === Logging & Figure ===
steps     = 300;
pos_log   = nan(steps,2);
Bpar_log  = nan(steps,1);
Bperp_log = nan(steps,1);
mag_log   = nan(steps,1);

figure('Position',[100 100 1400 500]);
subplot(1,3,1);
magF = sqrt(BxF.^2 + ByF.^2);
quiver(x,y,BxF./magF,ByF./magF,.5,'k'); hold on;
p_dot = plot(NaN,NaN,'ro','MarkerFaceColor','r');
h_head= quiver(NaN,NaN,0,0,2,'b','LineWidth',2);
plot(0,0,'bs','MarkerSize',10,'LineWidth',2);
axis equal; title('Flux & Path'); xlabel('X'); ylabel('Y');

subplot(1,3,2);
hP = plot(NaN,NaN,'-r','LineWidth',1.5); hold on;
hQ = plot(NaN,NaN,'-b','LineWidth',1.5);
hM = plot(NaN,NaN,'-k','LineWidth',2);
legend('B_{||}','B_{\perp}','|B|','Location','NW');
title('Signals'); xlabel('Step'); ylabel('Magnitude');

subplot(1,3,3);
h_sug = quiver(0,0,0,0,'LineWidth',2,'MaxHeadSize',2,'AutoScale','off');
axis([-1 1 -1 1]); axis off; title('Suggestion');

%% === Main Loop ===
for k = 1:steps
    % 1) Sample two‐antenna powers
    Bx   = interp2(x,y,BxF,pos(1),pos(2),'linear',0);
    By   = interp2(x,y,ByF,pos(1),pos(2),'linear',0);
    Bpar = abs(dot([Bx,By],heading));
    perp = [-heading(2), heading(1)];
    Bperp= abs(dot([Bx,By],perp));
    mag  = sqrt(Bpar^2 + Bperp^2);

    % 2) Update history & compute previous average
    avg_prev = state.sum_history / params.hist_size;
    old = state.history.buf(state.history.idx);
    state.history.buf(state.history.idx) = mag;
    state.sum_history = state.sum_history - old + mag;
    state.history.idx = state.history.idx + 1;
    if state.history.idx > params.hist_size
        state.history.idx = 1;
    end

    % 3) Reverse‐lock logic
    if ~state.reverse_lock
        if mag < avg_prev
            state.fwd_drops = state.fwd_drops + 1;
        else
            state.fwd_drops = 0;
        end
        if state.fwd_drops >= params.drop_steps
            state.reverse_lock = true;
            state.cd_timer     = params.reverse_cd;
            state.did_reverse  = true;
            state.fwd_drops    = 0;
        end
    else
        if state.cd_timer > 0
            state.cd_timer = state.cd_timer - 1;
        else
            state.reverse_lock = false;
        end
    end

    % 4) Steering decision
    if state.reverse_lock
        if state.did_reverse
            turn = pi;               % do U‐turn once
            state.did_reverse = false;
        else
            turn = 0;                % hold reversed heading
        end
        sug_vec = [0, -1];
    else
        ang = atan2(Bperp, Bpar);
        if abs(ang) <= params.fwd_thresh
            turn    = 0;
            sug_vec = [0, 1];
        else
            ratio = Bpar / max(eps, Bperp);
            if ratio < state.last_ratio
                state.turn_dir = -state.turn_dir;
            end
            state.last_ratio = ratio;
            if state.turn_dir > 0
                turn    =  params.max_turn;
                sug_vec = [-1, 0];
            else
                turn    = -params.max_turn;
                sug_vec = [ 1, 0];
            end
        end
    end

    % 5) Apply turn & move
    heading = rotate_vector(heading, turn);
    heading = heading / norm(heading);
    pos     = pos + params.step_size * heading;
    pos     = max(min(pos, range), -range);

    % 6) Log & plot
    pos_log(k,:) = pos;
    Bpar_log(k)  = Bpar;
    Bperp_log(k) = Bperp;
    mag_log(k)   = mag;
    subplot(1,3,1);
      set(p_dot,'XData',pos(1),'YData',pos(2));
      set(h_head,'XData',pos(1),'YData',pos(2),...
                 'UData',heading(1),'VData',heading(2));
      plot(pos_log(1:k,1),pos_log(1:k,2),'r-');
    subplot(1,3,2);
      set(hP,'XData',1:k,'YData',Bpar_log(1:k));
      set(hQ,'XData',1:k,'YData',Bperp_log(1:k));
      set(hM,'XData',1:k,'YData',mag_log(1:k));
    subplot(1,3,3);
      set(h_sug,'UData',sug_vec(1),'VData',sug_vec(2));

    drawnow; pause(0.03);

    % 7) Stop when close
    if norm(pos) < 1
        disp("Beacon reached!"); break;
    end
end

%% === rotate helper ===
function v2 = rotate_vector(v, theta)
    R  = [cos(theta), -sin(theta); sin(theta), cos(theta)];
    v2 = (R * v')';
end
