% handheld_guidance.m
% Simulate a handheld receiver guiding a person to a beacon.
% All signal‐processing and decision logic live in the helper function.
% Plots are configured so XData/YData vector sizes always match.

%% === Constants and Grid Setup ===
mu0    = 4*pi*1e-7;           % Permeability of free space (H/m)
m      = 1e-2;                % Magnetic moment of the beacon (A·m^2)
range  = 40;                  % Half-width of the square simulation area (m)
res    = 0.5;                 % Grid resolution (m)

% Create a 2D grid of (x,y) points spanning [-range, +range]
[x,y]  = meshgrid(-range:res:range, -range:res:range);

% Compute radial distance r and angle theta at each grid point
r = sqrt(x.^2 + y.^2);
r(r==0) = eps;                % Avoid divide-by-zero at the origin
theta  = atan2(y,x);          % Angle relative to x-axis

% Compute magnetic field components in polar form (Br, Bt)
Br = mu0*m ./ (4*pi*r.^3) .* (2 .* cos(theta));
Bt = mu0*m ./ (4*pi*r.^3) .* sin(theta);

% Convert polar components to Cartesian field vectors (BxF, ByF)
BxF = Br .* cos(theta) - Bt .* sin(theta);
ByF = Br .* sin(theta) + Bt .* cos(theta);

%% === Initialize Person Position & State ===
% Randomly place the “person” away from the beacon (>10 m from origin)
pos = (rand(1,2)*2-1) * (range-5);
while norm(pos) < 10
    pos = (rand(1,2)*2-1) * (range-5);
end

% Give the person a random initial heading (unit vector)
heading = rand(1,2);
heading = heading / norm(heading);

% Initialize state variables used by the guidance logic
state.last_Bmag = norm([ interp2(x,y,BxF,pos(1),pos(2),'linear',0), interp2(x,y,ByF,pos(1),pos(2),'linear',0) ]);
state.fwd_drop_count   = 0;    % Count of consecutive signal drops moving forward
state.rev_drop_count   = 0;    % Count of consecutive signal drops moving backward
state.reverse_lock     = false;% Whether we’re in “reverse” mode
state.reverse_cd_timer = 0;    % Cooldown timer before allowing another reverse

% Parameters controlling motion and turn logic
params.step_size   = 0.6;      % Distance moved each step (m)
params.max_turn    = pi/8;     % Maximum turn angle per step (rad)
params.drop_steps  = 3;        % How many drops to trigger a mode change
params.reverse_cd  = 5;        % Steps to wait before allowing reverse again
params.fwd_thresh  = pi/6;     % Angular threshold to keep going straight

%% === Prepare Logs & Figure ===
steps      = 200;                       % Number of simulation steps
Bmag_log   = zeros(steps,1);            % Log |B| over time
Bpar_log   = zeros(steps,1);            % Log parallel component over time
Bperp_log  = zeros(steps,1);            % Log perpendicular component over time
pos_log    = zeros(steps,2);            % Log positions over time
head_log   = zeros(steps,2);            % Log headings over time

% Create figure with three subplots: flux+path, signal strengths, suggestion
figure('Position',[50 50 1600 500]);

% --- Subplot 1: Flux Lines + Path ---
subplot(1,3,1);
mag = sqrt(BxF.^2 + ByF.^2);            % Magnitude for normalization
quiver(x,y, BxF./mag, ByF./mag, .5, 'k'); hold on;
p_dot = plot(NaN,NaN,'ro','MarkerFaceColor','r');  % Person marker
h_arr = quiver(NaN,NaN,0,0,2,'b','LineWidth',2);    % Heading arrow
plot(0,0,'bs','MarkerSize',10,'LineWidth',2);      % Beacon at origin
axis equal; title('Flux Lines + Path');
xlabel('X (m)'); ylabel('Y (m)');

% --- Subplot 2: Signal Strengths vs. Step ---
subplot(1,3,2);
h1 = plot(NaN,NaN,'-k','LineWidth',2); hold on;    % |B|
h2 = plot(NaN,NaN,'-r','LineWidth',1.5);          % B_{||}
h3 = plot(NaN,NaN,'-b','LineWidth',1.5);          % B_{\perp}
legend('|B|','B_{||}','B_{\perp}','Location','NW');
title('Signal Strengths'); xlabel('Step'); ylabel('Magnitude');

% --- Subplot 3: Receiver Suggestion Arrow ---
subplot(1,3,3);
h_sug = quiver(0,0,0,0,'LineWidth',2,'MaxHeadSize',2,'AutoScale','off');
axis([-1 1 -1 1]); axis off; title('Receiver Suggestion');

