******************************************************************************
  CAEN SpA - Front End Division
  Via Vetraia, 11 - 55049 - Viareggio ITALY
  +390594388398 - www.caen.it
******************************************************************************

---------------------------------------------------------
Content 
---------------------------------------------------------
Readme.txt          : This file
digiTES_UserManual  : Preliminary User Manual. WARNING: this manual is a work 
                      in progress and is not updated to the latest SW version
ReleaseNotes.txt    : Release Notes of the last software release
src                 : Directory containing the source files and the Makefile
include             : Directory containing the header files
lib                 : Directory containing precompiled objects (ZCcal.o)
bin                 : Working directory containing executable file, 
                      configuration files, temp files, etc..
bin/DataFiles       : Default directory for saving output data files
 
---------------------------------------------------------
System Requirements
---------------------------------------------------------
- Linux kernel 2.4 or 2.6 and GNU C/C++ compiler
- glibc version 2.11.1 or above
- gnuplot version 5.0 or above (www.gnuplot.info)
- CAENVME library version 2.5 or above (http://www.caen.it/csite/CaenProd.jsp?idmod=689&parent=38)
- CAENComm library version 1.2 or above (http://www.caen.it/csite/CaenProd.jsp?parent=38&idmod=684)
- CAENDigitizer library version 2.8.0 or above (http://www.caen.it/csite/CaenProd.jsp?parent=38&idmod=717)
- drivers (at least one, depending on the connection type)
   - DT57xx-N67xx-DT55xx-V1718-N957 Linux USB Driver (direct USB to 
     Desktop/NIM or VME through V1718)
   - A3818 Linux Driver (direct Optical Link to Desktop/NIM or VME 
     through V2718)
   - A2818 Linux Driver (same as A3818)

---------------------------------------------------------
Installation and run notes
---------------------------------------------------------
- goto into the directory bin
- type 'make' and press enter (the Makefile in bin will call the general
  Makefile for the compilation in src)
- check that the executable file 'digiTES' is created in bin
- edit the default configuration file 'digiTES_Config.txt': if necessary, 
  modify the connection parameters (e.g. [BOARD 0] Open USB 0 0 for a Desktop 
  connected through the USB)
- run the program: type './digiTES' and press enter

---------------------------------------------------------
Tips
---------------------------------------------------------
- type './digiTES -h' for help about command line options
- while running, press the space bar in the terminal window for on-line help
- in gnuplot windows, there are some bindkeys:
    a: autoscale on X and Y
    y: autoscale on Y only (for histograms)
    l: log/lin scale on Y
    p: previous zoom
    r: ruler on/off
    g: grid on/off
    +: zoom in around cursor line
    -: zoom out around cursor line
- window zoom: right-click on 1st corner, release, left-click on 2nd corner

- In the bin folder, there is a set of configuration files for typical cases; 
  after the installation, the config file is just a copy of the file 
  'digiTES_Config_with_comments.txt' that contains almost all parameters with
  comments that explain their meaning.
  Launch digiTES and press 'C' in the console window to select different config 
  files. Press 'E' to open the current config file in a text editor.
- Other config files are:
    - digiTES_Config_HPGe.txt: for germanium detector (in general for any signal 
      coming from a charge sensitive preamplifier with resistive feedback). 
    - digiTES_Config_NaI.txt: energy spectrum from scintillation detectors 
      with PMT
    - digiTES_Config_LaBr3.txt: energy and timing from fast scintillation 
      detectors with PMT.

    