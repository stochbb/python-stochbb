#!/usr/bin/python
from setuptools import setup, Extension

setup(
  name = "stochbb",
  version = "1.0.1",
  description = "StochBB - Stochastic building blocks",
  long_description = """StochBB analyses networks of dependent random variables.""",
  url = "https://stochbb.github.io",
  author = "Hannes Matuschek",
  author_email = "hmatuschek@gmail.com",
  license = "GPL-2",
  classifiers = [
    "Development Status :: 4 - Beta",
    "Intended Audience :: Developers",
    ],
  ext_modules = [Extension("_stochbb",["python/stochbb.i"], swig_opts=["-c++"], 
                           include_dirs=["/usr/include/eigen3"],
                           extra_compile_args=["-std=c++11"])],
  py_modules = ["python/stochbb"])
