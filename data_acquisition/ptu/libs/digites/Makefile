IDIR =./include
CC=g++
CFLAGS=-I$(IDIR) -Wall -g -lm -lCAENDigitizer -O2 -DLINUX -fPIC -shared

ODIR=./obj
LDIR =./lib
SDIR = ./src

_DEPS = BoardUtils.h Configure.h Console.h DataFiles.h digiTES.h fft.h Histograms.h ParamParser.h Plots.h PreProcess.h Queues.h Readout.h Statistics.h ZCcal.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = BoardUtils.o Configure.o Console.o DataFiles.o digiTES.o fft.o Histograms.o ParamParser.o Plots.o PreProcess.o Queues.o Readout.o Statistics.o ZCcal.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

libproject.so: $(OBJ)
	$(CC) -shared $^ -o $@ -lCAENDigitizer -lm -Wall -fPIC

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ 
