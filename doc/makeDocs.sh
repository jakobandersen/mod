#!/usr/bin/env bash

if [ "x$1" = "xclean" ]; then
    rm -rf build doctrees source/reference
    exit 0
fi

topSrcDir=${1:-..}

function outputRST {
	mkdir -p $topSrcDir/doc/source/$(dirname $1)
	cat | sed "s/	/    /g" > $topSrcDir/doc/source/$1.rst
}

function filterCPP {
	cat | awk '
BEGIN {
	inClass = 0
	lineNum = 0
	inNestedClass = 0
	nestedLineNum = 0
	lastClass = ""
	lastNested = ""
}
{
	if($0 ~ /^[\t]*\/\/ rst:/) {
		sub(/^[\t]*\/\/ rst:[ 	]?/, "")
		if(inClass)
			if(inNested)
				nestedRST[nestedLineNum++] = $0
			else
				normalRST[lineNum++] = $0
		else
			print
	} else if($0 ~ /^[\t]*\/\* rst:/) {
		sub(/^[\t]*\/\* rst: ?/, "")
		sub(/\*\/[\t]*\\?/, "")
		if(inClass)
			if(inNested)
				nestedRST[nestedLineNum++] = $0
			else
				normalRST[lineNum++] = $0
		else
			print
	} else if($0 ~ /^[\t]*\/\/ rst-class:/) {
		sub(/^[\t]*\/\/ rst-class: /, "")
		lastClass = $0
		sub(/ : .*/, "", lastClass)
		print ""
		print "Class ``" lastClass "``"
		print "--------------------------------------------------------------------------------------------------------------------------------"
		print ""
		print ".. class:: " $0
		print "    "
	} else if($0 ~ /^[\t]*\/\/ rst-class-start:/) {
		inClass	= 1
		print ""
		print "Synopsis"
		print "^^^^^^^^"
		print ""
		print ".. alias:: " lastClass
		print "	:maxdepth: 2"
		print "	:noroot:"
		print ""
	} else if($0 ~/^[\t]*\/\/ rst-class-end:/) {
		inClass = 0
		if(lineNum > 0) {
			print ""
			print "Details"
			print "^^^^^^^"
			print ""
			print ".. cpp:namespace-push:: " lastClass
			print ""
			for(i = 0; i < lineNum; i++) print normalRST[i]
			print ""
			print ".. cpp:namespace-pop::"
			print ""
			lineNum = 0
		}
		if(nestedLineNum > 0) {
			for(i = 0; i < nestedLineNum; i++)
				print nestedRST[i]
			print ""
			nestedLineNum = 0
		}
	} else if($0 ~/^[\t]*\/\/ rst-nested:/) {
		sub(/^[\t]*\/\/ rst-nested: /, "")
		if(!inClass) {
			print "Nested class outside class!" | "cat 1>&2"
			exit 1
		}
		inNested = 1
		nestedRST[nestedLineNum++] = ""
		nestedRST[nestedLineNum++] = "Class ``" $0 "``"
		nestedRST[nestedLineNum++] = "---------------------------------------------------------------"
		nestedRST[nestedLineNum++] = ""
		nestedRST[nestedLineNum++] = ".. class:: " $0
		nestedRST[nestedLineNum++] = ""
		lastNested = $0
	} else if($0 ~/^[\t]*\/\/ rst-nested-start:/) {
		nestedRST[nestedLineNum++] = ""
	} else if($0 ~/^[\t]*\/\/ rst-nested-end:/) {
		nestedRST[nestedLineNum++] = ""
		inNested = 0
	} else if($0 ~/^[\t]*\/\/ rst/) {
		print "Unknown rst command in line " NR ":" | "cat 1>&2"
		print $0 | "cat 1>&2"
		exit 1
	}
}
'
}

function getModHeaders {
	find $topSrcDir/libs/libmod/src/mod -iname "*.hpp" \
		| grep -v -e "/lib/"  \
		| grep -v -e "/internal/"  \
		| sed -e "s/.*\/src\/mod\///" -e "s/\.hpp$//" \
		| sort
}

