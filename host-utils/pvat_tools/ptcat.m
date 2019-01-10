%%concatenate PVAT points 'a' & 'b'

function pt = ptcat(a, b)
    pt.p = [a.p b.p];
    pt.v = [a.v b.v];
    pt.a = [a.a b.a];
    pt.t = [a.t b.t+a.t(end)];
end