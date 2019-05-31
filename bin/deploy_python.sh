#!/usr/bin/env sh
set -e

mkdir -p python/rdrive/lib
/bin/cp -rf c/lib/libservo_api-32bit.dll python/rdrive/lib/libservo_api-32bit.dll
/bin/cp -rf c/lib/libservo_api-64bit.dll python/rdrive/lib/libservo_api-64bit.dll
bin/copy-sources.sh
bin/publish-python.sh