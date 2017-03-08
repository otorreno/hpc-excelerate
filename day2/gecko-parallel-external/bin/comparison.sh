#!/bin/bash 

FL=1000000   # frequency limit

if [ $# != 6 ]; then
   echo " ==== ERROR ... you called this script inappropriately."
   echo ""
   echo "   usage:  $0 seqXName seqYName lenght similarity WL strand"
   echo ""
   exit -1
fi

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
strand=${6}

echo "${BINDIR}/hits ${seqXName} ${seqYName} ${seqXName}-${seqYName}-K${WL}.hits ${FL} ${WL}"
${BINDIR}/hits ${seqXName} ${seqYName} ${seqXName}-${seqYName}-K${WL}.hits ${FL} ${WL}

echo "${BINDIR}/sortHits 10000000 32 ${seqXName}-${seqYName}-K${WL}.hits ${seqXName}-${seqYName}-K${WL}.hits.sorted"
${BINDIR}/sortHits 10000000 32 ${seqXName}-${seqYName}-K${WL}.hits ${seqXName}-${seqYName}-K${WL}.hits.sorted

echo "${BINDIR}/filterHits ${seqXName}-${seqYName}-K${WL}.hits.sorted ${seqXName}-${seqYName}-K${WL}.hits.sorted.filtered ${WL}"
${BINDIR}/filterHits ${seqXName}-${seqYName}-K${WL}.hits.sorted ${seqXName}-${seqYName}-K${WL}.hits.sorted.filtered ${WL}

echo "${BINDIR}/FragHits $1 $2 ${seqXName}-${seqYName}-K${WL}.hits.sorted.filtered ${seqXName}-${seqYName}-s${strand}.frags ${length} ${similarity} ${WL} 1 ${strand}"
${BINDIR}/FragHits $1 $2 ${seqXName}-${seqYName}-K${WL}.hits.sorted.filtered ${seqXName}-${seqYName}-s${strand}.frags ${length} ${similarity} ${WL} 1 ${strand}