#!/usr/bin/env bash

if [ $# != 4 ]; then
	echo "***ERROR*** Use: $0 L(200) S(40) K(32) fastaFilesExtension"
	exit -1
fi

L=$1
S=$2
WL=$3
EXT=$4

array=()
x=0

BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

for elem in $(ls -d ./*.$EXT | awk -F "/" '{print $NF}' | awk -F ".$EXT" '{print $1}')
do
	array[$x]=$elem
	x=`expr $x + 1`
done

for ((i=0 ; i < ${#array[@]} ; i++))
do
	for ((j=i ; j < ${#array[@]} ; j++))
	do
		if [ $i != $j ]; then
			seqX=${array[$i]}
			seqY=${array[$j]}
			echo "${BINDIR}/frags.sh ${seqX}.$EXT ${seqY}.$EXT $L $S $WL"
		fi
	done
done