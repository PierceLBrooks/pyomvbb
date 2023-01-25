git clean -dfX
git submodule update --init --recursive
python -m pip install wheel
python ./setup.py bdist_wheel