%% === Main Simulation Loop ===
for k = 1:steps
  % 1) Sample the field at the person’s current position
  Bx = interp2(x,y,BxF,pos(1),pos(2),'linear',0);
  By = interp2(x,y,ByF,pos(1),pos(2),'linear',0);

  % 2) Guidance helper returns new heading, suggestion, updated state,
  %    and decomposed B components
  [heading, sug, state, Bpar, Bperp, Bmag] = guidance( [Bx,By], heading, state, params );

  % 3) Move the person along the updated heading
  pos = pos + params.step_size * heading;

  % 4) Log data for plotting
  Bmag_log(k)  = Bmag;
  Bpar_log(k)  = Bpar;
  Bperp_log(k) = Bperp;
  pos_log(k,:) = pos;
  head_log(k,:)= heading;

  % 5) Update flux/path plot
  subplot(1,3,1);
    set(p_dot, 'XData', pos(1), 'YData', pos(2));
    set(h_arr, 'XData', pos(1), 'YData', pos(2), ...
               'UData', heading(1), 'VData', heading(2));
    plot(pos_log(1:k,1), pos_log(1:k,2), 'r-');

  % 6) Update signal strength plot
  subplot(1,3,2);
    set(h1, 'XData', 1:k, 'YData', Bmag_log(1:k));
    set(h2, 'XData', 1:k, 'YData', Bpar_log(1:k));
    set(h3, 'XData', 1:k, 'YData', Bperp_log(1:k));

  % 7) Update suggestion arrow based on helper’s string output
  switch sug
    case "Straight ahead", vec = [0,1];
    case "Turn left",      vec = [-1,0];
    case "Turn right",     vec = [1,0];
    case "Turn around",    vec = [0,-1];
  end
  subplot(1,3,3);
    set(h_sug, 'XData',0, 'YData',0, 'UData',vec(1), 'VData',vec(2));

  drawnow;
  pause(0.05);  % Small delay for real-time effect

  % Stop if beacon is “reached” (arbitrary threshold on |B|)
  if Bmag > 0.3
    disp("Beacon reached!");
    break;
  end
end

%% === guidance helper function ===
function [heading, sug, state, Bpar, Bperp, Bmag] = guidance( ...
        Bvec, heading, state, params )
  % Compute total field magnitude and detect if it's dropped since last step
  Bmag = norm(Bvec);
  drop = (Bmag < state.last_Bmag);
  state.last_Bmag = Bmag;

  % --- Forward/Reverse Mode Logic ---
  if ~state.reverse_lock
    % If forward, count consecutive drops
    if drop, state.fwd_drop_count = state.fwd_drop_count + 1;
    else     state.fwd_drop_count = 0;
    end
    % Lock into reverse mode after enough drops
    if state.fwd_drop_count >= params.drop_steps
      state.reverse_lock     = true;
      state.fwd_drop_count   = 0;
      state.reverse_cd_timer = params.reverse_cd;
    end
  else
    % In reverse mode: wait out cooldown, then count drops to unlock
    if state.reverse_cd_timer > 0
      state.reverse_cd_timer = state.reverse_cd_timer - 1;
    else
      if drop, state.rev_drop_count = state.rev_drop_count + 1;
      else     state.rev_drop_count = 0;
      end
      if state.rev_drop_count >= params.drop_steps
        state.reverse_lock    = false;
        state.rev_drop_count  = 0;
      end
    end
  end

  % --- Decompose field into components relative to heading ---
  perp = [-heading(2), heading(1)];     % Perpendicular unit vector
  Bpar  = dot(Bvec, heading);          % Parallel component
  Bperp = dot(Bvec, perp);             % Perpendicular component
  if state.reverse_lock
    % If in reverse mode, invert both components
    Bpar  = -Bpar;
    Bperp = -Bperp;
  end

  % --- Choose suggestion based on components ---
  if Bpar < 0
    sug = "Turn around";
  else
    ang = atan2(Bperp, Bpar);          % Angle off straight
    if abs(ang) <= params.fwd_thresh
      sug = "Straight ahead";
    elseif ang > 0
      sug = "Turn left";
    else
      sug = "Turn right";
    end
  end

  % --- Update heading vector according to suggestion ---
  switch sug
    case "Straight ahead", turn = 0;
    case "Turn left",      turn =  params.max_turn;
    case "Turn right",     turn = -params.max_turn;
    otherwise              turn = pi;    % Turn around
  end
  % Build rotation matrix and apply
  R = [cos(turn), -sin(turn); sin(turn), cos(turn)];
  heading = (R * heading')';
  heading = heading / norm(heading);    % Re-normalize to unit length
end
