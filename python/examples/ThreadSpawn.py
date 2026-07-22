
import numpy as np, randompack
from threading import Thread

def worker(i, results):
  rng = randompack.Rng()
  rng.seed(123, spawn_key=[i])
  results[i] = rng.normal(1000)

results = [None]*3
threads = [None]*3
for i in range(3):
  threads[i] = Thread(target=worker, args=(i,results))
  threads[i].start()
for thread in threads: thread.join() # wait till threads finish
print(np.array(results))
