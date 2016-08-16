#!/bin/bash

printf "\n"
echo This script is for removing unwanted Plotter output files. Which directory do you want to consider?
printf "\n"

read dirname

for y in `ls -d */`
do

  new_file=`echo $y | sed 's!/!!'`
  cd ${new_file}
  cd $dirname
  rm *NormalizedHistos*root
  rm *ProbabilityHistos*root
  rm *log
  echo Plotter output files in directory ${new_file}/$dirname have been removed
  cd ./../..

done

