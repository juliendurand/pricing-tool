from distutils.core import setup, Extension
import glob
import os

source_files = glob.glob(os.path.join('src', '*.cpp'))

module = Extension(
    'pricingtool',
    sources=source_files,
    include_dirs=['.'],
    language='c++'
)

setup(
    name='PricingToolExtension',
    version='1.0',
    author='Julien Durand',
    description='This is an extension to perform actuarial glm',
    ext_modules=[module]
)
