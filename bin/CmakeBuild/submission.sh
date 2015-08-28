#! /bin/sh
#PBS -N g09DNNTSBuilder
#PBS -o pbsjob.out
#PBS -e pbsjob.err
#PBS -M jsmith48@ufl.edu
#PBS -r n
#PBS -l walltime=48:00:00
#PBS -l nodes=1:ppn=21
#PBS -l pmem=10gb

cd $PBS_O_WORKDIR

g09root="/apps"
export g09root
. $g09root/g09/bsd/g09.profile

export LD_RUN_PATH="/usr/gnu5.2/lib64:$LD_RUN_PATH"

export OMP_NUM_THREADS=21

./g09DNNTSBuilder -r uniform -i input.ipt -o outputtrainH2O.opt -d tdatatrainH2O.dat

