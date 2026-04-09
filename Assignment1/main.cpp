#include "DMA.h"
#include "MEM.h"
#include "CPU.h"
#include<systemc.h>

int sc_main(int argc, char* argv[]){
    // Instantiate components
    DMA    *dma;
    MEM    *memory;
    CPU    *cpu;
    dma    = new DMA("dma", 0x4000);
    memory = new MEM("memory", 0x10000, 0x5000, 3);
    cpu    = new CPU("cpu");

    sc_time prd = sc_time(10, SC_NS);
    sc_clock clk("clk", prd);
    sc_signal<bool> Interrupt, Reset;

    cpu->Interrupt(Interrupt);
    dma->Interrupt(Interrupt);
	
    cpu->clk(clk);
    cpu->Reset(Reset);
    dma->clk(clk);
    dma->Reset(Reset);
	
    cpu->Msocket.bind(dma->Ssocket);
    dma->Msocket.bind(memory->Ssocket);
	
    sc_trace_file *tf = sc_create_vcd_trace_file("RESULT");
    sc_trace(tf, clk, "clk");
    sc_trace(tf, Reset, "Reset");
    sc_trace(tf, Interrupt, "Interrupt");
    sc_trace(tf, dma->SOURCE, "SOURCE");
    sc_trace(tf, dma->TARGET, "TARGET");
    sc_trace(tf, dma->SIZE, "SIZE");
    sc_trace(tf, dma->START, "START");
    sc_trace(tf, dma->BASE, "BASE");

    sc_trace(tf, cpu->data_source, "data_source");
    sc_trace(tf, cpu->data_target, "data_target");
    sc_trace(tf, cpu->data_size, "data_size");
    sc_trace(tf, cpu->data_start, "data_start");
    sc_trace(tf, cpu->count, "count");

    sc_start(10, SC_NS);
    Reset.write(1);
    sc_start(10, SC_NS);
    Reset.write(0);
    sc_start(1000, SC_NS);

    sc_close_vcd_trace_file(tf);
    sc_stop();
	
    return 0;
}







