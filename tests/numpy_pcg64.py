import numpy as np
bg = np.random.PCG64DXSM()
st = bg.state
st['state']['state'] = (1 << 96) + 42;
st['state']['inc'] = 123;
bg.state = st;
g = np.random.Generator(bg)
x = g.integers(0, 2**64, size=3, dtype=np.uint64)
print('draw0:', x[0])
print('draw1:', x[1])
print('draw2:', x[2])
