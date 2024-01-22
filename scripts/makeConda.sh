#!/usr/bin/env bash
set -e
root=$(git rev-parse --show-toplevel)
cd $root

version=$(cat VERSION | tr -d '\n')
iBuild=mod-conda:$version
iExtract=mod-conda-extract-$version

#cd build && rm mod-*.tar.gz && make dist && cd ..
docker build -t $iBuild -f conda/build.Dockerfile --build-arg version=$version .
docker create --name $iExtract $iBuild
rm -rf ./conda/conda-bld
docker cp $iExtract:/opt/conda-bld ./conda/conda-bld
docker rm $iExtract
