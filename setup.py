from setuptools import setup, Extension

core_extension = Extension(
    "pyroast._core",
    sources=["pyroast/_core.c"],
)

setup(
    ext_modules=[core_extension],
)