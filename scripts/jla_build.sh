#!/usr/bin/env bash
export AS_RLIMIT=300000000
root=$(git rev-parse --show-toplevel)
cd $root
numThreads=2
prefix="../stage"
type="OptDebug"
while true; do
	case $1 in
	-j)
		if [ "x$2" = "x" ]; then
			echo "Missing argument for '$1'"
			exit 1
		fi
		numThreads=$2
		shift
		shift
		;;
	"--prefix="*)
		prefix=${1#--prefix=}
		shift
		;;
	"--type="*)
		type=${1#--type=}
		shift
		;;
	"")
		break
		;;
	*)
		echo "Unknown option '$1'"
		exit 1
	esac
done

args="-DCMAKE_BUILD_TYPE=$type"
args+=" -DBUILD_TESTING=on"
args+=" -DCMAKE_PREFIX_PATH=$HOME/programs"
args+=" -DBOOST_ROOT=$HOME/programs"
args+=" -DCMAKE_MODULE_LINKER_FLAGS='-flto=$numThreads'"
args+=" -DCMAKE_SHARED_LINKER_FLAGS='-flto=$numThreads'"
args+=" -DCMAKE_INSTALL_PREFIX=$prefix $@"

./bootstrap.sh                                  \
	&& rm -rf build && mkdir build              \
	&& cd build && cmake ../ $args
res=$?
if [ $res -ne 0 ]; then
	echo "Error during configuration"
	exit $res
fi
time make -j $numThreads                        \
	&& make install
res=$?
if [ $res -ne 0 ]; then
	echo "Error during installation"
	exit $res
fi
