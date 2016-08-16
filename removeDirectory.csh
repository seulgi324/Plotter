#!/bin/bash

printf "\n"
echo This script is for removing unwanted analysis directories. Which directory do you want to remove?
printf "\n"

read dirname

for y in `ls -d */`
do

  new_file=`echo $y | sed 's!/!!'`
  cd ${new_file}
  rm -rf $dirname
  echo Directory ${new_file}/$dirname has been removed
  cd ./..

done