function getPyModCPPs {
	function raw {
		find $topSrcDir/libs/pymod/src/mod/py -iname "*.cpp" \
			| sed -e "s!.*/libs/pymod/src/mod/py/!!" -e "s/\.cpp$//" \
			| sort \
			| grep -v Module | grep -v Collections | grep -v Function
	}
	raw | while read f; do
		fFull=$topSrcDir/libs/pymod/src/mod/py/$f.cpp
		grep "rst:" $fFull &> /dev/null
		if [ $? -ne 0 ]; then
			continue
		fi
		echo $f
	done;
}

function makeLibMod {
	function data {
		local f=$1
		echo ".. _cpp-$f:"
		echo ""
		echo "**********************************************************"
		echo "$f.hpp"
		echo "**********************************************************"
		cat << "EOF"
.. default-domain:: cpp
.. default-role:: cpp:expr

.. py:currentmodule:: mod
.. cpp:namespace:: mod

EOF
		fFull=$topSrcDir/libs/libmod/src/mod/$f.hpp
		cat $fFull | filterCPP
		if [ $? -ne 0 ]; then
			echo "Error in $fFull" 1>&2
			return 1
		fi
	}
	for f in $(getModHeaders); do
		data $f | outputRST libmod/$f
		if [ ${PIPESTATUS[0]} -ne 0 ]; then
			return 1
		fi
	done
	function getFolders {
		getModHeaders \
			| grep "/" | sed "s/\/.*//" | uniq
	}
    function folderToc {
        echo $1
        cat << "EOF"
==============================================================================

.. toctree::
   :maxdepth: 1

EOF
        getModHeaders | grep "^$1" | sed -e "s/^$1\///" -e "s/^/   /"
    }
    for f in $(getFolders); do
        folderToc $f | outputRST libmod/$f/index
    done
	function dataToc {
		echo ".. toctree::"
		echo "   :maxdepth: 1"
		echo ""
		getModHeaders | grep -v "/" | sed 's/^/   /'
		echo ""
		echo ".. toctree::"
		echo "   :maxdepth: 2"
		echo ""
		getFolders | sed 's/$/\/index/' | sed 's/^/   /'
	}
	dataToc | outputRST libmod/Toc
	if [ ${PIPESTATUS[0]} -ne 0 ]; then
		return 1
	fi
}

function makePyMod {
	function data {
		local f=$1
		echo ".. _py-$f:"
		echo ""
		echo "**********************************************************"
		echo "$f"
		echo "**********************************************************"
		cat << "EOF"
.. default-domain:: py

.. py:currentmodule:: mod
.. cpp:namespace:: mod

EOF
		fFull=$topSrcDir/libs/pymod/src/mod/py/$f.cpp
		cat $fFull | filterCPP
		if [ $? -ne 0 ]; then
			echo "Error in $fFull" 1>&2
			return 1
		fi
	}
	for f in $(getPyModCPPs); do
		data $f | outputRST pymod/$f
		if [ ${PIPESTATUS[0]} -ne 0 ]; then
			return 1
		fi
	done
	function getFolders {
		getPyModCPPs \
			| grep "/" | sed "s/\/.*//" | uniq
	}
    function folderToc {
        echo $1
        cat << "EOF"
==============================================================================

.. toctree::
   :maxdepth: 1

EOF
        getPyModCPPs | grep "^$1" | sed -e "s/^$1\///" -e "s/^/   /"
    }
    for f in $(getFolders); do
        folderToc $f | outputRST pymod/$f/index
    done
	function dataToc {
		echo ".. toctree::"
		echo "   :maxdepth: 1"
		echo ""
		echo "   protocols"
		getPyModCPPs | grep -v "/" | sed 's/^/   /'
		echo ""
		echo ".. toctree::"
		echo "   :maxdepth: 2"
		echo ""
		getFolders | sed 's/$/\/index/' | sed 's/^/   /'
	}
	dataToc | outputRST pymod/Toc
}

function makeExamples {
	rm -rf $topSrcDir/doc/source/_static/examples
	mkdir -p $topSrcDir/doc/source/_static/examples
	cp -a $topSrcDir/examples/libmod_cmake $topSrcDir/doc/source/_static/examples/
	cp -a $topSrcDir/examples/pymod_extension $topSrcDir/doc/source/_static/examples/
	cp -a $topSrcDir/examples/py $topSrcDir/doc/source/_static/examples/
	$topSrcDir/scripts/makePyExamples.py $topSrcDir rst
}

makeLibMod || exit 1
makePyMod || exit 1
makeExamples || exit 1
