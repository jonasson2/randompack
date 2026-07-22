
import numpy as np, randompack
from threading import Thread

def worker(i, rng, results):
  results[i] = rng.normal(1000)

rng = [None]*4
rng[0] = randompack.Rng("ranlux++")
rng[0].seed(123)
results = [None]*3
threads = [None]*3
for i in range(3):
  rng[i+1] = rng[i].duplicate()
  rng[i+1].jump(32)
  threads[i] = Thread(target=worker, args=(i,rng[i+1],results))
  threads[i].start()
for thread in threads: thread.join() # wait till threads finish
print(np.array(results))
