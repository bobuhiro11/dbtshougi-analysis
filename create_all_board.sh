#!/bin/bash
#PBS -V
#PBS -l nodes=1:ppn=1
#PBS -l walltime=6:00:00

cd $PBS_O_WORKDIR # qsubを実行したディレクトリへ移動

g++ -DMAX_BOARD_NUM=300000000 create_all_board.cpp
./a.out
