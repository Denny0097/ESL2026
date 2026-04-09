SYSTEMC_DIR = /usr/local/systemc-3.0.2
PROJECT     = dma
BUILDFLAGS  = -g3
CXX         = g++ -std=c++17

INCFLAGS    = -I. -I${SYSTEMC_DIR}/include
LDFLAGS     = -L${SYSTEMC_DIR}/lib -Wl,-rpath,${SYSTEMC_DIR}/lib -lsystemc -lm
SRC_CPPHEAD = MEM DMA CPU
SRC_CPP     =  
HEADERS     = 
MAIN        = main.cpp 
OBJECTS     = $(SRC_CPPHEAD:=.o) $(SRC_CPP:cpp=o)

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/systemc/cur/lib-linux64
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
 
