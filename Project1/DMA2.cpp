#include "DMA2.h"

// Target socket
void DMA2::b_transport(int /* id */, tlm::tlm_generic_payload& trans, sc_time& delay) {
    tlm::tlm_command cmd = trans.get_command();
    uint64_t addr = trans.get_address();
    unsigned char *ptr = trans.get_data_ptr(); // point to data
    
    // Check if the address hits the DMA base address range
    if (addr >= base_addr && addr < base_addr + 0x20) {
        uint64_t offset = addr - base_addr;

        // Channel select (0x0~0xF -> ch 0; 0x10~0x1F -> ch 1)
        int ch_id = (offset >= 0x10) ? 1 : 0;

        // turn to 0x0, 0x4, 0x8, 0xC
        uint64_t local_offset = offset % 0x10; 
        
        if (cmd == tlm::TLM_WRITE_COMMAND) {
            uint32_t val = *((uint32_t *)ptr);
            switch (local_offset) {
                case 0x0:
                    ch[ch_id].source_reg = val;
                    break;
                case 0x4:
                    ch[ch_id].target_reg = val;
                    break;
                case 0x8:
                    ch[ch_id].size_reg = val;
                    break;
                case 0xC:
                    ch[ch_id].start_clear_reg = val & 0x1;
                    break;
            }
        } else if (cmd == tlm::TLM_READ_COMMAND) {
            uint32_t ret = 0;
            switch (local_offset) {
                case 0x0:
                    ret = ch[ch_id].source_reg;
                    break;
                case 0x4:
                    ret = ch[ch_id].target_reg;
                    break;
                case 0x8:
                    ret = ch[ch_id].size_reg;
                    break;
                case 0xC:
                    ret = ch[ch_id].start_clear_reg;
                    break;
            }
            *((uint32_t *)ptr) = ret;
        }
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }

    wait(delay);
    delay = SC_ZERO_TIME;
}

// Thread Wrapper Declarations
void DMA2::dma_process_0() { execute_channel_fsm(0, master_p0, INT1); }
void DMA2::dma_process_1() { execute_channel_fsm(1, master_p1, INT2); }

// Initiator socket: DMA Explicit FSM Thread
void DMA2::execute_channel_fsm(int id,
                               tlm_utils::simple_initiator_socket<DMA2> &m_port,
                               sc_out<bool> &int_port) {
    // 1. Reset logic
    ch[id].source_reg = 0;
    ch[id].target_reg = 0;
    ch[id].size_reg = 0;
    ch[id].start_clear_reg = 0;
    int_port.write(false);

    ch[id].state = IDLE;
    ch[id].current_count = 0;
    ch[id].data_buffer = 0;

    wait();  // Wait for reset deactivation

    // 2. FSM Main Loop
    while (true) {
        wait();
        tlm::tlm_generic_payload trans;
        sc_time delay = SC_ZERO_TIME;

        switch (ch[id].state) {
            case IDLE:
                if (ch[id].start_clear_reg == 1) {
                    ch[id].current_count = 0;
                    if (ch[id].size_reg > 0)
                        ch[id].state = READ_MEM;
                    else {
                        int_port.write(true);
                        ch[id].state = WAIT_CLEAR;
                    }
                }
                break;

            case READ_MEM:
                trans.set_command(tlm::TLM_READ_COMMAND);
                trans.set_address(ch[id].source_reg + ch[id].current_count);
                trans.set_data_ptr((unsigned char *)&ch[id].data_buffer);
                trans.set_data_length(4);
                trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                m_port->b_transport(trans, delay);
                ch[id].state = WRITE_MEM;
                break;

            case WRITE_MEM:
                trans.set_command(tlm::TLM_WRITE_COMMAND);
                trans.set_address(ch[id].target_reg + ch[id].current_count);
                trans.set_data_ptr((unsigned char *)&ch[id].data_buffer);
                trans.set_data_length(4);
                trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                m_port->b_transport(trans, delay);

                ch[id].current_count += 4;

                if (ch[id].current_count < ch[id].size_reg) {
                    ch[id].state = READ_MEM;
                } else {
                    int_port.write(true);
                    ch[id].state = WAIT_CLEAR;
                }
                break;

            case WAIT_CLEAR:
                if (ch[id].start_clear_reg == 0) {
                    int_port.write(false);
                    ch[id].state = IDLE;
                }
                break;
        }
    }
}