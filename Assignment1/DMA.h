#ifndef DMA_H
#define DMA_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>

SC_MODULE(DMA) {
    // TLM sockets
    tlm_utils::simple_initiator_socket<DMA> Msocket;
    tlm_utils::simple_target_socket<DMA> Ssocket;

    // Other I/O ports
    sc_in<bool>  clk;            // Clock pin
    sc_out<bool> Interrupt;      // Interrupt signal
    sc_in<bool>  Reset;          // Reset pin

    // Control Registers
    sc_uint<32> SOURCE;          // SOURCE register
    sc_uint<32> TARGET;          // TARGET register
    sc_uint<32> SIZE;            // SIZE register
    sc_uint<32> START;           // START/CLEAR register

    sc_dt::uint64 BASE = 0x1000; // BASE address register

    // Data area
    tlm::tlm_generic_payload trans;
    unsigned char trf_data[4]; // Transfer data area, 4 bytes
    sc_time delay = sc_time(10, SC_NS);  // 10 ns transfer latency
    bool block, intr;

    // Process function and TLM slave socket function
    void dma_process();
    void b_transport(tlm::tlm_generic_payload&, sc_time&);

    // DMA construction. The BASE address is hard coded.
    SC_CTOR(DMA, sc_dt::uint64 base_)
        : Msocket("Msocket"), Ssocket("Ssocket"), 
          SOURCE(0), TARGET(0), SIZE(0), START(0), BASE(base_),
          block(false), intr(false) {
        Ssocket.register_b_transport(this, &DMA::b_transport);
        SC_CTHREAD(dma_process, clk.pos());
        reset_signal_is(Reset, true);
    }
};
#endif
