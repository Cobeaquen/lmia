#include "mia.h"
#include <algorithm>
#include <memoryapi.h>

std::vector<std::function<void(Mia* mia)>> ALUInstructionTable = {
        nopNF, busToARNF, busToAR1CNF, arResetF,
        addF, subF, andF, orF,
        addNF, lslF, arhrlslF, asrF,
        arhrasrF, lsrF, rtlF, arhrrtlF
};
std::vector<std::function<void(Mia* mia)>> TBTable = {
        TB_None, TB_IR, TB_PM, TB_PC,
        TB_AR, TB_HR, TB_GRx, TB9To24
};
std::vector<std::function<void(Mia* mia)>> FBTable = {
        FB_None, FB_IR, FB_PM, FB_PC,
        FB_None, FB_HR, FB_GRx, FB_ASR
};
std::vector<std::function<void(Mia* mia)>> PBitTable = {
        PCNone, PCCount
};
std::vector<std::function<void(Mia* mia)>> LCInstructionTable = {
        LCNone, LCCount, LCFromBus, LCFromuADR
};
std::vector<std::function<uint8_t(Mia* mia)>> SEQInstruction = {
        uPCCount, uPCFromK1, uPCFromK2, uPCReset,
        uPCJumpZ0, uPCJump, uPCSubroutine, uPCReturn,
        uPCJumpZ1, uPCJumpN1, uPCJumpC1, uPCJumpO1,
        uPCJumpL1, uPCJumpC0, uPCJumpO0, uPCResetHalt
};

void initializeMia(Mia* mia)
{
    clearPM(mia);
    clearUM(mia);
    for (int i = 0; i < k1_SIZE; ++i) {
        mia->k1[i] = 0x0;
    }
    for (int i = 0; i < k2_SIZE; ++i) {
        mia->k2[i] = 0x0;
    }

    mia->bus = 0x0;
    mia->ar = 0x0;
    mia->pc = 0x0;
    mia->asr = 0x0;
    mia->ir = 0x0;
    mia->upc = 0x0;
    mia->gr0 = 0x0;
    mia->gr1 = 0x0;
    mia->gr2 = 0x0;
    mia->gr3 = 0x0;
    mia->hr = 0x0;
    mia->lc = 0x0;
    mia->status.flags = 0x0;
    mia->futureStatus.flags = 0x0;
    mia->supc = 0x0;
    mia->uIR = 0x0;
    mia->clockCycles = 0x0;
    //mia->breakPoints = std::vector<uint16_t>();
    //mia->debugger = MiaDebug();
}

void clearPM(Mia* mia)
{
    for (int i = 0; i < PRIMARY_MEMORY_WORDS; ++i)
    {
        mia->pm[i] = 0x0000;
    }
}

void clearUM(Mia* mia)
{
    for (int i = 0; i < MICRO_MEMORY_WORDS; ++i)
    {
        mia->um[i] = 0x00000000;
    }
}

uint16_t readPM(Mia *mia, uint8_t abs_adr)
{
    return mia->pm[abs_adr];
}

void writePM(Mia *mia, uint8_t abs_adr, uint16_t value)
{
    mia->pm[abs_adr] = value;
}

uint32_t readUM(Mia *mia, uint8_t abs_adr)
{
    return mia->um[abs_adr];
}

void writeUM(Mia *mia, uint8_t abs_adr, uint32_t value)
{
    mia->um[abs_adr] = value;
}

void writeK1(Mia* mia, uint8_t abs_adr, uint8_t value)
{
    mia->k1[abs_adr] = value;
}
uint8_t readK1(Mia* mia, uint8_t abs_adr)
{
    return mia->k1[abs_adr];
}
void writeK2(Mia* mia, uint8_t abs_adr, uint8_t value)
{
    mia->k2[abs_adr] = value;

}
uint8_t readK2(Mia* mia, uint8_t abs_adr)
{
    return mia->k2[abs_adr];
}

void clock(Mia* mia)
{
    //Instruction instruction;
    //instruction.byteCode = readPM(mia, mia->pc);
    uint8_t halt = 0;
    while (!halt)
    {
        if (std::find(mia->breakPoints.begin(), mia->breakPoints.end(), mia->upc) != mia->breakPoints.end())
        { // We're at a breakpoint
            break;
        }
        uClock(mia, &halt);
        updateFlags(mia); // Update to new status flags
    }
}

