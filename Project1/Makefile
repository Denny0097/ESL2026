SYSTEMC_DIR = ${SYSTEMC_HOME}
PROJECT     = dma2
BUILDFLAGS  = -g3
CXX         = g++ -std=c++17

INCFLAGS    = -I. -I${SYSTEMC_DIR}/include
LDFLAGS     = -L${SYSTEMC_DIR}/lib -Wl,-rpath,${SYSTEMC_DIR}/lib -lsystemc -lm
SRC_CPPHEAD = DMA2
SRC_CPP     =
HEADERS     =
MAIN        = main.cpp
OBJECTS     = $(SRC_CPPHEAD:=.o) $(SRC_CPP:cpp=o)

EXE = $(PROJECT)

all: $(EXE)

$(EXE): $(MAIN) $(OBJECTS) $(HEADERS)
	@echo "$@ building..."
	$(CXX) $(INCFLAGS) $(MAIN) $(OBJECTS) $(LDFLAGS) -o $@
	@echo ""
	@echo "$@ build done successfully..."
	@echo ""

%.o:%.cpp %.h
	@echo "Compiling $< ..."
	$(CXX) -c $< $(INCFLAGS)

clean:
	rm -f $(EXE)
	rm -f *.o
	rm *.vcd
