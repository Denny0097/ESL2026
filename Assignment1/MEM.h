#ifndef MEM_H
#define MEM_H

// Needed for the simple_target_socket
// #define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
using namespace sc_core;
using namespace sc_dt;
using namespace std;
// using namespace tlm;
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

SC_MODULE(MEM){
    tlm_utils::simple_target_socket<MEM> Ssocket;

    void func();
    void b_transport(tlm::tlm_generic_payload&, sc_time&);

    sc_dt::uint64 adr;

    SC_CTOR(MEM, unsigned int size_, sc_uint<32> base_, unsigned int lat_) :
                SIZE(size_), BASE(base_), latency(lat_), Ssocket("Ssocket") {
        Ssocket.register_b_transport(this, &MEM::b_transport);
        SC_METHOD(func);
        mem = new unsigned char[SIZE];
        for (unsigned int i = 0; i < SIZE; i++)
            mem[i] = ((BASE/1024) + 57 + i) % 256;
    }
private:
    sc_dt::uint64 BASE = 0;
    const unsigned int SIZE = 1024;
    const unsigned int latency = 3;
    unsigned char *mem;
};
#endif
