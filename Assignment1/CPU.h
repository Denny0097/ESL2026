#ifndef CPU_H
#define CPU_H

#include <iomanip>
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

SC_MODULE(CPU) {
    tlm_utils::simple_initiator_socket<CPU> Msocket;

    sc_in<bool>  clk;
    sc_in<bool>  Reset;
    sc_in<bool>  Interrupt;

    sc_uint<32> data_source;
    sc_uint<32> data_target;
    sc_uint<32> data_size;
    sc_uint<32> data_start;
    int count     = 0;
    int rst_count = 0;
    bool errTest = false;
    unsigned int Tcount = 0;

    tlm::tlm_generic_payload trans_m;
    sc_time delay = sc_time(10, SC_NS);

    void startCPU();

    SC_CTOR(CPU) : Msocket("Msocket") {
        SC_THREAD(startCPU);
        sensitive << clk.pos() << Reset.pos();
        //SC_CTHREAD(startCPU, clk.pos());
        //reset_signal_is(Reset, true);
    }
};
#endif
