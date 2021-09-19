#
# Zmey project
# Copyright (C) 2021 by Contributors <https://github.com/Tyill/zmey>
#
# This code is licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

from __future__ import absolute_import
import sys

from setuptools import find_packages

kwargs = {}
if "--inplace" in sys.argv:
    from distutils.core import setup
else:
    from setuptools import setup
    kwargs = {'install_requires': ['flask']}

with open("README.md", "r") as fh:
    long_description = fh.read()

__version__ = '1.0.1'

setup(
    name="zmeyweb",
    version=__version__,
    url="https://github.com/Tyill/zmey",
    packages=find_packages(),
    package_data={ 
      "zmeyweb": ["*.cng", "api/*.py", 
                 "html/*.html", "html/auth/*.html", "html/gui/*.html",
                 "static/*.ico", "static/css/*", "static/images/*", "static/js/*", "static/js/fonts/*"],
    },
    description="Application for schedule and monitor workflows",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="MIT",
    classifiers=[
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Intended Audience :: Developers',
        'Environment :: Web Environment',
        'Programming Language :: Python :: 3.6',
        'Topic :: System :: Monitoring',
    ],
    **kwargs)