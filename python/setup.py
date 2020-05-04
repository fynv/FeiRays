from setuptools import setup
from codecs import open
import os

here = os.path.abspath(os.path.dirname(__file__))

with open(os.path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
	name = 'FeiRays',
	version = '0.0.2',
	description = 'Vulkan based Monte-Carol Ray-tracing',
	long_description=long_description,
	long_description_content_type='text/markdown',  
	url='https://github.com/fynv/FeiRays',
	license='Anti 996',
	author='Fei Yang',
	author_email='hyangfeih@gmail.com',
	keywords='Vulkan pathtracer xorwow ray-tracing',
	packages=['FeiRays'],
	data_files=[("Fei", ["PyFeiRays.dll", "libPyFeiRays.so"])],
	install_requires = ['cffi','pillow'],	
)

