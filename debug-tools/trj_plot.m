% trajaectory visualization

dt = 1e-3;

%% generate a set of movements
%pt.t = [0 1];
%pt.p = [0 60];
%pt.v = [0 0];
%pt.a = [0 0];

pt = move_to(0, 100, 30, 250);
pt = ptcat(pt, move_to(pt.p(end), 0, 40, 100));
pt = ptcat(pt, move_to(pt.p(end), -250, 100, 150));
pt = ptcat(pt, move_to(pt.p(end), 0, 200, 1500));


[t, q] = qupsample(pt, single(dt));
dq = diff(q)/dt;
dq = 0.5*([dq(1) dq] + [dq dq(end)]);

Nd = 1;

pt.p = zeros(1, length(t)/Nd);
pt.v = zeros(1, length(t)/Nd);
pt.a = zeros(1, length(t)/Nd);
pt.t = zeros(1, length(t)/Nd);
pt.o = zeros(1, length(t)/Nd) + 3;

for k = 1:length(pt.t)
  pt.p(k) = int32(q(k*Nd) / 360 * 2^23);
  pt.v(k) = int32(dq(k*Nd) / 390 * 2^16);
  pt.t(k) = t(k*Nd);  
endfor

for k = 1:length(pt.t)
  pt.p(k) = single(pt.p(k)) / 2^23 * 360;
  pt.v(k) = single(pt.v(k)) / 2^16 * 390;
endfor


%% interpolate trajectory
[t, q] = qupsample(pt, single(dt));

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
dq = diff(q)/dt;
dq = 0.5*([dq(1) dq] + [dq dq(end)]);
plot(t, dq); 
ylabel('velocity');
xlabel('time,s');
hold on;
plot(pt.t, pt.v, 'x');
hold off;
grid;

% visualize acceleration
subplot(3, 1, 3);
ddq = diff(dq)/dt;
ddq = 0.5*([ddq(1) ddq] + [ddq ddq(end)]);
plot(t, ddq);
hold on;
plot(pt.t, pt.a, 'x');
hold off;
ylabel('acceleration');
xlabel('time,s');
grid;


