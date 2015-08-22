#! /bin/sh
#PBS -N g09DNNTSBuilder
#PBS -o pbsjob.out
#PBS -e pbsjob.err
#PBS -M jsmith48@ufl.edu
#PBS -r n
#PBS -l walltime=12:00:00
#PBS -l nodes=1:ppn=1
#PBS -l pmem=1gb

cd $PBS_O_WORKDIR

module load mkl
module load gaussian/g09

export LD_RUN_PATH="/home/jsmith48/scratch/compilers/usr/lib64:$LD_RUN_PATH"

export OMP_NUM_THREADS=

./g09DNNTSBuilder -i input.ipt -o output.opt -d tdata.dat

