##############################################################
# Author: Andres Florez, Universidad de los Andes, Colombia. #
##############################################################

ObjSuf = o
SrcSuf = cc
ExeSuf =
DllSuf = so
OutPutOpt = -o
HeadSuf = h

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)

# Linux with egcs

CXX = g++
CXXFLAGS += $(ROOTCFLAGS) -I./ -g

LD = g++
LDFLAGS += $(ROOTLIBS) -g

SOFLAGS = -shared
LIBS =

SRCDIR = src
OBJDIR = obj

#------------------------------------------------------------------------------
SOURCES = $(wildcard $(SRCDIR)/*.cc)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)
#------------------------------------------------------------------------------

all: Plotter $(OBJECTS)

Final: obj/Final.o obj/Normalizer.o obj/Style.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)	

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%: $(OBJDIR)/%.o
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	@echo "Cleaning..."
	@ls $(OBJDIR)
	@rm -f $(OBJECTS)

job: Final
	./Final config/final.config
	root -l open.C

.SUFFIXES: .$(SrcSuf) .cc .o .so
