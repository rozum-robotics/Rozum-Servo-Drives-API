%%calculate coeeficients for movement from 'ps' to 'pf'
% pf - final position
% vf - final velocity
% af - final acceleration
% ps - start position
% vs - start velocity
% as - start acceleration
% dt - time delta

function a = trj_coeff(pf, vf, af, ps, vs, as, dt)
  if dt == 0
    a(1:6) = 0;
  else
    a(1) = ps;
    a(2) = vs;
    a(3) = as / 2;
    a(4) = -(20 * ps - 20 * pf + 8 * dt * vf + 12 * dt * vs - af * dt^2 + 3 * as * dt^2) / (2 * dt^3);
    a(5) = (30 * ps - 30 * pf + 14 * dt * vf + 16 * dt * vs - 2 * af * dt^2 + 3 * as * dt^2) / (2 * dt^4);
    a(6) = -(12 * ps - 12 * pf + 6 * dt * vf + 6 * dt * vs - af * dt^2 + as * dt^2) / (2 * dt^5);
  end
end