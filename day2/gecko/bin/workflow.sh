#!/bin/bash

FL=1000   # frequency limit

if [ $# != 5 ]; then
   echo " ==== ERROR ... you called this script inappropriately."
   echo ""
   echo "   usage:  $0 seqXName seqYName lenght similarity WL"
   echo ""
   exit -1
fi

{

dirNameX=$(readlink -f $1 | xargs dirname)
seqXName=$(basename "$1")
extensionX="${seqXName##*.}"
seqXName="${seqXName%.*}"

dirNameY=$(readlink -f $2 | xargs dirname)
seqYName=$(basename "$2")
extensionY="${seqYName##*.}"
seqYName="${seqYName%.*}"

BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

length=${3}
similarity=${4}
WL=${5} # wordSize
fixedL="1"
strand=${7}

mkdir intermediateFiles

mkdir intermediateFiles/${seqXName}-${seqYName}
mkdir results
mkdir intermediateFiles/dictionaries
mkdir intermediateFiles/hits

ln -s ${dirNameX}/${seqXName}.${extensionX} intermediateFiles/${seqXName}-${seqYName}
ln -s ${dirNameY}/${seqYName}.${extensionY} intermediateFiles/${seqXName}-${seqYName}

cd intermediateFiles/${seqXName}-${seqYName}

###############


echo "${BINDIR}/reverseComplement ${seqYName}.${extensionX} ${seqYName}-revercomp.${extensionY}"
${BINDIR}/reverseComplement ${seqYName}.${extensionX} ${seqYName}-revercomp.${extensionY}

if [[ ! -f ../dictionaries/${seqXName}.d2hP ]];	then
	echo "${BINDIR}/dictionary.sh ${seqXName}.${extensionX} &"
	${BINDIR}/dictionary.sh ${seqXName}.${extensionX} &		
fi
		
if [[ ! -f ../dictionaries/${seqYName}.d2hP ]];	then
	echo "${BINDIR}/dictionary.sh ${seqYName}.${extensionY} &"
	${BINDIR}/dictionary.sh ${seqYName}.${extensionY} &
fi
		
if [[ ! -f ../dictionaries/${seqYName}-revercomp.d2hP ]];	then
	echo "${BINDIR}/dictionary.sh ${seqYName}-revercomp.${extensionY} &"
	${BINDIR}/dictionary.sh ${seqYName}-revercomp.${extensionY} &
fi		

echo "Waiting for the calculation of the dictionaries"

for job in `jobs -p`
do
    wait $job
done


mv ${seqXName}.d2hP ../dictionaries/
mv ${seqXName}.d2hW ../dictionaries/
mv ${seqYName}.d2hP ../dictionaries/
mv ${seqYName}.d2hW ../dictionaries/
mv ${seqYName}-revercomp.d2hP ../dictionaries/
mv ${seqYName}-revercomp.d2hW ../dictionaries/
		
ln -s ../dictionaries/${seqXName}.d2hP .
ln -s ../dictionaries/${seqXName}.d2hW .

ln -s ../dictionaries/${seqYName}.d2hP .
ln -s ../dictionaries/${seqYName}.d2hW .

ln -s ../dictionaries/${seqYName}-revercomp.d2hP .
ln -s ../dictionaries/${seqYName}-revercomp.d2hW .

echo "${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}.${extensionY} ${length} ${similarity} ${WL} ${fixedL} f &"
${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}.${extensionY} ${length} ${similarity} ${WL} ${fixedL} f &

echo "${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}-revercomp.${extensionY} ${length} ${similarity} ${WL} ${fixedL} r &"
${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}-revercomp.${extensionY} ${length} ${similarity} ${WL} ${fixedL} r &

echo "Waiting for the comparisons"

for job in `jobs -p`
do
    wait $job
done

echo "${BINDIR}/combineFrags ${seqXName}-${seqYName}-sf.frags ${seqXName}-${seqYName}-revercomp-sr.frags ${seqXName}-${seqYName}.frags"
${BINDIR}/combineFrags ${seqXName}-${seqYName}-sf.frags ${seqXName}-${seqYName}-revercomp-sr.frags ${seqXName}-${seqYName}.frags

echo "${BINDIR}/getInfo ${seqXName}-${seqYName}.frags > ${seqXName}-${seqYName}.csv"
${BINDIR}/getInfo ${seqXName}-${seqYName}.frags > ${seqXName}-${seqYName}.csv.tmp
cat ${seqXName}-${seqYName}.frags.INF ${seqXName}-${seqYName}.csv.tmp > ${seqXName}-${seqYName}.csv
rm -rf ${seqXName}-${seqYName}.csv.tmp
	
if [[ -L "../../${seqXName}.fasta" ]]
then
	rm ../../${seqXName}.fasta
fi

if [[ -L "../../${seqYName}.fasta" ]]
then
	rm ../../${seqYName}.fasta
fi

mv ${seqXName}-${seqYName}.frags ../../results
mv ${seqXName}-${seqYName}.frags.INF ../../results
mv ${seqXName}-${seqYName}.frags.MAT ../../results
mv ${seqXName}-${seqYName}.csv ../../results

cd ..
rm -rf ${seqXName}-${seqYName}

} &> /dev/null
