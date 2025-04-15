% === Constants ===
mu0 = 4 * pi * 1e-7;
m = 1e-2;
range = 40;
res = 1;

[x, y] = meshgrid(-range:res:range, -range:res:range);
r = sqrt(x.^2 + y.^2); r(r == 0) = eps;
theta = atan2(y, x);

Br = mu0 * m ./ (4 * pi * r.^3) .* (2 * cos(theta));
Bt = mu0 * m ./ (4 * pi * r.^3) .* (sin(theta));

Bx_field = Br .* cos(theta) - Bt .* sin(theta);
By_field = Br .* sin(theta) + Bt .* cos(theta);

% === Random Drone Start ===
while true
    drone_pos = (rand(1,2) * 2 - 1) * (range - 5);
    if norm(drone_pos) > 10
        break;
    end
end

% === Initial Setup ===
heading = rand(1,2); heading = heading / norm(heading);
path = drone_pos;
last_B_mag = 0;
sniff_interval = 4;
steps_since_sniff = 0;
last_full_sniff_step = -Inf;

% === Logs ===
B_mag_log = [];
B_parallel_log = [];
B_perp_log = [];
step_log = [];
pos_log = drone_pos;

% === Plot Setup ===
figure('Position',[50 50 1400 500]);

subplot(1,2,1);
quiver(x, y, Bx_field ./ sqrt(Bx_field.^2 + By_field.^2), ...
    By_field ./ sqrt(Bx_field.^2 + By_field.^2), 0.5, 'k'); hold on;
drone_plot = plot(drone_pos(1), drone_pos(2), 'ro', 'MarkerSize', 8, 'MarkerFaceColor', 'r');
plot(0, 0, 'bo', 'MarkerSize', 10, 'LineWidth', 2);
axis equal;
title('Drone Path (Smoothed, Stable)');
xlabel('X (m)'); ylabel('Y (m)');

heading_plot = quiver(drone_pos(1), drone_pos(2), heading(1), heading(2), ...
    1.5, 'Color', [0.2 0.6 1], 'LineWidth', 2);

subplot(1,2,2); hold on;
h_mag = plot(NaN, NaN, '-k', 'LineWidth', 2);
h_par = plot(NaN, NaN, '-r', 'LineWidth', 1.5);
h_perp = plot(NaN, NaN, '-b', 'LineWidth', 1.5);
legend('|B|', 'B_{||}', 'B_{\perp}', 'Location', 'northwest');
title('Antenna Signal Strengths Over Time');
xlabel('Step'); ylabel('Signal Strength');

% === Motion Parameters ===
step_size = 0.6;
fov_half_angle = pi/8;
stop_threshold_B_mag = 0.3;
drop_threshold = 0.05;
signal_window = 3;
min_full_sniff_gap = 3;

% === Main Loop ===
for step = 1:1000
    % === Noisy field reading ===
    Bx = interp2(x, y, Bx_field, drone_pos(1), drone_pos(2), 'linear', 0);
    By = interp2(x, y, By_field, drone_pos(1), drone_pos(2), 'linear', 0);
    B_vec = [Bx, By];
    B_mag_noisy = norm(B_vec) * (1 + 0.05 * randn());

    % === Smooth comparison ===
    if isempty(B_mag_log)
        smoothed_B = B_mag_noisy;
    else
        recent_B = [B_mag_log(end-min(signal_window-1,length(B_mag_log))+1:end), B_mag_noisy];
        smoothed_B = mean(recent_B);
    end

    % === Sniffing logic ===
    drop_detected = (smoothed_B < last_B_mag * (1 - drop_threshold));
    do_full_sniff = (step == 1 || (drop_detected && (step - last_full_sniff_step >= min_full_sniff_gap)));
    do_sniff = do_full_sniff || (steps_since_sniff >= sniff_interval);

    if do_sniff
        candidate_positions = [];
        candidate_headings = [];
        candidate_scores = [];

        if do_full_sniff
            scan_angles = linspace(0, 2*pi, 8);
            last_full_sniff_step = step;
        else
            scan_angles = linspace(-fov_half_angle, fov_half_angle, 3);
            current_angle = atan2(heading(2), heading(1));
            scan_angles = mod(current_angle + scan_angles, 2*pi);
        end

        current_pos = drone_pos;

        for angle = scan_angles
            test_heading = [cos(angle), sin(angle)];
            test_pos = current_pos + step_size * test_heading;

            drone_pos = test_pos;
            heading = test_heading;

            Bx = interp2(x, y, Bx_field, drone_pos(1), drone_pos(2), 'linear', 0);
            By = interp2(x, y, By_field, drone_pos(1), drone_pos(2), 'linear', 0);
            B_vec_test = [Bx, By];
            B_mag_test = norm(B_vec_test) * (1 + 0.05 * randn());

            forward_bias = 0.02 * dot(test_heading, heading);
            score = B_mag_test + forward_bias;

            candidate_positions(end+1, :) = test_pos;
            candidate_headings(end+1, :) = test_heading;
            candidate_scores(end+1) = score;

            subplot(1,2,1);
            set(drone_plot, 'XData', drone_pos(1), 'YData', drone_pos(2));
            set(heading_plot, 'XData', drone_pos(1), 'YData', drone_pos(2), ...
                'UData', heading(1), 'VData', heading(2));
            plot([current_pos(1), drone_pos(1)], [current_pos(2), drone_pos(2)], 'Color', [0.6 0.6 0.6]);

            drawnow;
            pause(0.05);
            drone_pos = current_pos;
        end

        [~, best_idx] = max(candidate_scores);
        drone_pos = candidate_positions(best_idx, :);
        heading = candidate_headings(best_idx, :) / norm(candidate_headings(best_idx, :));
        steps_since_sniff = 0;
    else
        drone_pos = drone_pos + step_size * heading;
        steps_since_sniff = steps_since_sniff + 1;
    end

    % === Final field reading & logs ===
    Bx = interp2(x, y, Bx_field, drone_pos(1), drone_pos(2), 'linear', 0);
    By = interp2(x, y, By_field, drone_pos(1), drone_pos(2), 'linear', 0);
    B_vec = [Bx, By];
    B_mag_noisy = norm(B_vec) * (1 + 0.05 * randn());

    heading_perp = [-heading(2), heading(1)];
    B_parallel = dot(B_vec, heading);
    B_perp = dot(B_vec, heading_perp);

    B_mag_log(end+1) = B_mag_noisy;
    B_parallel_log(end+1) = B_parallel;
    B_perp_log(end+1) = B_perp;
    step_log(end+1) = step;
    pos_log(end+1, :) = drone_pos;
    path = [path; drone_pos];

    % === Update Plots ===
    set(drone_plot, 'XData', drone_pos(1), 'YData', drone_pos(2));
    set(heading_plot, 'XData', drone_pos(1), 'YData', drone_pos(2), ...
        'UData', heading(1), 'VData', heading(2));

    subplot(1,2,1);
    plot(path(:,1), path(:,2), 'r-', 'LineWidth', 1.5);

    subplot(1,2,2);
    set(h_mag, 'XData', step_log, 'YData', B_mag_log);
    set(h_par, 'XData', step_log, 'YData', B_parallel_log);
    set(h_perp, 'XData', step_log, 'YData', B_perp_log);

    drawnow;
    pause(0.05);

    if B_mag_noisy > stop_threshold_B_mag
        disp("Beacon located!");
        break;
    end

    last_B_mag = smoothed_B;
end
