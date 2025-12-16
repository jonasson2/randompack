import numpy as np

bg = np.random.Philox()
state = bg.state;
state['state']['key'] = [1, 2];
state['state']['counter'] = [1, 2, 3, 4];
bg.state = state;

g = np.random.Generator(bg)
x = g.integers(0, 2**64, size=3, dtype=np.uint64)
print('draw0:', x[0])
print('draw1:', x[1])
print('draw2:', x[2])
print('counter:', state['state']['counter'])
print('key:', state['state']['key'])
