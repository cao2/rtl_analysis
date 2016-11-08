#ifndef _MSG_H_
#define _MSG_H_

#include <strstream>
#include <string>

using namespace std;

typedef uint64_t config_t;
const config_t null_cfg = 0;

// Define indices for components.
const uint16_t cpu0 = 1;
const uint16_t cache0 = 2;
const uint16_t cpu1 = 3;
const uint16_t cache1 = 4;
const uint16_t membus = 5;
const uint16_t mem = 6;
const uint16_t bus0 =7;
const uint16_t bus1 =8;
string blk_vector[9] = {"-", "cpu0", "cache0", "cpu1", "cache1", "membus","mem","bus0","bus1"};

// Define commands
typedef uint16_t command_t;
const command_t rd = 1;
const command_t wt = 2;
const command_t snp =3;
const command_t wb =4;
const command_t pwr=5;

string cmd_vector[6] = {"-", "rd", "wt", "snp", "wb","pwr"};

// Define the ranges of memory address spaces.
typedef uint32_t address_t;
const uint32_t NDEF = 0;
const uint32_t INTR = 1;
const uint32_t DMEM = 2;
const uint32_t MMIO = 3;
string addr_vector[4] = {"-", "INTR", "DMEM", "MMIO"};


class message_t
{
public:
    uint32_t src;
    uint32_t dest;
    command_t cmd;
    address_t addr;
    
    config_t pre_cfg, post_cfg;
    
    message_t() {
        src = dest = cmd = addr = NDEF;
        pre_cfg = post_cfg = null_cfg;
    };
    
    message_t(const message_t& other) {
        src = other.src;
        dest = other.dest;
        cmd = other.cmd;
        addr = other.addr;
        pre_cfg = other.pre_cfg;
        post_cfg = other.post_cfg;
    }
    
    message_t& operator=(const message_t& other) {
        src = other.src;
        dest = other.dest;
        cmd = other.cmd;
        addr = other.addr;
        pre_cfg = other.pre_cfg;
        post_cfg = other.post_cfg;
        return *this;
    }
    
    bool operator==(const message_t& other) {
        return (src == other.src &&
                dest == other.dest &&
                cmd == other.cmd &&
                addr == other.addr &&
                pre_cfg == other.pre_cfg &&
                post_cfg == other.post_cfg);
    }
    
    void insert_pre_cfg(uint32_t i) {
        pre_cfg = pre_cfg | (1 << i);
    }
    
    void insert_post_cfg(uint32_t i) {
        post_cfg = post_cfg | (1 << i);
    }
    
    void set_msg(uint16_t source, uint16_t destination, command_t command, address_t address) {
        src = source;
        dest = destination;
        cmd = command;
        addr = address;
    }
    
    std::string toString(void) const
    {
        char str[50];
        sprintf( str, "%u", addr);
        return blk_vector[src] + ":" +  blk_vector[dest] + ":" + cmd_vector[cmd] + ":" + str;
    }
};

#endif
