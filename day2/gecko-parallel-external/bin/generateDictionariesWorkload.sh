#!/usr/bin/env bash

if [ $# != 1 ]; then
	echo "***ERROR*** Use: $0 fastaFilesExtension"
	exit -1
fi

array=()
x=0
EXT=$1

BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

for elem in $(ls -d ./*.$EXT | awk -F "/" '{print $NF}' | awk -F ".$EXT" '{print $1}')
do
	array[$x]=$elem
	x=`expr $x + 1`
done


for ((i=0 ; i < ${#array[@]} ; i++))
do
	seqX=${array[$i]}
	echo "${BINDIR}/dictionary.sh ${seqX}.$EXT"
done
