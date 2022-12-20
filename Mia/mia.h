#ifndef MIA_MIA_H
#define MIA_MIA_H

#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <vector>
//#include "MiaDebug.h"

#define MIA_JIT

#define PRIMARY_MEMORY_WORDS 256
#define MICRO_MEMORY_WORDS 256

#define k1_SIZE 20
#define k2_SIZE 4

// Status flag bit masks and offsets
#define ZF_OFFSET 6
#define NF_OFFSET 7
#define CF_OFFSET 0
#define OF_OFFSET 11

#define ZF_BITMASK (1 << ZF_OFFSET)
#define NF_BITMASK (1 << NF_OFFSET)
#define CF_BITMASK (1 << CF_OFFSET)
#define OF_BITMASK (1 << OF_OFFSET)

// Micro instruction bit masks
#define ALU_OFFSET 21
#define TB_OFFSET 18
#define FB_OFFSET 15
#define S_OFFSET 14
#define P_OFFSET 13
#define LC_OFFSET 11
#define SEQ_OFFSET 7
#define U_ADR_OFFSET 0

#define ALU_BITMASK (0b1111 << ALU_OFFSET)
#define TB_BITMASK (0b111 << TB_OFFSET)
#define FB_BITMASK (0b111 << FB_OFFSET)
#define S_BITMASK (0b1 << S_OFFSET)
#define P_BITMASK (0b1 << P_OFFSET)
#define LC_BITMASK (0b11 << LC_OFFSET)
#define SEQ_BITMASK (0b1111 << SEQ_OFFSET)
#define U_ADR_BITMASK (0b1111111 << U_ADR_OFFSET)

// Machine instruction bit masks
#define OP_OFFSET 12
#define GRX_OFFSET 10
#define M_OFFSET 8
#define A_OFFSET 0

#define OP_BITMASK (0b1111 << OP_OFFSET)
#define GRX_BITMASK (0b11 << GRX_OFFSET)
#define M_BITMASK (0b11 << M_OFFSET)
#define A_BITMASK (0b11111111 << A_OFFSET)

union MInstructionFields
{
    struct
    {
        uint8_t OP : 4;
        uint8_t GRx : 2;
        uint8_t M : 2;
        uint8_t adr : 8;
    };
    uint16_t byteCode;
};

union Instruction
{
    struct
    {
        union MInstructionFields fields;
        uint16_t operand : 16;
    };
    uint32_t byteCode;
};

union uInstruction
{
    struct // 25 bits used
    {
        uint8_t alu : 4;
        uint8_t tb : 3;
        uint8_t fb : 3;
        uint8_t s : 1;
        uint8_t p : 1;
        uint8_t lc : 2;
        uint8_t seq : 4;
        uint8_t uAdr : 7;
        uint8_t unUsed : 7;
    };
    uint32_t byteCode;
};

union Flags
{
    struct
    {
        uint8_t Z : 1;
        uint8_t N : 1;
        uint8_t O : 1;
        uint8_t C : 1;
        uint8_t L : 1;
    };
    uint8_t flags;
};

struct Mia
{
    uint16_t pm[PRIMARY_MEMORY_WORDS];
    uint32_t um[MICRO_MEMORY_WORDS];

    int16_t bus;

    uint8_t k1[k1_SIZE];
    uint8_t k2[k2_SIZE];

    uint8_t pc;
    uint16_t upc;
    uint16_t supc;

    //union MInstructionFields ir;
    uint16_t ir;
    uint8_t asr;
    int16_t ar;
    int16_t hr;

    int16_t gr0;
    int16_t gr1;
    int16_t gr2;
    int16_t gr3;

    union Flags status;
    union Flags futureStatus;

    uint8_t lc;
    uint32_t uIR;

    std::vector<uint16_t> breakPoints;
    uint32_t clockCycles;
    //MiaDebug debugger;
    //union uInstruction uIR;
};

//void setupTables();
void initializeMia(Mia*);

void clearPM(Mia* mia);
void clearUM(Mia* mia);

uint16_t readPM(Mia* mia, uint8_t abs_adr);
void writePM(Mia* mia, uint8_t abs_adr, uint16_t value);
uint32_t readUM(Mia* mia, uint8_t abs_adr);
void writeUM(Mia* mia, uint8_t abs_adr, uint32_t value);

void writeK1(Mia* mia, uint8_t abs_adr, uint8_t value);
uint8_t readK1(Mia* mia, uint8_t abs_adr);
void writeK2(Mia* mia, uint8_t abs_adr, uint8_t value);
uint8_t readK2(Mia* mia, uint8_t abs_adr);

