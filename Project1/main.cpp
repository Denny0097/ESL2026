#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h" // 引入 Multi Socket
#include "DMA2.h"
#include <map>

// ===========================================================================
// ARMv8-0 (CPU_0)
// ===========================================================================
SC_MODULE(CPU_0) {
    tlm_utils::simple_initiator_socket<CPU_0> master_dma; // 連接 DMA2 暫存器
    tlm_utils::simple_initiator_socket<CPU_0> master_mem; // 連接 MEM (存取旗標與資料)
    sc_in<bool> intr_in;

    void process() {
        wait(20, SC_NS); // wait for turn on

        // Step A: initialization DRAM2
        uint32_t data = 0x01234567; mem_write(0x200400, data);
                 data = 0x89ABCDEF; mem_write(0x200404, data);
        cout << "@" << sc_time_stamp() << " [ARMv8-0] RAM2 initialized" << endl;

        // Step B: repeat 3 times data transmission
        for (int i = 0; i < 3; i++) {
            // Step C: monitor 0x200800 until equal 0 (read MEM)
            while (mem_read(0x200800) != 0) wait(10, SC_NS);
            cout << "@" << sc_time_stamp() << " [ARMv8-0] flag == 0, DMA CH0 " << i+1 << " rounds" << endl;

            // set DMA CH0 (write DMA)
            dma_write(0x100000, 0x200400); // SRC = RAM2
            dma_write(0x100004, 0x300000); // DST = RAM3
            dma_write(0x100008, 1024);     // SIZE
            dma_write(0x10000C, 1);        // START

            // waiting for intr
            while (intr_in.read() == false) wait(10, SC_NS);
            dma_write(0x10000C, 0);        // clear intr
            cout << "@" << sc_time_stamp() << " [ARMv8-0] DMA CH0 completed, set flag to 1" << endl;

            // Step D: setting flag to 1 (write MEM)
            mem_write(0x200800, 1);
        }
        cout << "@" << sc_time_stamp() << " [ARMv8-0] completed" << endl;
    }

    // read mem by master_dma 
    uint32_t mem_read(uint64_t addr) {
        uint32_t data; tlm::tlm_generic_payload trans; sc_time delay = SC_ZERO_TIME;
        trans.set_command(tlm::TLM_READ_COMMAND); trans.set_address(addr);
        trans.set_data_ptr((unsigned char*)&data); trans.set_data_length(4);
        master_mem->b_transport(trans, delay); wait(delay); return data;
    }
    void mem_write(uint64_t addr, uint32_t data) {
        tlm::tlm_generic_payload trans; sc_time delay = SC_ZERO_TIME;
        trans.set_command(tlm::TLM_WRITE_COMMAND); trans.set_address(addr);
        trans.set_data_ptr((unsigned char*)&data); trans.set_data_length(4);
        master_mem->b_transport(trans, delay); wait(delay);
    }
    // write mem by master_dma 
    void dma_write(uint64_t addr, uint32_t data) {
        tlm::tlm_generic_payload trans; sc_time delay = SC_ZERO_TIME;
        trans.set_command(tlm::TLM_WRITE_COMMAND); trans.set_address(addr);
        trans.set_data_ptr((unsigned char*)&data); trans.set_data_length(4);
        master_dma->b_transport(trans, delay); wait(delay);
    }
    SC_CTOR(CPU_0) { SC_THREAD(process); }
};

// ===========================================================================
// ARMv8-1 (CPU_1)
// ===========================================================================
SC_MODULE(CPU_1) {
    tlm_utils::simple_initiator_socket<CPU_1> master_dma;
    tlm_utils::simple_initiator_socket<CPU_1> master_mem;
    sc_in<bool> intr_in;

    void process() {
        wait(20, SC_NS); // wait for turn on

        // Step A: initialization
        uint32_t data = 0xFEDCBA98; mem_write(0x300400, data);
                 data = 0x76543210; mem_write(0x300404, data);
        cout << "@" << sc_time_stamp() << " [ARMv8-1] RAM3 initialized" << endl;

        // Step B: repeat 3 times data transmission
        for (int i = 0; i < 3; i++) {
            // Step C: monitor 0x200800 until equal to 1
            while (mem_read(0x200800) != 1) wait(10, SC_NS);
            cout << "@" << sc_time_stamp() << " [ARMv8-1] flag == 1, DMA CH1 " << i+1 << " rounds" << endl;

            // sets DMA CH1
            dma_write(0x100010, 0x300400); // SRC = RAM3
            dma_write(0x100014, 0x200000); // DST = RAM2
            dma_write(0x100018, 1024);     // SIZE
            dma_write(0x10001C, 1);        // START

            // waitting for intr
            while (intr_in.read() == false) wait(10, SC_NS);
            dma_write(0x10001C, 0);        // clear intr
            cout << "@" << sc_time_stamp() << " [ARMv8-1] DMA CH1 completed, set flag to 0\n" << endl;

            // Step D: sets flag to 0
            mem_write(0x200800, 0);
        }
        cout << "@" << sc_time_stamp() << " [ARMv8-1] completed" << endl;
        sc_stop();
    }

    uint32_t mem_read(uint64_t addr) { 
        uint32_t data; tlm::tlm_generic_payload trans; sc_time delay = SC_ZERO_TIME;
        trans.set_command(tlm::TLM_READ_COMMAND); trans.set_address(addr);
        trans.set_data_ptr((unsigned char*)&data); trans.set_data_length(4);
        master_mem->b_transport(trans, delay); wait(delay); return data;
    }
    void mem_write(uint64_t addr, uint32_t data) { 
        tlm::tlm_generic_payload trans; sc_time delay = SC_ZERO_TIME;
        trans.set_command(tlm::TLM_WRITE_COMMAND); trans.set_address(addr);
        trans.set_data_ptr((unsigned char*)&data); trans.set_data_length(4);
        master_mem->b_transport(trans, delay); wait(delay);
    }
    void dma_write(uint64_t addr, uint32_t data) {
        tlm::tlm_generic_payload trans; sc_time delay = SC_ZERO_TIME;
        trans.set_command(tlm::TLM_WRITE_COMMAND); trans.set_address(addr);
        trans.set_data_ptr((unsigned char*)&data); trans.set_data_length(4);
        master_dma->b_transport(trans, delay); wait(delay);
    }
    SC_CTOR(CPU_1) { SC_THREAD(process); }
};