void uClock(Mia* mia, uint8_t* halt)
{
    mia->clockCycles++;
    mia->bus = 0x0;
    //union uInstruction instruction;
    //instruction.byteCode = fetchUM(mia);
    mia->uIR = fetchUM(mia); //instruction.byteCode;

    /*
    uint8_t alu = getALUField(mia->uIR);
    uint8_t tb = getTBField(mia->uIR);
    uint8_t fb = getFBField(mia->uIR);
    uint8_t s = getSField(mia->uIR);
    uint8_t p = getPField(mia->uIR);
    uint8_t lc = getLCField(mia->uIR);
    uint8_t seq = getSEQField(mia->uIR);
    uint8_t uAdr = getUADRField(mia->uIR);
    */
    TBTable[getTBField(mia->uIR)](mia); // Send selected register to bus
    ALUInstructionTable[getALUField(mia->uIR)](mia); // Execute ALU function
    if (getTBField(mia->uIR) == 0b111)
    {
        mia->upc++;
        return; // Sent constant to bus - only ALU and TB instruction fields are used
    }
    FBTable[getFBField(mia->uIR)](mia); // Store bus data in selected register

    PBitTable[getPField(mia->uIR)](mia); // Either increment PC by one or do nothing

    LCInstructionTable[getLCField(mia->uIR)](mia);

    // Get next uPC
    if (halt == nullptr)
        SEQInstruction[getSEQField(mia->uIR)](mia);
    else
        *halt = SEQInstruction[getSEQField(mia->uIR)](mia);
}

uint32_t fetchUM(Mia *mia)
{
    return readUM(mia, mia->upc);
}

void toBus(Mia* mia, const int16_t value)
{
    mia->bus = value;
}


// From bus

void FB_None(Mia* mia) { }
void FB_IR(Mia* mia)
{
    mia->ir = mia->bus;
}
void FB_PM(Mia* mia)
{
    writePM(mia, mia->asr, mia->bus);
}
void FB_PC(Mia* mia)
{
    mia->pc = mia->bus;
}
void FB_HR(Mia* mia)
{
    mia->hr = mia->bus;
}
void FB_GRx(Mia *mia)
{
    if (getSField(mia->uIR))
    { // Use M field
        *(&mia->gr0 + getMField(mia->ir)) = mia->bus;
    }
    else
    { // Use GRx field
        *(&mia->gr0 + getGRxField(mia->ir)) = mia->bus;
    }
}
void FB_ASR(Mia *mia)
{
    mia->asr = mia->bus;
}


// TB functions

void TB_None(Mia* mia) { }
void TB_IR(Mia* mia)
{
    toBus(mia, mia->ir);
}
void TB_PM(Mia* mia)
{
    toBus(mia, readPM(mia, mia->asr));
}
void TB_PC(Mia* mia)
{
    toBus(mia, mia->pc);
}
void TB_AR(Mia* mia)
{
    toBus(mia, mia->ar);
}
void TB_HR(Mia* mia)
{
    toBus(mia, mia->hr);
}
void TB_GRx(Mia* mia)
{
    if (getSField(mia->uIR))
    { // Use M field
        toBus(mia, *(&mia->gr0 + getMField(mia->ir)));
    }
    else
    {
        toBus(mia, *(&mia->gr0 + getGRxField(mia->ir)));
    }
}
void TB9To24(Mia* mia)
{
    toBus(mia, mia->uIR & 0xffff); //((mia->uIR.fb & 0b001) << 15) | (mia->uIR.s << 14) | (mia->uIR.p << 13) | (mia->uIR.lc << 11) | (mia->uIR.seq << 7) | (mia->uIR.uAdr));
}


// ALU operations

