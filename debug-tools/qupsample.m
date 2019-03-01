%%calculate trajectory using provided PVAT points
% pt - structure with PVAT points
% pt.p - position vector
% pt.v - velocity vector
% pt.a - acceleration vector
% pt.t - time vector
% dt - time step of final trajectory

function [t, q] = qupsample(pt, dt)
  t = 0:dt:max(pt.t);
  q = zeros(1, length(t));

  for k = 1:(length(pt.t)-1)
    if isfield(pt, 'o')
      if pt.o(k) == 1
        a = trj_coeff1(pt.p(k+1), pt.p(k), ...
          pt.t(k+1) - pt.t(k));
      elseif pt.o(k) == 3
        a = trj_coeff3(pt.p(k+1), pt.v(k+1), ...
          pt.p(k), pt.v(k), ...
          pt.t(k+1) - pt.t(k));
      else
        a = trj_coeff(pt.p(k+1), pt.v(k+1), pt.a(k+1), ...
          pt.p(k), pt.v(k), pt.a(k), ...
          pt.t(k+1) - pt.t(k));        
      end
    else
      a = trj_coeff(pt.p(k+1), pt.v(k+1), pt.a(k+1), ...
        pt.p(k), pt.v(k), pt.a(k), ...
        pt.t(k+1) - pt.t(k));         
    end

    ind = (t >= pt.t(k)) & (t <= pt.t(k+1));
    q(ind) = trj(a, t(ind) - pt.t(k));
  end
end