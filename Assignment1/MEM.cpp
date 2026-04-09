#include "MEM.h"

void MEM::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
    wait(latency);

    tlm::tlm_command      cmd = trans.get_command();
    unsigned char*        ptr = trans.get_data_ptr();
    unsigned int          len = trans.get_data_length();
    unsigned char*        byt = trans.get_byte_enable_ptr();
    unsigned int          wid = trans.get_streaming_width();
    unsigned int          rwData;
	
    adr = (trans.get_address() - BASE);
    std::cout << "adr = 0x" << std::hex << adr << " ";
    if (cmd == tlm::TLM_READ_COMMAND) {
        memcpy(ptr, &mem[adr], len);
        memcpy(&rwData, ptr, len);
        std::cout << "Read MEM 0x" << std::hex << trans.get_address()
                  << " length " << std::dec << len << ":0x"
                  << std::hex << rwData << std::endl;
    } else if (cmd == tlm::TLM_WRITE_COMMAND) {
        memcpy(&mem[adr], ptr, len);
        memcpy(&rwData, mem+adr, len);
        std::cout << "Write MEM 0x" << std::hex << trans.get_address()
                  << std::dec << " length " << len << ":0x"
                  << std::hex << rwData << std::endl;
    }
  
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    return;
}

void MEM::func() {
    // never triggered since no sensitivity list
}
