%%calculate time-optimal movement with acceleration and velocity limits
% ps - start position
% pf - final position
% vm - max. velocity
% am - max. acceleration

function pt = move_to(ps, pf, vm, am)
    dir = sign(pf - ps);
    d = 3/4 * vm^2 / am * dir;

    if abs(2 * d) >= abs(pf - ps)
        d = 0.5 * abs(pf - ps);
        vm = 2 * sqrt(am * d / 3);
        d = d * dir;
    end
    tc = (pf - ps - 2 * d) / (dir * vm);
    ta = abs(2 * d / vm);

    pt.a = [0 0 0 0];
    pt.p = [ps (ps+d) (pf-d) pf];
    pt.v = [0 (dir*vm) (dir*vm) 0];
    pt.t = cumsum([0 ta tc ta]);
end
