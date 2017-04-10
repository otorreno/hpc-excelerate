#!/bin/bash
# number of cores in the same node:
##SBATCH -c 17
# amount of memory
##SBATCH --mem=300gb
# walltime
#SBATCH --time=10:00:00

#Loading the MPI module
module load openmpi

if [ $# != 7 ]; then
   echo " ==== ERROR ... you called this script inappropriately."
   echo ""
   echo "   usage:  $0 seqXName seqYName lenght similarity WL cores tasks_per_core"
   echo ""
   exit -1
fi

length=$3
similarity=$4
WL=$5
procs=$(($6-1))
tpn=$7
np=$(($procs*$tpn))

BASE_DIR="/mnt/home/users/tic_182_uma/oscart/gecko-parallel-internal/bin"

#Reverse complement
echo "Reverse complement"
echo "---------------------------------------------------------------------------------------"

echo "$BASE_DIR/reverseComplement $2 $2-revercomp"
$BASE_DIR/reverseComplement $2 $2-revercomp

echo "======================================================================================="

#Dictionaries
echo "Dictionaries"
echo "---------------------------------------------------------------------------------------"

echo "/usr/bin/time -f %E --output=$1-dict-time.txt mpirun -np $procs $BASE_DIR/dictionary $1 32 $1-dict.sh $tpn"
/usr/bin/time -f %E --output=$1-dict-time.txt mpirun -np $procs $BASE_DIR/dictionary $1 32 $1-dict.sh $tpn

echo "/usr/bin/time -f %E --output=$2-dict-time.txt mpirun -np $procs $BASE_DIR/dictionary $2 32 $2-dict.sh $tpn"
/usr/bin/time -f %E --output=$2-dict-time.txt mpirun -np $procs $BASE_DIR/dictionary $2 32 $2-dict.sh $tpn

echo "mpirun -np $procs $BASE_DIR/dictionary $2-revercomp 32 $2-dict.sh $tpn"
mpirun -np $procs $BASE_DIR/dictionary $2-revercomp 32 $2-dict.sh $tpn

echo "======================================================================================="

#Hits
echo "Hits"
echo "---------------------------------------------------------------------------------------"

echo "/usr/bin/time -f %E --output=$1-$2-hits-time.txt mpirun -np $procs $BASE_DIR/hits $1 $2 $1-$2-hits.sh $1-$2.hits 1000000 $WL $tpn"
/usr/bin/time -f %E --output=$1-$2-hits-time.txt mpirun -np $procs $BASE_DIR/hits $1 $2 $1-$2-hits.sh $1-$2.hits 1000000 $WL $tpn

echo "/usr/bin/time -f %E --output=$1-$2-revercomp-hits-time.txt mpirun -np $procs $BASE_DIR/hits $1 $2-revercomp $1-$2-hits.sh $1-$2-revercomp.hits 1000000 $WL $tpn"
/usr/bin/time -f %E --output=$1-$2-revercomp-hits-time.txt mpirun -np $procs $BASE_DIR/hits $1 $2-revercomp $1-$2-hits.sh $1-$2-revercomp.hits 1000000 $WL $tpn

#rm -rf *.dict

cat $1-$2.hits.[ACGT]* > $1-$2.hits
#rm -rf $1-$2.hits.[ACGT]*

cat $1-$2-revercomp.hits.[ACGT]* > $1-$2-revercomp.hits
#rm -rf $1-$2-revercomp.hits.[ACGT]*

echo "======================================================================================="

#SortHits
echo "SortHits"
echo "---------------------------------------------------------------------------------------"

echo "/usr/bin/time -f %E --output=$1-$2-sortHits-time.txt mpirun -np $np $BASE_DIR/sortHits $1-$2.hits $1-$2.hits.sorted"
/usr/bin/time -f %E --output=$1-$2-sortHits-time.txt mpirun -np $np $BASE_DIR/sortHits $1-$2.hits $1-$2.hits.sorted

echo "/usr/bin/time -f %E --output=$1-$2-revercomp-sortHits-time.txt mpirun -np $np $BASE_DIR/sortHits $1-$2-revercomp.hits $1-$2-revercomp.hits.sorted"
/usr/bin/time -f %E --output=$1-$2-revercomp-sortHits-time.txt mpirun -np $np $BASE_DIR/sortHits $1-$2-revercomp.hits $1-$2-revercomp.hits.sorted

#rm $1-$2.hits
#rm $1-$2-revercomp.hits

echo "======================================================================================="

echo "FilterHits"
echo "---------------------------------------------------------------------------------------"

echo "$BASE_DIR/filterHits $1-$2.hits.sorted $1-$2.hits.sorted.filtered $WL"
$BASE_DIR/filterHits $1-$2.hits.sorted $1-$2.hits.sorted.filtered $WL

echo "$BASE_DIR/filterHits $1-$2-revercomp.hits.sorted $1-$2-revercomp.hits.sorted.filtered $WL"
$BASE_DIR/filterHits $1-$2-revercomp.hits.sorted $1-$2-revercomp.hits.sorted.filtered $WL

#rm -rf $1-$2.hits.sorted
#rm -rf $1-$2-revercomp.hits.sorted

echo "======================================================================================="

echo "FragHits"
echo "---------------------------------------------------------------------------------------"

echo "$BASE_DIR/splitHits $1-$2.hits.sorted.filtered $1-$2.hits.sorted.filtered.index $np"
$BASE_DIR/splitHits $1-$2.hits.sorted.filtered $1-$2.hits.sorted.filtered.index $np

echo "$BASE_DIR/splitHits $1-$2-revercomp.hits.sorted.filtered $1-$2-revercomp.hits.sorted.filtered.index $np"
$BASE_DIR/splitHits $1-$2-revercomp.hits.sorted.filtered $1-$2-revercomp.hits.sorted.filtered.index $np

echo "/usr/bin/time -f %E --output=$1-$2-fragHits-time.txt mpirun -np $(($np+1)) $BASE_DIR/fragHits $1 $2 $1-$2.hits.sorted.filtered $1-$2-f.frags $length $similarity $WL 1 f $1-$2.hits.sorted.filtered.index"
/usr/bin/time -f %E --output=$1-$2-fragHits-time.txt mpirun -np $(($np+1)) $BASE_DIR/fragHits $1 $2 $1-$2.hits.sorted.filtered $1-$2-f.frags $length $similarity $WL 1 f $1-$2.hits.sorted.filtered.index

echo "/usr/bin/time -f %E --output=$1-$2-revercomp-fragHits-time.txt mpirun -np $(($np+1)) $BASE_DIR/fragHits $1 $2-revercomp $1-$2-revercomp.hits.sorted.filtered $1-$2-r.frags $length $similarity $WL 1 r $1-$2-revercomp.hits.sorted.filtered.index"
/usr/bin/time -f %E --output=$1-$2-revercomp-fragHits-time.txt mpirun -np $(($np+1)) $BASE_DIR/fragHits $1 $2-revercomp $1-$2-revercomp.hits.sorted.filtered $1-$2-r.frags $length $similarity $WL 1 r $1-$2-revercomp.hits.sorted.filtered.index

echo "======================================================================================="

#rm -rf $1-$2.hits.sorted.filtered $1-$2.hits.sorted.filtered.index
#rm -rf $1-$2-revercomp.hits.sorted.filtered $1-$2-revercomp.hits.sorted.filtered.index

echo "GenerateCSV"
echo "---------------------------------------------------------------------------------------"

echo "$BASE_DIR/combineFrags $1-$2-f.frags $1-$2-r.frags $1-$2.frags"
$BASE_DIR/combineFrags $1-$2-f.frags $1-$2-r.frags $1-$2.frags

echo "$BASE_DIR/generateCSV $1-$2.frags $1-$2.csv"
$BASE_DIR/generateCSV $1-$2.frags $1-$2.csv

echo "======================================================================================="

dictX=`cat $1-dict-time.txt`
dictY=`cat $2-dict-time.txt`
hits=`cat $1-$2-hits-time.txt`
sortHits=`cat $1-$2-sortHits-time.txt`
fragHits=`cat $1-$2-fragHits-time.txt`
hitsrevercomp=`cat $1-$2-revercomp-hits-time.txt`
sortHitsrevercomp=`cat $1-$2-revercomp-sortHits-time.txt`
fragHitsrevercomp=`cat $1-$2-revercomp-fragHits-time.txt`

echo "===================================" >> time.txt
echo "Number of cores: $procs" >> time.txt
echo "===================================" >> time.txt
echo "$1-dict;$2-dict;$1-$2-hits;$1-$2-sortHits;$1-$2-fragHits;$1-$2-revercomp-hits;$1-$2-revercomp-sortHits;$1-$2-revercomp-fragHits" >> time.txt
echo "$dictX;$dictY;$hits;$sortHits;$fragHits;$hitsrevercomp;$sortHitsrevercomp;$fragHitsrevercomp" >> time.txt

rm -rf $1-dict-time.txt $2-dict-time.txt $1-$2-hits-time.txt $1-$2-sortHits-time.txt $1-$2-fragHits-time.txt $1-$2-revercomp-hits-time.txt $1-$2-revercomp-sortHits-time.txt cat $1-$2-revercomp-fragHits-time.txt
