#!/usr/bin/env sh
set -e
/bin/cp -rf c/include python/rdrive/include
/bin/cp -rf c/src python/rdrive/src
/bin/cp -rf c/core.mk python/rdrive/core.mk
/bin/cp -rf c/Makefile python/rdrive/Makefile
/bin/cp -rf version python/rdrive/version
