#!/usr/bin/env bash
set -e
cd python
pip install -U setuptools twine
python setup.py sdist
twine upload --repository-url ${PYPI_URL} -u ${PYPI_USER} -p ${PYPI_PASSWORD} dist/*