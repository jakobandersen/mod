#!/usr/bin/env bash
# This should be executed in the build dir with the path to lcov as first arg.
for v in SRC LCOV GENHTML PROJECT_NAME PROJECT_VERSION; do
	if [ "x${!v}" = "x" ]; then
		echo "Missing $v variable."
		exit 1
	fi
done
mkdir -p cov
cd cov
$LCOV -c -i -d ../ -o base.cov
$LCOV -e base.cov "$SRC/*" -o base.filt.cov
cp base.filt.cov merged.filt.cov
find . -iname "*.all.cov" | while read f; do
	name=${f%.all.cov}
	$LCOV -e $f "$SRC/*" -o $name.filt.cov
	# overall merged
	$LCOV -a merged.filt.cov -a $name.filt.cov -o merged2.cov
	mv merged2.cov merged.filt.cov
	# just this one
	$LCOV -a base.filt.cov -a $name.filt.cov -o merged2.cov
	mv merge2.cov $name.filt.cov
	mkdir -p ${name}_html
	$GENHTML --show-details --title "$PROJECT_NAME $PROJECT_VERSION, $name" \
		-o ${name}_html $name.filt.cov
done
mkdir -p html
$GENHTML --show-details --title "$PROJECT_NAME $PROJECT_VERSION" -o html merged.filt.cov
