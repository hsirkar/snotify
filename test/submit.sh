#!/bin/bash

#SBATCH -p debug
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH -t 00:03

./hello-world
