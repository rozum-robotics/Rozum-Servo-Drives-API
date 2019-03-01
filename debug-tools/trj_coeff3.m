%%calculate coeeficients for movement from 'ps' to 'pf'
% pf - final position
% vf - final velocity
% ps - start position
% vs - start velocity
% dt - time delta

function a = trj_coeff3(pf, vf, ps, vs, dt)
  if dt == 0
    a(1:6) = 0;
  else
  a(1) = ps;
  a(2) = vs;
  a(3) = -(dt*vf + 2*dt*vs - 3*pf + 3*ps) / dt^2;
  a(4) = (dt*vf + dt*vs - 2*pf + 2*ps) / dt^3;
  a(5) = 0;
  a(6) = 0;
  end
end