from setuptools import setup, find_packages
from glob import glob

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir

import sys

__version__ = "0.1.0"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension(
        "_spyder",
        sorted(glob("src/spyder/*.cc")),
        # Example: passing in the version to the compiled code
        define_macros=[("VERSION_INFO", __version__)],
    ),
]

install_requires = ["click>=7.0.0"]

setup(
    name="spyder",
    version=__version__,
    author="Desh Raj",
    author_email="r.desh26@gmail.com",
    url="https://github.com/desh2608/spyder",
    description="A simple Python package for fast DER computation",
    long_description="",
    package_dir={"": "src"},
    packages=find_packages("spyder"),
    ext_modules=ext_modules,
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    install_requires=install_requires,
    entry_points={"console_scripts": ["spyder=spyder.der:compute_der_from_rttm"]},
)
