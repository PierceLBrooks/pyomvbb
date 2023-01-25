#!/bin/sh

git clean -dfX
git submodule update --init --recursive
python3 -m pip install wheel
python3 ./setup.py bdist_wheel
