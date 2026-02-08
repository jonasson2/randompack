import sys
from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy as np

extra_link_args = []
if sys.platform == "darwin":
    extra_link_args += ["-framework", "Accelerate"]

extensions = [
    Extension(
        "randompack._core",
        sources=["randompack/_core.pyx", "src/randompack.c"],
        include_dirs=[np.get_include(), "src"],
        extra_link_args=extra_link_args,
    )
]

setup(
    name="randompack",
    version="0.1.0",
    packages=["randompack"],
    ext_modules=cythonize(extensions, language_level="3"),
)
