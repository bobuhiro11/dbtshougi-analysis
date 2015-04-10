#!/bin/bash
#PBS -V
#PBS -l nodes=1:ppn=1
#PBS -l walltime=6:00:00
#PBS -o /home/n-miki/dobutushogi-create-all-board.log

source /home/n-miki/.bashrc
script_dir=$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)
cd ${script_dir}

g++ -DMAX_BOARD_NUM=300000000 create_all_board.cpp
./a.out
