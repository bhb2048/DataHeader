#
#	Makefile for Headerlib.a
#	
#	20-sep-2002
#	
include ../makeinclude

CXX = g++
OPTIMIZER = -g

CXXFLAGS= $(OPTIN) $(OPTIMIZER) $(CXXOPTS) $(INCLUDEDIR)
ARFLAGS= rs

INCLUDEDIR = -I/usr/local/include

# list of all source files
SOURCES = AdGain.cc AdLog.cc AnalysisGr.cc Calibrate.cc ChannelGr.cc ChannelSet.cc \
			DisplayGr.cc SampleGr.cc DataHeader.cc AMode.cc AModule.cc ModuleControls.cc \
			RtAnalysisGr.cc
			
%.o : %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


LIB= $(LIBPTH)/Headerlib.a

LIBOBJS= $(SOURCES:.cc=.o)

$(LIB):	$(LIB)($(LIBOBJS))

clean:
	rm -f $(LIB) *.o
	
tidy:
	rm -f *.o

depend:
	makedepend -I. $(SOURCES) >/dev/null 2>&1
# DO NOT DELETE
