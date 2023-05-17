### version 0.1a

* initial release.

### version 0.2a

* Added LEF -> DEF database unit setting (default=100).
* Added DESIGN [design name] command to configuration file.
* Fixed return values from LEF parser.
* Added END DESIGN to DEF file.

### version 0.2b

* ignore PROPERTYDEFINITIONS blocks.
* added optional 'FLIP' flag to flip PADs.
* fixed DEF corner cell orientation.
* enhanced SVG output so the designer can see the cell orientation.

### version 0.2c

* fixed a bug were PADRING would infintely loop if a gap could not be closed with the available filler cells.
* name of replaced cells are now correctly displayed.
* fixed a bug where replacing cells corrupted the cell database.
* fixed a bug where non-square corner cells were not placed correctly in DEF file.

### version 0.2d

* updated contrib/cxxopts.h
* pin names can now have a bus index [].
* config file can now have pin names with \ and .
* include file fix in debugutils.
