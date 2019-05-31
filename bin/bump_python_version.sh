#!/usr/bin/env sh
set -e

python_version=`sed -n -e "s/^VERSION = '\(.*\)'.*/\1/p" python/setup.py`
api_version=`cat version`

if [ $api_version != $python_version ]
then
    echo "VERSIONS DON'T MATCH! Bumping from $python_version to $api_version"
    sed -i "s/^\(VERSION = '\).*\('.*\)$/\1$api_version\2/g" python/setup.py
fi