void nopNF(Mia *mia)
{

}
void busToARNF(Mia *mia)
{
    mia->ar = mia->bus;
}
void busToAR1CNF(Mia *mia)
{

}
void arResetF(Mia *mia)
{
    mia->ar = 0;
    mia->futureStatus.Z = 1;
    mia->futureStatus.N = 0;
}
void addF(Mia *mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    int16_t bus = mia->bus;
    __asm
    {
        mov ebx, ar
        mov ax, [ebx]
        add ax, bus

        pushf
        pop flags

        mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
    mia->futureStatus.O = (flags & OF_BITMASK) >> OF_OFFSET;
}

void subF(Mia *mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    int16_t bus = mia->bus;
    __asm
    {
            mov ebx, ar
            mov ax, [ebx]
            sub ax, bus

            pushf
            pop flags

            mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
    mia->futureStatus.O = (flags & OF_BITMASK) >> OF_OFFSET;
}
void andF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    int16_t bus = mia->bus;
    __asm
    {
            mov ebx, ar
            mov ax, [ebx]
            and ax, bus

            pushf
            pop flags

            mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
}
void orF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    int16_t bus = mia->bus;
    __asm
    {
            mov ebx, ar
            mov ax, [ebx]
            or ax, bus

            pushf
            pop flags

            mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
}
void addNF(Mia* mia)
{
    mia->ar += mia->bus;
}
void lslF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    __asm
    {
            mov ebx, ar
            mov ax, [ebx]
            shl ax, 0x1

            pushf
            pop flags

            mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
}
void arhrlslF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    int16_t* hr = &mia->hr;
    __asm
    {
            mov ebx, ar
            mov eax, [ebx] ; eax = ar
            mov edx, hr
            mov ecx, [edx] ; ecx = hr

            shl eax, 0x10
            or eax, ecx
            shl eax, 0x1 ; ax = hr

            pushf
            pop flags

            push ax
            shr eax, 0x10
            pop cx

            mov [ebx], ax
            mov [edx], cx
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
}

void asrF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    __asm
    {
            mov ebx, ar
            mov ax, [ebx]
            sar ax, 0x1

            pushf
            pop flags

            mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
}
void arhrasrF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    int16_t* hr = &mia->hr;
    __asm
    {
            mov ebx, ar
            mov eax, [ebx] ; eax = ar
            mov edx, hr
            mov ecx, [edx] ; ecx = hr

            shl eax, 0x10
            or eax, ecx
            sar eax, 0x1 ; ax = hr

            pushf
            pop flags

            push ax
            shr eax, 0x10
            pop cx

            mov [ebx], ax
            mov [edx], cx
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
}
void lsrF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    __asm
    {
            mov ebx, ar
            mov ax, [ebx]
            shr ax, 0x1

            pushf
            pop flags

            mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
}
void rtlF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    __asm
    {
            mov ebx, ar
            mov ax, [ebx]
            rol ax, 0x1

            pushf
            pop flags

            mov [ebx], ax
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
}
void arhrrtlF(Mia* mia)
{
    uint16_t flags;
    int16_t* ar = &mia->ar;
    int16_t* hr = &mia->hr;

    __asm
    {
            mov ebx, ar
            mov eax, [ebx] ; eax = ar
            mov edx, hr
            mov ecx, [edx] ; ecx = hr

            shl eax, 0x10
            or eax, ecx
            rol eax, 0x1 ; ax = hr

            pushf
            pop flags

            push ax
            shr eax, 0x10
            pop cx

            mov [ebx], ax
            mov [edx], cx
    }
    mia->futureStatus.Z = (flags & ZF_BITMASK) >> ZF_OFFSET;
    mia->futureStatus.N = (flags & NF_BITMASK) >> NF_OFFSET;
    mia->futureStatus.C = (flags & CF_BITMASK) >> CF_OFFSET;
}


// P-bit operations

void PCCount(Mia* mia)
{
    mia->pc++;
}
void PCNone(Mia* mia) { }


// LC operations

void LCNone(Mia* mia) {}
void LCCount(Mia* mia)
{
    mia->futureStatus.L = --mia->lc == 0;
}
void LCFromBus(Mia* mia)
{
    mia->lc = mia->bus;
    mia->futureStatus.L = mia->lc == 0;
}
void LCFromuADR(Mia* mia)
{
    mia->lc = getUADRField(mia->uIR);
    mia->futureStatus.L = mia->lc == 0;
}


// SEQ operations

uint8_t uPCCount(Mia* mia)
{
    mia->upc++;
    return 0;
}
uint8_t uPCFromK1(Mia* mia)
{
    mia->upc = readK1(mia, getOPField(mia->ir));
    //mia->upc = mia->k1[mia->ir.OP];
    return 0;
}
uint8_t uPCFromK2(Mia* mia)
{
    mia->upc = readK2(mia, getMField(mia->ir));
    //mia->upc = mia->k2[mia->ir.M];
    return 0;
}
uint8_t uPCReset(Mia* mia)
{
    mia->upc = 0x0;
    return 0;
}
uint8_t uPCJumpZ0(Mia* mia)
{
    if (!mia->status.Z)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCJump(Mia* mia)
{
    mia->upc = getUADRField(mia->uIR);
    return 0;
}
uint8_t uPCSubroutine(Mia* mia) // SuPC:= uPC+1, uPC:= uADR
{
    mia->supc = mia->upc + 1;
    mia->upc = getUADRField(mia->uIR);
    return 0;
}
uint8_t uPCReturn(Mia* mia) // uPC:= SuPC
{
    mia->upc = mia->supc;
    return 0;
}
uint8_t uPCJumpZ1(Mia* mia)
{
    if (mia->status.Z)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCJumpN1(Mia* mia)
{
    if (mia->status.N)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCJumpC1(Mia* mia)
{
    if (mia->status.C)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCJumpO1(Mia* mia)
{
    if (mia->status.O)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCJumpL1(Mia* mia)
{
    if (mia->status.L)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCJumpC0(Mia* mia)
{
    if (!mia->status.C)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCJumpO0(Mia* mia)
{
    if (!mia->status.O)
        mia->upc = getUADRField(mia->uIR);
    else
        mia->upc++;
    return 0;
}
uint8_t uPCResetHalt(Mia* mia)
{
    mia->upc = 0x0;
    return 1;
}

inline uint16_t getHostFlags()
{
    uint16_t flags;
    __asm
    {
        pushf
        pop flags
    }
    return flags;
}

void updateFlags(Mia* mia)
{
    mia->status = mia->futureStatus;
}

unsigned getField(unsigned all, unsigned bitMask, unsigned offset)
{
    return (all & bitMask) >> offset;
}
unsigned setField(unsigned all, unsigned bitMask, unsigned offset, unsigned value)
{
    return (all & ~bitMask) | ((value << offset) & bitMask);
}

uint8_t getALUField(unsigned IR)
{
    return getField(IR, ALU_BITMASK, ALU_OFFSET);
}
uint8_t getTBField(unsigned IR)
{
    return getField(IR, TB_BITMASK, TB_OFFSET);
}
uint8_t getFBField(unsigned IR)
{
    return getField(IR, FB_BITMASK, FB_OFFSET);
}
uint8_t getSField(unsigned IR)
{
    return getField(IR, S_BITMASK, S_OFFSET);
}
uint8_t getPField(unsigned IR)
{
    return getField(IR, P_BITMASK, P_OFFSET);
}
uint8_t getLCField(unsigned IR)
{
    return getField(IR, LC_BITMASK, LC_OFFSET);
}
uint8_t getSEQField(unsigned IR)
{
    return getField(IR, SEQ_BITMASK, SEQ_OFFSET);
}
uint8_t getUADRField(unsigned IR)
{
    return getField(IR, U_ADR_BITMASK, U_ADR_OFFSET);
}

unsigned setALUField(unsigned IR, uint8_t value)
{
    return setField(IR, ALU_BITMASK, ALU_OFFSET, value);
}
unsigned setTBField(unsigned IR, uint8_t value)
{
    return setField(IR, TB_BITMASK, TB_OFFSET, value);
}
unsigned setFBField(unsigned IR, uint8_t value)
{
    return setField(IR, FB_BITMASK, FB_OFFSET, value);
}
unsigned setSField(unsigned IR, uint8_t value)
{
    return setField(IR, S_BITMASK, S_OFFSET, value);
}
unsigned setPField(unsigned IR, uint8_t value)
{
    return setField(IR, P_BITMASK, P_OFFSET, value);
}
unsigned setLCField(unsigned IR, uint8_t value)
{
    return setField(IR, LC_BITMASK, LC_OFFSET, value);
}
unsigned setSEQField(unsigned IR, uint8_t value)
{
    return setField(IR, SEQ_BITMASK, SEQ_OFFSET, value);
}
unsigned setUADRField(unsigned IR, uint8_t value)
{
    return setField(IR, U_ADR_BITMASK, U_ADR_OFFSET, value);
}

unsigned getOPField(unsigned IR)
{
    return getField(IR, OP_BITMASK, OP_OFFSET);
}
unsigned getGRxField(unsigned IR)
{
    return getField(IR, GRX_BITMASK, GRX_OFFSET);
}
unsigned getMField(unsigned IR)
{
    return getField(IR, M_BITMASK, M_OFFSET);
}
unsigned getAField(unsigned IR)
{
    return getField(IR, A_BITMASK, A_OFFSET);
}