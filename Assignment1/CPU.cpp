#include "CPU.h"

void CPU::startCPU() {
    while(1) {
        wait();
        if (Reset.read()) {
            rst_count = 0;
            continue;
        } else if (clk.read()) {
            if (rst_count < 2) {
                rst_count++;
                continue;
            }
            if (Interrupt.read() == 1) {
                data_source = 0x5000;
                data_target = 0x5080;
                data_size = 0x0000;
                data_start = 0x0000;
                trans_m.set_command(tlm::TLM_WRITE_COMMAND);
                trans_m.set_write();
                trans_m.set_data_length(24);
                trans_m.set_address(0x400c);
                trans_m.set_data_ptr((unsigned char*)&data_start);
                // Send transaction
                Msocket->b_transport(trans_m, delay);
                if (trans_m.is_response_error()) {
                    cerr << "Error in write transaction." << std::endl;
                }
                count = 0;
                std::cout << "CPU received Interrupt, CLEAR sent" << std::endl;
                while (Interrupt.read() == 1) {
                    std::cout << "CPU waits for Interrupt off" << std::endl;
                    wait(delay*4);
                }
                std::cout << "CPU receives Interrupt Cleared" << std::endl;
            } else if (Interrupt.read() == 0) {
                data_source = 0x5000;
                data_target = 0x5080;
                data_size = 0x0012;
                data_start = 0x0001;

                while (count < 4) {
                    // Common TLM command settings
                    trans_m.set_command(tlm::TLM_WRITE_COMMAND);
                    trans_m.set_write();
                    trans_m.set_data_length(24);

                    if (count == 0){
                        trans_m.set_address(0x4000);
                        trans_m.set_data_ptr((unsigned char*)&data_source);
                    } else if (count == 1) {
                        trans_m.set_address(0x4004);
                        trans_m.set_data_ptr((unsigned char*)&data_target);
                    } else if (count == 2) {
                        trans_m.set_address(0x4008);
                        trans_m.set_data_ptr((unsigned char*)&data_size);
                    } else if (count == 3){
                        trans_m.set_address(0x400c);
                        trans_m.set_data_ptr((unsigned char*)&data_start);
                    }
                    // Send transaction
                    Msocket->b_transport(trans_m, delay);
                    if (trans_m.is_response_error()) {
                        cerr << "Error in write transaction." << std::endl;
                    }
                    // std::cout << "CPU programs DMA" << std::endl;
                    wait(delay);
                    count++;
                }
                // Testing the blocking of register writes
                if (!errTest) {
                    trans_m.set_command(tlm::TLM_WRITE_COMMAND);
                    trans_m.set_write();
                    trans_m.set_data_length(24);
                    if (Tcount == 5) {
                        trans_m.set_address(0x4000);
                        trans_m.set_data_ptr((unsigned char*)&data_source);
                        Msocket->b_transport(trans_m, delay);
                        if (trans_m.is_response_error()) {
                            cerr << "Error in write transaction." << std::endl;
                        }
                    } else if (Tcount == 10) {
                        trans_m.set_address(0x400C);
                        data_start = 0x0000;
                        trans_m.set_data_ptr((unsigned char*)&data_start);
                        Msocket->b_transport(trans_m, delay);
                        if (trans_m.is_response_error()) {
                            cerr << "Error in write transaction." << std::endl;
                        }
                    } else if (Tcount == 40) {
                        trans_m.set_address(0x400C);
                        data_start = 0x0001;
                        trans_m.set_data_ptr((unsigned char*)&data_start);
                        Msocket->b_transport(trans_m, delay);
                        if (trans_m.is_response_error()) {
                            cerr << "Error in write transaction." << std::endl;
                        }
                        errTest = true;
                    }
                    Tcount++;
                }
            }
        }
        //std::cout << "One DMA run is done" << endl;
    }
}
