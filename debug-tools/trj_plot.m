% trajaectory visualization

dt = 1e-3;

%% generate a set of movements
pt = move_to(0, 100, 40, 100);
pt = ptcat(pt, move_to(pt.p(end), 0, 40, 10));
pt = ptcat(pt, move_to(pt.p(end), -50, 10, 50));
pt = ptcat(pt, move_to(pt.p(end), 0, 20, 10));

%% interpolate trajectory
[t, q] = qupsample(pt, dt);

%% visualize results

% visualize position
subplot(3, 1, 1); 
plot(t, q); 
ylabel('position');
xlabel('time,s');
hold on;
plot(pt.t, pt.p, 'x');
plot(pt.t, pt.p, 'g');
hold off;
grid;

% visualize velocity
subplot(3, 1, 2);
dq = [0 diff(q)]/dt;
plot(t, dq); 
ylabel('velocity');
xlabel('time,s');
hold on;
plot(pt.t, pt.v, 'x');
hold off;
grid;

% visualize acceleration
subplot(3, 1, 3);
ddq = [0 diff(dq)]/dt;
plot(t, ddq);
hold on;
plot(pt.t, pt.a, 'x');
hold off;
ylabel('acceleration');
xlabel('time,s');
grid;


