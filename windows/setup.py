from distutils.core import setup
import py2exe

py2exe_options = {
  "compressed": 1,
  "optimize": 2,
  "bundle_files": 2}

setup(
  options = {"py2exe": py2exe_options},
  console = [{"script" : "../scratch2vsido.py"}],
  zipfile = None)
