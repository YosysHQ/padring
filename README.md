# PADRING - a padring generator for ASICs
[![Build Status](http://api.cirrus-ci.com/github/YosysHQ/padring.svg)](https://cirrus-ci.com/github/YosysHQ/padring)

![](doc/padring.png)

This tool makes padrings for ASICs using a LEF file and a placement/configuration file.
The padrings can be output in GDS2, DEF and/or SVG format.
Check out the example in the `example` directory.

## Commandline options
* -h : show help.
* -L, --lef \<filename\> : mandatory, filename of LEF file that describes the ASIC cells.
* --svg \<filename\> : optional, filename of SVG to generate.
* --def \<filename\> : optional, filename of DEF to generate.
* --filler \<prefix\> : optional, filler cell prefix string to use when searching for filler cells.
* -o, --output \<filename\> : optional, filename of GDS2 to generate.

The filler cells are auto-detected by the padring program. Should this process fail, the user can add an explicit prefix which will be used to find the filler cells.

Multiple LEF files can be specified. During loading, existing cells with the same name will be overwritten.

## Configuration file

The following commands are available:

#### DESIGN \<name\> ;
* Sets the name of the design in the GDS2/DEF output file.
* Optional
* Default = PADRING

#### GRID \<grid size\> ;
* Sets the placement grid size in microns.
* Optional
* Default = 1 micron

#### AREA \<width\> \<height\> ;
* Defines the chip size in microns.
* Mandatory

#### CORNER \<instance_name\> \<location\> \<cell_name\> ;
* instance_name: name of the corner instance, i.e. corner_1.
* location: location of the corner, one of NE,SE,NW,SW.
* cell_name: name of corner cell from the cell library.

#### PAD \<instance_name\> \<location\> [FLIP] \<cell_name\> ;
* instance_name: name of the pad instance, i.e. gpio_1.
* location: location of the pad, one of N,S,E,W.
* optional 'FLIP': flips cell in Y axis.
* cell_name: name of pad cell from the cell library.

#### SPACE \<space\> ;
* space: the space between the preceeding and succeeding cell, in microns.

Space between the I/O pads is distributed evenly unless a specific space between two pads is specified directly using the SPACE command.


## Building

Dependencies:
* CMAKE 3.10 or better.
* Ninja build.
* C++17 capable compiler.
* Optionally: Doxygen.

Building:
* Run `bootstrap.sh` to initialize the CMAKE/Ninja build system.
* Run `ninja` from the build directory.