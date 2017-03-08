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

seqXName=$(basename "$1")
extensionX="${seqXName##*.}"
seqXName="${seqXName%.*}"

seqYName=$(basename "$2")
extensionY="${seqYName##*.}"
seqYName="${seqYName%.*}"

BINDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

length=${3}
similarity=${4}
WL=${5} # wordSize

mkdir ${seqXName}-${seqYName}
mkdir frags

cd ${seqXName}-${seqYName}

ln -s ../${seqXName}.${extensionX} .
ln -s ../${seqYName}.${extensionY} .
ln -s ../reverses/${seqYName}-revercomp.${extensionY} .

# Link the previously calculated dictionaries. They must be calculated already
ln -s ../dictionaries/${seqXName}.d2hP .
ln -s ../dictionaries/${seqXName}.d2hW .

ln -s ../dictionaries/${seqYName}.d2hP .
ln -s ../dictionaries/${seqYName}.d2hW .

ln -s ../dictionaries/${seqYName}-revercomp.d2hP .
ln -s ../dictionaries/${seqYName}-revercomp.d2hW .

echo "${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}.${extensionY} ${length} ${similarity} ${WL} f &"
${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}.${extensionY} ${length} ${similarity} ${WL} f &

echo "${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}-revercomp.${extensionY} ${length} ${similarity} ${WL} r &"
${BINDIR}/comparison.sh ${seqXName}.${extensionX} ${seqYName}-revercomp.${extensionY} ${length} ${similarity} ${WL} r &

echo "Waiting for the comparisons"

for job in `jobs -p`
do
    wait $job
done

if [[ ! -f ${seqXName}-${seqYName}-sf.frags ]];       then
	echo -n -e \\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00 > ${seqXName}-${seqYName}-sf.frags
fi

if [[ ! -f ${seqXName}-${seqYName}-sr.frags ]];       then
	echo -n -e \\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00 > ${seqXName}-${seqYName}-sr.frags
fi

echo "${BINDIR}/combineFrags ${seqXName}-${seqYName}-sf.frags ${seqXName}-${seqYName}-revercomp-sr.frags ${seqXName}-${seqYName}.frags"
${BINDIR}/combineFrags ${seqXName}-${seqYName}-sf.frags ${seqXName}-${seqYName}-revercomp-sr.frags ${seqXName}-${seqYName}.frags

mv ${seqXName}-${seqYName}.frags ../frags

cd ..
rm -r ${seqXName}-${seqYName}

} &> /dev/null