void uClock(Mia* mia, uint8_t* halt);
void clock(Mia* mia);

uint32_t fetchUM(Mia* mia);
//uint32_t fetchPM(Mia* mia);

// Instructions

/*
std::vector<std::function<void(Mia* mia)>> ALUInstructionTable;
std::vector<std::function<void(Mia* mia)>> TBTable;
std::vector<std::function<void(Mia* mia)>> FBTable;
std::vector<std::function<void(Mia* mia)>> PBitTable;
std::vector<std::function<uint8_t(Mia* mia)>> SEQInstruction;
std::vector<std::function<void(Mia* mia)>> LCInstructionTable;
*/

// ALU
void nopNF(Mia* mia);
void busToARNF(Mia* mia);
void busToAR1CNF(Mia* mia);
void arResetF(Mia* mia); // flags
void addF(Mia* mia); // flags
void subF(Mia* mia); // flags
void andF(Mia* mia); // flags
void orF(Mia* mia); // flags
void addNF(Mia* mia);
void lslF(Mia* mia); // flags
void arhrlslF(Mia* mia); // flags
void asrF(Mia* mia); // flags
void arhrasrF(Mia* mia); // flags
void lsrF(Mia* mia); // flags
void rtlF(Mia* mia); // flags
void arhrrtlF(Mia* mia); // flags

// FB
void FB_None(Mia* mia);
void FB_IR(Mia* mia);
void FB_PM(Mia* mia);
void FB_PC(Mia* mia);
void FB_HR(Mia* mia);
void FB_GRx(Mia* mia);
void FB_ASR(Mia* mia);

// TB
void TB_None(Mia* mia);
void TB_IR(Mia* mia);
void TB_PM(Mia* mia);
void TB_PC(Mia* mia);
void TB_AR(Mia* mia);
void TB_HR(Mia* mia);
void TB_GRx(Mia* mia);
void TB9To24(Mia* mia);

void toBus(Mia* mia, int16_t value);

// SEQ
uint8_t uPCCount(Mia* mia);
uint8_t uPCFromK1(Mia* mia);
uint8_t uPCFromK2(Mia* mia);
uint8_t uPCReset(Mia* mia);
uint8_t uPCJumpZ0(Mia* mia);
uint8_t uPCJump(Mia* mia);
uint8_t uPCSubroutine(Mia* mia);
uint8_t uPCReturn(Mia* mia);
uint8_t uPCJumpZ1(Mia* mia);
uint8_t uPCJumpN1(Mia* mia);
uint8_t uPCJumpC1(Mia* mia);
uint8_t uPCJumpO1(Mia* mia);
uint8_t uPCJumpL1(Mia* mia);
uint8_t uPCJumpC0(Mia* mia);
uint8_t uPCJumpO0(Mia* mia);
uint8_t uPCResetHalt(Mia* mia);

// P
void PCCount(Mia* mia);
void PCNone(Mia* mia);

// LC
void LCNone(Mia* mia);
void LCCount(Mia* mia);
void LCFromBus(Mia* mia);
void LCFromuADR(Mia* mia);

// Helper functions

inline uint16_t getHostFlags();

void updateFlags(Mia* mia);

unsigned getField(unsigned all, unsigned bitMask, unsigned offset);
unsigned setField(unsigned all, unsigned bitMask, unsigned offset, unsigned value);

uint8_t getALUField(unsigned IR);
uint8_t getTBField(unsigned IR);
uint8_t getFBField(unsigned IR);
uint8_t getSField(unsigned IR);
uint8_t getPField(unsigned IR);
uint8_t getLCField(unsigned IR);
uint8_t getSEQField(unsigned IR);
uint8_t getUADRField(unsigned IR);

unsigned setALUField(unsigned IR, uint8_t value);
unsigned setTBField(unsigned IR, uint8_t value);
unsigned setFBField(unsigned IR, uint8_t value);
unsigned setSField(unsigned IR, uint8_t value);
unsigned setPField(unsigned IR, uint8_t value);
unsigned setLCField(unsigned IR, uint8_t value);
unsigned setSEQField(unsigned IR, uint8_t value);
unsigned setUADRField(unsigned IR, uint8_t value);

unsigned getOPField(unsigned IR);
unsigned getGRxField(unsigned IR);
unsigned getMField(unsigned IR);
unsigned getAField(unsigned IR);

#endif //MIA_MIA_H
