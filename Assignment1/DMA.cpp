#include "DMA.h"

void DMA::b_transport(tlm::tlm_generic_payload& trans, sc_time& r_delay) {
    wait(r_delay);

    // Assumes all control registers are accessed as full 32-bit words
    tlm::tlm_command cmd_r = trans.get_command();
    unsigned char* r_data = trans.get_data_ptr();
    unsigned int r_len = trans.get_data_length();
    sc_dt::uint64 addr = trans.get_address();

    // Set OK status first, and change if anything goes otherwise
    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    // Reading and writing control registers based on address
    addr -= BASE;
    if (cmd_r == tlm::TLM_WRITE_COMMAND) {
        std::cout << "DMA received WRITE at " << std::hex << "0x" << addr+BASE
                  << " length " << std::dec << r_len <<  ": ";
        switch (addr) {
            case 0x0:               // SOURCE register
                if (!block) {
                    memcpy((unsigned char *)&SOURCE, r_data, r_len);
                    std::cout << std::hex << "0x" << SOURCE << std::endl;
                } else {
                    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                    std::cout << "blocked" << std::endl;
                }
                break;
            case 0x4:              // TARGET register
                if (!block) {
                    memcpy((unsigned char *)&TARGET, r_data, r_len);
                    std::cout << std::hex << "0x" << TARGET << std::endl;
                } else {
                    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                    std::cout << "blocked" << std::endl;
                }
                break;
            case 0x8:             // SIZE register
                if (!block) {
                    memcpy((unsigned char *)&SIZE, r_data, r_len);
                    std::cout << std::hex << "0x" << SIZE << std::endl;
                } else {
                    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                    std::cout << "blocked" << std::endl;
                }
                break;
            case 0xC:            // START/CLEAR register
                memcpy((unsigned char *)&START, r_data, r_len);
                std::cout << std::hex << "0x" << START << std::endl;
                if (START == 0x1)
                    block = true;  // Block from control register write
                else
                    block = false;
                break;
            default:
                trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                std::cout << std::hex << "wrong address" << std::endl;
        }
    } else if (cmd_r == tlm::TLM_READ_COMMAND) {
        switch (addr) {
            case 0x0:
                memcpy(r_data, (unsigned char *)&SOURCE, r_len);
                break;
            case 0x4:
                memcpy(r_data, (unsigned char *)&TARGET, r_len);
                break;
            case 0x8:
                memcpy(r_data, (unsigned char *)&SIZE, r_len);
                break;
            case 0xC:
                memcpy(r_data, (unsigned char *)&START, r_len);
                break;
            default:
                break;
        }
        std::cout << "DMA received READ at " << std::hex << "0x" << addr+BASE
                  << "length " << std::dec << r_len <<  ": "
                  << std::hex << "0x" << (sc_uint<32>)*r_data << std::endl;
    } else {               // TLM_IGNORE_COMMAND
        std::cout << "DMA receives TLM_IGNORE_COMMAND" << std::endl;
    }

    return;
}

void DMA::dma_process() {
    // Reset behavior
    SOURCE = 0;
    TARGET = 0;
    SIZE = 0;
    START = 0;
    block = false;
    intr = false;
    Interrupt.write(false);

    // DMA operational loop
    while (true) {
        wait();

        unsigned int rwData;
        // Start DMA transfer
        if (START) {
            if (SIZE > 0) {
                // TLM transaction common parameters
                trans.set_data_ptr(trf_data);
                if (SIZE >= 4)
                    trans.set_data_length(4);
                else
                    trans.set_data_length(SIZE);

                // TLM read transaction
                // trans.set_command(tlm::TLM_READ_COMMAND);
                trans.set_read();  // set TLM_READ_COMMAND
                trans.set_address(SOURCE);
                Msocket->b_transport(trans, delay);
                memcpy(&rwData, trf_data, 4);
                std::cout << "DMA read 0x" << std::hex << SOURCE << ":0x"
                          << rwData << std::endl;

                // Write to target
                // trans.set_command(tlm::TLM_WRITE_COMMAND);
                trans.set_write();  // set TLM_WRITE_COMMAND
                trans.set_address(TARGET);
                Msocket->b_transport(trans, delay);
                std::cout << "DMA write 0x" << std::hex << TARGET << ":0x"
                          << rwData << std::endl;

                //std::cout << "DMA SOURCE = " << std::hex << SOURCE
                //          << " TARGET = " << std::hex << TARGET << std::endl;
                if (SIZE > 4) {
                    SOURCE += 4;
                    TARGET += 4;
                    SIZE -= 4;
                } else
                    SIZE = 0;
                continue;
            }

            // Signal transfer is complete
            if (!intr) {
                Interrupt.write(true);
                intr = true;        // Interrupt pulled
                std::cout << "DMA Interrupt sent" << std::endl;
            }
        } else {
            block = false;  // Resume control register write
            if (intr) {
                Interrupt.write(0);
                intr = false;   // Interrupt cleared
                std::cout << "DMA clears Interrupt" << std::endl;
            }
        }
    }
}