// ===========================================================================
// Target Memory Module
// ===========================================================================
SC_MODULE(MEM) {
    // Target(slave) Socket: 2 for DMA，2 for CPU
    tlm_utils::simple_target_socket<MEM> target_dma0;
    tlm_utils::simple_target_socket<MEM> target_dma1;
    tlm_utils::simple_target_socket<MEM> target_cpu0;
    tlm_utils::simple_target_socket<MEM> target_cpu1;
    
    std::map<uint64_t, uint32_t> mem_map; 

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        uint64_t addr = trans.get_address();
        unsigned char* ptr = trans.get_data_ptr();

        if (trans.get_command() == tlm::TLM_READ_COMMAND) {
            uint32_t val = mem_map.count(addr) ? mem_map[addr] : 0x0; 
            *((uint32_t*)ptr) = val;
        } else if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
            mem_map[addr] = *((uint32_t*)ptr);
        }
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    SC_CTOR(MEM) : target_dma0("target_dma0"), target_dma1("target_dma1"),
                   target_cpu0("target_cpu0"), target_cpu1("target_cpu1") { 
        target_dma0.register_b_transport(this, &MEM::b_transport); 
        target_dma1.register_b_transport(this, &MEM::b_transport); 
        target_cpu0.register_b_transport(this, &MEM::b_transport); 
        target_cpu1.register_b_transport(this, &MEM::b_transport); 
        
        // data initialization
        mem_map[0x200400] = 0x01234567; // RAM2
        mem_map[0x200404] = 0x89ABCDEF; // RAM2
        mem_map[0x300400] = 0xFEDCBA98; // RAM3
        mem_map[0x300404] = 0x76543210; // RAM3
        mem_map[0x200800] = 0;          // flag
    }
};

// ===========================================================================
// sc_main
// ===========================================================================
int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> reset("reset");
    sc_signal<bool> intr1("intr1");
    sc_signal<bool> intr2("intr2");

    CPU_0* cpu0 = new CPU_0("cpu0");
    CPU_1* cpu1 = new CPU_1("cpu1");
    DMA2* dma = new DMA2("dma");
    MEM* mem = new MEM("mem");

    // =====================================
    // TLM and Signal Binding
    // =====================================
    
    cpu0->master_dma.bind(dma->slave_p);
    cpu1->master_dma.bind(dma->slave_p);
    
    // CPU to MEM 
    cpu0->master_mem.bind(mem->target_cpu0);
    cpu1->master_mem.bind(mem->target_cpu1);

    // DMA to MEM 
    dma->master_p0.bind(mem->target_dma0); 
    dma->master_p1.bind(mem->target_dma1); 
    
    cpu0->intr_in(intr1);
    cpu1->intr_in(intr2);
    dma->clk(clk);
    dma->reset(reset);
    dma->INT1(intr1);
    dma->INT2(intr2);

    // Simulation Execution
    reset.write(false); 
    sc_start(20, SC_NS);
    reset.write(true);  
    sc_start(); // until cpu1 call sc_stop()

    // Verification
    cout << "\n--- Backdoor Verification in sc_main ---" << endl;
    uint32_t res1 = mem->mem_map[0x300000];
    uint32_t res3 = mem->mem_map[0x200000];

    cout << "[CH0] Check 0x300000 (RAM3): Expected 0x01234567, Got 0x" << hex << res1 << endl;
    cout << "[CH1] Check 0x200000 (RAM2): Expected 0xFEDCBA98, Got 0x" << hex << res3 << endl;

    if (res1 == 0x01234567 && res3 == 0xFEDCBA98) {
        cout << "\n>>> [SUCCESS] Dual-Channel DMA2 Transfer Verified! <<<" << endl;
    } else {
        cout << "\n>>> [FAILED] Errors Detected! <<<" << endl;
    }

    delete cpu0; delete cpu1; delete dma; delete mem;
    return 0;
}