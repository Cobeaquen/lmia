//#include <stdio.h>
#include "LuaHelper.h"

static int runComputer(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    printf("running computer!");

    union uInstruction microInstructions[] = {
                  //ALU       //TB      //FB      //S   //P   //LC   //SEQ       //uADR
            { 0b0000, 0b010, 0b001, 0b0, 0b0, 0b00, 0b0000, 0b0000000 }, // IR := PM
            { 0b0001, 0b111, 0b000, 0b1, 0b1, 0b00, 0b0000, 0b0000100 }, // AR := 0b100
            { 0b0100, 0b111, 0b000, 0b1, 0b0, 0b00, 0b0000, 0b0000010 },
            { 0b0001, 0b111, 0b000, 0b1, 0b1, 0b11, 0b1111, 0b0000111 },
            { 0b1001, 0b000, 0b000, 0b0, 0b0, 0b00, 0b0000, 0b0000000 },

            { 0b0001, 0b111, 0b000, 0b0, 0b0, 0b00, 0b0000, 0b1110001 }, // AR := 0b0000000001110001
            { 0b0000, 0b100, 0b101, 0b0, 0b0, 0b00, 0b0000, 0b0000000 }, // HR := AR
            { 0b0001, 0b111, 0b000, 0b0, 0b0, 0b00, 0b0000, 0b0011010 }, // AR := 0b0000000000011010
            { 0b1010, 0b000, 0b000, 0b0, 0b0, 0b00, 0b0000, 0b0000000 }, // ARHR LSL

            { 0b0000, 0b000, 0b000, 0b0, 0b0, 0b00, 0b1111, 0b0000000 }, // Interrupt
    };

    uint32_t uInstructions[] = {
            0b0000010001000000000000000, // IR := PM
            0b0001111000110000000000100, // AR := 0b100
            0b0100111000100000000000010,
            0b0001111000111111110000111,
            0b1001000000000000000000000,

            0b0001111000000000001110001, // AR := 0b0000000001110001
            0b0000100101000000000000000, // HR := AR
            0b0001111000000000000011010, // AR := 0b0000000000011010
            0b1010000000000000000000000, // ARHR LSL

            0b0000000000000011110000000, // Interrupt
    };

    union Instruction machineInstructions[] = {
            { 0b0001, 0b10, 0b01, 0b00000000 },
    };

    for (int i = 0; i < 1; ++i) {
        writePM(mia, i, machineInstructions[i].byteCode);
    }

    for (int i = 0; i < 10; ++i)
    {
        writeUM(mia, i, uInstructions[i]);
    }

    clock(mia);

    lua_pushfstring(L, "Mia:\nAR: %d", mia->ar);

    return 1;
}

/*
// metatable method for handling "array[index]"
static int array_index(lua_State* L) {
    int** parray = (int**)luaL_checkudata(L, 1, "array");
    int index = luaL_checkinteger(L, 2);
    lua_pushnumber(L, (*parray)[index - 1]);
    return 1;
}

// metatable method for handle "array[index] = value"
static int array_newindex(lua_State* L) {
    int** parray = (int**)luaL_checkudata(L, 1, "array");
    int index = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    (*parray)[index - 1] = value;
    return 0;
}

// create a metatable for our array type
static void create_array_type(lua_State* L) {
    static const luaL_reg array[] = {
       { "__index",  array_index  },
       { "__newindex",  array_newindex  },
       { NULL, NULL }
    };
    luaL_newmetatable(L, "array");
    luaL_openlibs(L, NULL, array, 0);
}
// expose an array to lua, by storing it in a userdata with the array metatable
static int expose_array(lua_State* L, int array[]) {
    int** parray = (int**)lua_newuserdata(L, sizeof(int**));
    *parray = array;
    luaL_getmetatable(L, "array");
    lua_setmetatable(L, -2);
    return 1;
}

// test data
int mydata[] = { 1, 2, 3, 4 };

// test routine which exposes our test array to Lua
static int getarray(lua_State* L) {
    return expose_array(L, mydata);
}

int __declspec(dllexport) __cdecl luaopen_array(lua_State* L) {
    create_array_type(L);

    // make our test routine available to Lua
    lua_register(L, "array", getarray);
    return 0;
}

*/

static int gcMia(lua_State* L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    std::cout << "GARBAGE COLLECTED MIA OBJECT";
    delete *mia;
    return 0;
}

static int initMia(lua_State *L)
{
    static const luaL_Reg regs[] = {
       { "__gc",  gcMia  },
       { NULL, NULL }
    };

    luaL_newmetatable(L, "mi");
    luaL_setfuncs(L, regs, NULL);

    Mia **mia;
    mia = (Mia**)lua_newuserdata(L, sizeof(Mia*));
    *mia = new Mia();

    luaL_getmetatable(L, "mi");
    lua_setmetatable(L, -2);

    initializeMia(*mia);

    return 1;
}

int initializeLua()
{
    lua_State *L;

    L = luaL_newstate();
    if (L == NULL)
    {
        fprintf(stderr, "Lua: cannot initialize\n");
        return -1;
    }

    lua_pushcfunction(L, runComputer);
    lua_setglobal(L, "runComputer");
    lua_pushcfunction(L, initMia);
    lua_setglobal(L, "initMia");
    registerLuaFunctions(L);

    luaL_openlibs(L);

    printf("initialized lua\n");
    luaL_dofile(L, "C:\\Users\\cobea\\source\\repos\\Mia\\Debug\\script.lua");

    lua_close(L);

    return 0;
}

int main() {
    if (initializeLua() != 0)
        return -1;

    return 0;
}