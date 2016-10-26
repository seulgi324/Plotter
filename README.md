# Plotter

## Things to come

There currently isn't a way to plot signal (you can make it a background act like a background, the the actual functionality hasn't been fully added to the code

More flexible legend position/sizer.  Maybe a dynamic system to have the legend not obscure the graph.  Long term goal

# How to Run

simply type
```
make
./Plotter config/<YOUR CONFIG FILE>
```

The config files are given names based on the update time, so to make sure you are using the correct values for cross-section and luminosity, use the newest config file

To change the looks of the graph, most of the style options are in the style directory.  The default style is used if nothing is specified in the config file, but any personal style can be made

More documentation on the styles to come.

## Options

The plotter comes with options that can be added at runtime similar to bash options.  To list all of the option and details about them, simply type

`./Plotter -help`

The option are what allow the plotter to configure which graph with go on the bottom of the canvas.  You can set it to Ratio Plot (Default), different significance plots, or remove the bottom graph all together.

## Important Files

This plotter is made for making stack plots and associated information.  Because it is made for making stack plots, a background is needed for the program.  It will crash if you do not have a background.

For the plotter to work, there are two main files that are needed for the program to run: the config file and the style file

### Config File

This is the file that is given to run the program, ie ```./Plotter <CONFIG FILE>```.  The config file generally will look something like this:

```
luminosity 15900
output     output.root
style      cms

files/Data.root Data.root

files/DYJetsToLL_M-10to50_amcatnloFXFX.root    DY+Jets.root    18610     0.999226 bg
...
...
file/Signal.root   Signal.root #### ### sig
```

The first three lines are variables used to set up the subsequent run: without these, the program will not run.  The luminosity and output are fairly obvious, but the style is the name of the style file you are using.  These are stored in the style directory, namely `Plotter/style`.  More on the style classes in the next section.

The next lines are for the actual files to be normalized.  There are three types of files used in the program: Data, Background, Signal.  Background is what is actually stacked while the Data and Signal are plotted alone.  

For Signal and Background there are several pieces of information needed:

``` 
<Infile location>  <Outfile location>  <Cross-section>  <Skim efficiency>  <tag> 
```

+ Infile location is just the location of the specific file 
+ Outfile location is the name for the normalized file.  NOTE: multiple files can be associated with a single outfile.  This allows multiple samples to be added together easily (eg ZZ, WZ, and WW can all be added to a file VV.root)
+ Cross-section is just the cross-section for the input file
+ Skim Efficiency is important for the Ntuples, for privately generated samples, just set this to 1
+ tag tells the program what type of sample you are using the tags are:
  + bg -- background
  + sig -- signal
  
For Data, since it doesn't need to be normalized, the only information needed is the infile location and outfile location.  The tag is also excluded because a sample without a cross-section is assumed to be data.

As with all BSM3G code, these config files support C++ and Python commenting (`//` and `#` respectively).

### Style File

Style files are simply holders for the information that will go into the overall TStyle class for each graph.  Benefits are that the style for the graphs are easily configured and allow for different styles for different uses (eg different styles for different journals).  The drawback is these styles are global so minor tweaks still require changing in ROOT or making changes to the Plotter code.

Regardless, if you look in the file `style/default`, you will see a long list of variables, some with numbers after them, some without.  This list is all of the variables that can be changed in the TStyle class.  The style file only reads in variables that have a input value after them, so all of the variables without numbers are skipped.  

If you have any questions about what any of the variables do, visit the [TStyle ROOT page](https://root.cern.ch/doc/master/classTStyle.html) and search for the variable; each variable VAR correspond to a ROOT function setVAR().

There are a few Plotter specific style variables though: 

+ PadRatio: This is the ratio of the height of the top graph and bottom graph.  If only the top graph is requested (see Options), this variable is ignored
+ TopWSRatio: This variable stands for "Top Whitespace Ratio."  When making the stack plots, the ratio of the height of the graph to the height of the whitespace above the graph can be set here.  example: the graph has a maximum of 1200 and the TopWSRatio is set to 15.  The Y Axis will be set to the range 1280.
+ RebinLimit: The plotter automatically combines bins together if they have a large error (if Error is just sqrt(Number of Events) then it effectively adds bins together that have low number of events.)  The percent error of a bin must be less than (up to a constant factor) of the RebinLimit, else it will be added to adjacent bins till the percent error is lower than the RebinLimit.  To remove rebinning, just make the RebinLimit an arbitrarily large number.
+ DivideBins: This represents a boolean value, so set it only to 0 or 1 (ie false or true).  To get a smoother distribution after variable rebinning, you can set the option to divide each bin by its width.  This also updates the Y Axis label to [Events/GeV].
+ BinLimit: There are certain graphs we don't want rebinned (eg Multiplicity graphs).  In general these graphs have a small number of bins, so to insure these graphs aren't rebinned, the BinLimit is the minimum number of bins needed after selecting the bins to rebin the graphs in the final root file.

The style file is read in by the Style Class, so if any features want to be added, be sure to have them read in properly by the Style Class





