%%calculate coeeficients for movement from 'ps' to 'pf'
% pf - final position
% ps - start position
% dt - time delta

function a = trj_coeff1(pf, ps, dt)
  if dt == 0
    a(1:6) = 0;
  else
  a(1) = ps;
  a(2) = (pf - ps) / dt;
  a(3) = 0;
  a(4) = 0;
  a(5) = 0;
  a(6) = 0;
  end
end