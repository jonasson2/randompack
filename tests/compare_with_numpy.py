import numpy as np
bg = np.random.PCG64DXSM(seed=123)
stdict = bg.state['state'];
print("state:", stdict['state'])
print("incre:", stdict['inc'])
g = np.random.Generator(bg)
x = g.integers(0, 2**64, size=3, dtype=np.uint64)
print('draw0:', x[0])
print('draw1:', x[1])
print('draw2:', x[2])
