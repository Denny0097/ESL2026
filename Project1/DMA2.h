#ifndef DMA2_H
#define DMA2_H

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

using namespace sc_core;

// Define FSM states for the DMA controller
enum dma_state_e { IDLE, READ_MEM, WRITE_MEM, WAIT_CLEAR };

// Control Registers sets for each port
struct DMA_Channel {
    uint32_t source_reg;
    uint32_t target_reg;
    uint32_t size_reg;
    uint32_t start_clear_reg;

    dma_state_e state;
    uint32_t current_count;
    uint32_t data_buffer;
};

SC_MODULE(DMA2) {
    // Ports & Sockets
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_out<bool> INT1;
    sc_out<bool> INT2;

    tlm_utils::multi_passthrough_target_socket<DMA2> slave_p;
    tlm_utils::simple_initiator_socket<DMA2> master_p0;
    tlm_utils::simple_initiator_socket<DMA2> master_p1;

    uint32_t base_addr;

    // ch[0] for ARMv8-0，ch[1] for ARMv8-1
    DMA_Channel ch[2];

    // Thread Wrapper
    void dma_process_0();
    void dma_process_1();

    // Shared Initiator FSM function
    void execute_channel_fsm(int id,
                             tlm_utils::simple_initiator_socket<DMA2>& m_port,
                             sc_out<bool>& int_port);
                             
    virtual void b_transport(int id, tlm::tlm_generic_payload& trans, sc_time& delay);

    SC_CTOR(DMA2)
        : master_p0("master_p0"), master_p1("master_p1"), slave_p("slave_p") {
        base_addr = 0x100000;
        slave_p.register_b_transport(this, &DMA2::b_transport);

        SC_CTHREAD(dma_process_0, clk.pos());
        reset_signal_is(reset, false);

        SC_CTHREAD(dma_process_1, clk.pos());
        reset_signal_is(reset, false);
    }
};

#endif