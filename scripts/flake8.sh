#!/usr/bin/env bash
# Create an "installation" of the Python parts by symlinking them into
# build/checkPython/mod, and then run the Python tools.
root=$(git rev-parse --show-toplevel)
res=$?
if [ $res -ne 0 ]; then
	exit $res
fi
cd $root
mkdir -p build/checkPython/mod
rm -f build/checkPython/mod/*
for f in $(ls libs/pymod/lib/mod); do
	ln -s -T ../../../libs/pymod/lib/mod/$f build/checkPython/mod/$f
done
ln -s -T ../../../libs/epim/lib/mod/epim build/checkPython/mod/epim

flake8 \
	--ignore W191,W503,E124,E127,E128 \
	--exclude "*/mod/__init__.py" \
	build/checkPython/mod
