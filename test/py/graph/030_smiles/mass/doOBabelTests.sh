#!/usr/bin/env bash
# This script is based on the cansmiles_shuffle.sh script in Open Babels test suite

niter=${1:-20}
echo "$0: $niter shuffles per molecule";

n=0
errors=0
logfile=log.txt
tmpfile=tmp.smi
canfile=tmp.can

unset xpg_echo

rm -f smiles.py

cat nci.smi | while read -r smiles ; do
	# Generate N random SMILES for this molecule
	echo "Processing $smiles"
	smiles=$(echo $smiles | sed 's/[0-9]*$//' \
		| sed -e 's/\\/-/g' -e 's/\//-/g'						\
		)
	n=$(expr $n + 1)
	(
	i=0
	while test $i -lt $niter ; do
		echo $smiles
		i=`expr $i + 1`
	done;
	) | obabel -i smi -xC -o smi 2> $logfile | sed 's/[[:space:]]*$//' > $tmpfile
	sed -i 's/\(.*\)/smiles("\1")/' $tmpfile

	# Canonicalize them: They should all come out the same
	mod nopost -f $tmpfile \
		-e "with open('out/_test.smi', 'w') as f:" \
		-e "	for a in inputGraphs:" \
		-e "		f.write(a.smiles)" \
		-e '		f.write("\n")' &> /dev/null
	if test $? -ne 0; then
		echo "ERROR start log:"
		cat out/log.txt
		echo "ERROR stop log"
		break;
	fi
	sort -u out/_test.smi > $canfile

	# We should only have one SMILES now.  Any more (or zero) is an error.
	count=$(cat $canfile | wc -l)
	if [ "$count" -ne "1" ] ; then
		echo ""
		echo "error: Record $n: Got $count SMILES for one molecule:"
		cat $canfile
		echo "input smiles:"
		cat $tmpfile
		echo "output file:"
		cat out/*_test.smi
		echo "log file:"
		cat $logfile
		echo ""
		errors=`expr $errors + 1`
		break
	fi
	(
		cat $canfile
		echo $smiles
	) | obabel -i smi -o can 2>> $logfile | sort -u > $tmpfile
	newcount=$(wc -l $tmpfile)
	if [ "$count" -ne "1" ]; then
		echo ""
		echo "error 2: Record $n: not the same output molecule as input molecule:"
		echo -n "input: "
		echo $smiles | obabel -i smi -o can 2> /dev/null
		echo -n "output: "
		cat $canfile
		errors=$(expr $errors + 1)
	fi;
	echo "$smiles" | obabel -i smi -xC -o smi 2> /dev/null | sed 's/[[:space:]]*$//' | \
		sed 's/\(.*\)/smiles("\1")/' >> smiles.py
done

echo ""
echo "$n SMILES tested, $errors errors."
rm $logfile $tmpfile $canfile
mod clean
