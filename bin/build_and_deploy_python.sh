#!/usr/bin/env sh
set -e

# Collect Win binaries
mkdir -p python/rdrive/lib
/bin/cp -rf c/lib/libservo_api-32bit.dll python/rdrive/lib/libservo_api-32bit.dll
/bin/cp -rf c/lib/libservo_api-64bit.dll python/rdrive/lib/libservo_api-64bit.dll

# Collect sources
/bin/cp -rf c/include python/rdrive/include
/bin/cp -rf c/src python/rdrive/src
/bin/cp -rf c/core.mk python/rdrive/core.mk
/bin/cp -rf c/Makefile python/rdrive/Makefile
/bin/cp -rf version python/rdrive/version

# Make a Python package
cd python
pip install -U setuptools twine
python setup.py sdist

# Publish a package
twine upload --repository-url ${PYPI_URL} -u ${PYPI_USER} -p ${PYPI_PASSWORD} dist/*