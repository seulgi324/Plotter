#!/bin/bash

printf "\n"
echo Before running the plotting routine, we need to create the analysis directory. What do you want to name your directory?
printf "\n"

read dirname

for y in `ls -d */`
do

  new_file=`echo $y | sed 's!/!!'`
  cd ${new_file}
  mkdir $dirname
  cp default/* $dirname
  cd ./..
  cp BSM3GPlotter ${new_file}/$dirname

done

rm moveRootFilesToDirectories.csh
cp moveRootFilesToDirectories_default.csh moveRootFilesToDirectories.csh
sed -i -e s/TEMPDIRECTORY/"$dirname"/g moveRootFilesToDirectories.csh
./moveRootFilesToDirectories.csh

printf "\n"
echo The analysis root files have been moved to directory $dirname

rm main.in
cp main.in.default main.in
sed -i -e s/TEMPDIRECTORY/"$dirname"/g main.in

rm Plotter.in
cp Plotter.in.default Plotter.in
sed -i -e s/TEMPDIRECTORY/"$dirname"/g Plotter.in

printf "\n"
echo The Plotter configuration files have been modified accordingly
printf "\n"

