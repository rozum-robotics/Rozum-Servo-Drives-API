d = dlmread('data.csv', ',');
t = dlmread('task.csv', ',');


subplot(2, 1, 1);
plot(d(:, 3), d(:, 1));
hold on;
plot(t(:, 3), t(:, 1), 'o');
hold off;
grid on;
legend('Calculated position', 'PVT position');
xlabel('Time, ms');
ylabel('Position, deg');

subplot(2, 1, 2);
plot(d(:, 3), d(:, 2) / 100 / 60 * 360);
hold on;
plot(t(:, 3), t(:, 2), 'o');
hold off;
grid on;
legend('Calculated velocity', 'PVT velocity');
xlabel('Time, ms');
ylabel('Velocity, deg/s');

print ('PVT', '-dpng');