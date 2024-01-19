#!/usr/bin/env bash

function handleFile {
	f=$1
	shift
	if test "x$1" = "x-p"; then
		mod=$2
		shift
		shift
	else
		mod="mod"
	fi
	dir=$(dirname $f)
	base=$(basename $f)
	echo "Executing $f"
	cd $dir
	$mod -f $base "$@"
	res=$?
	if [ $res -ne 0 ]; then
		echo "Execution of $f failed: $res"
		echo "pwd: $(pwd)"
		exit $res
	fi
	mod clean
}

origDir=$(pwd)

function doIt {
	for f in $(find . -name "*.py" | sort -n); do
		cd $origDir
		handleFile $f "$@"
	done
}

time doIt "$@"
