%%calculate a piece of trajectory using coefficients 'a' & time vector 't'

function y = trj(a, t)
    y = a(1) + a(2)*t + a(3)*t.^2 + a(4)*t.^3 + a(5)*t.^4 + a(6)*t.^5;
end