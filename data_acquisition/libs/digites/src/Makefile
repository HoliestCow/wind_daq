
P00 = digiTES
P01 = Console
P02 = BoardUtils
P03 = Configure
P04 = DataFiles
P05 = ParamParser
P06 = Plots
P07 = PreProcess
P08 = Queues
P09 = Readout
P10 = Histograms
P11 = Statistics
P12 = fft

BINDIR = ../bin
LIBDIR = ../lib
INCL   = -I../include 
LIBR   = -DLINUX -lCAENDigitizer -lCAENComm -lm 
FLAGS  = -fpermissive -Wno-write-strings


all:	$(P00) 

$(P00): $(P00).o $(P01).o $(P02).o $(P03).o $(P04).o $(P05).o $(P06).o $(P07).o $(P08).o $(P09).o $(P10).o $(P11).o $(P12).o $(LIBDIR)/ZCcal.o
	g++ $(FLAGS) -o $(BINDIR)/digiTES $(P00).o $(P01).o $(P02).o $(P03).o $(P04).o $(P05).o $(P06).o $(P07).o $(P08).o $(P09).o $(P10).o $(P11).o $(P12).o $(LIBDIR)/ZCcal.o $(LIBR)
	 

$(P00).o:$(P00).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P00).cpp
$(P01).o: $(P01).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P01).cpp
$(P02).o:$(P02).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P02).cpp
$(P03).o:$(P03).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P03).cpp
$(P04).o:$(P04).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P04).cpp
$(P05).o:$(P05).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P05).cpp
$(P06).o:$(P06).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P06).cpp
$(P07).o:$(P07).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P07).cpp
$(P08).o:$(P08).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P08).cpp
$(P09).o:$(P09).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P09).cpp
$(P10).o:$(P10).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P10).cpp
$(P11).o:$(P11).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P11).cpp
$(P12).o:$(P12).cpp 
	g++  $(FLAGS) $(LIBR) $(INCL) -c $(P12).cpp
	
clean: 
	rm -rf *.o $(BINDIR)/digiTES


	