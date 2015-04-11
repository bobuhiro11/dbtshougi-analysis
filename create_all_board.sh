#!/bin/bash
#PBS -V
#PBS -l nodes=1:ppn=1
#PBS -l walltime=6:00:00

cd $PBS_O_WORKDIR # qsubを実行したディレクトリへ移動

g++ create_all_board.cpp
./a.out 300000000
