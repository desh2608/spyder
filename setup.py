"""Setup script for the package."""

import setuptools
from Cython.Build import cythonize

VERSION = "0.1.0"

with open("README.md", "r") as file_object:
    LONG_DESCRIPTION = file_object.read()

SHORT_DESCRIPTION = """
A simple Python package to compute Diarization Error Rate (DER).
""".strip()

setuptools.setup(
    name="spyder",
    version=VERSION,
    author="Desh Raj",
    author_email="r.desh26@gmail.com",
    description=SHORT_DESCRIPTION,
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/markdown",
    url="https://github.com/desh2608/spyder",
    packages=setuptools.find_packages(),
    ext_modules=cythonize(["*.pyx"]),
    zip_safe=False,
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
    ],
)
