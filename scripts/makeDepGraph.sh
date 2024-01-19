#!/usr/bin/env bash
root=$(git rev-parse --show-toplevel)
res=$?
if [ $res -ne 0 ]; then
	exit $res
fi
cd $root
python3 scripts/printDepGraph.py > depGraph.dot
dot -Tpdf -o depGraph.pdf depGraph.dot
