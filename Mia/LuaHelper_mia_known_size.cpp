#include "LuaHelper.h"
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

void registerLuaFunctions(lua_State *L)
{
    lua_pushcfunction(L, RunASMInstruction);
    lua_setglobal(L, "RunASMInstruction");

    lua_pushcfunction(L, RunMicroInstruction);
    lua_setglobal(L, "RunMicroInstruction");

    lua_pushcfunction(L, WriteK1);
    lua_setglobal(L, "WriteK1");

    lua_pushcfunction(L, WriteK2);
    lua_setglobal(L, "WriteK2");

    lua_pushcfunction(L, ReadK1);
    lua_setglobal(L, "ReadK1");

    lua_pushcfunction(L, ReadK2);
    lua_setglobal(L, "ReadK2");

    lua_pushcfunction(L, ReadPM);
    lua_setglobal(L, "ReadPM");

    lua_pushcfunction(L, ReadAllPM);
    lua_setglobal(L, "ReadAllPM");

    lua_pushcfunction(L, WritePM);
    lua_setglobal(L, "WritePM");

    lua_pushcfunction(L, ReadUM);
    lua_setglobal(L, "ReadUM");

    lua_pushcfunction(L, ReadAllUM);
    lua_setglobal(L, "ReadAllUM");

    lua_pushcfunction(L, WriteUM);
    lua_setglobal(L, "WriteUM");

    lua_pushcfunction(L, GetMiaData);
    lua_setglobal(L, "GetMiaData");

    lua_pushcfunction(L, InsertBreakPoint);
    lua_setglobal(L, "InsertBreakPoint");

    lua_pushcfunction(L, ReadUserInput);
    lua_setglobal(L, "ReadUserInput");

    //luaopen_array(L);
}

static int RunASMInstruction(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    clock(mia);
    return 0;
}
static int RunMicroInstruction(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uClock(mia, 0);
    updateFlags(mia);

    return 0;
}

static int WriteK1(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint8_t val = luaL_checkinteger(L, 3);

    writeK1(mia, adr, val);

    return 0;
}

static int WriteK2(lua_State *L) // adr, val
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint8_t val = luaL_checkinteger(L, 3);

    writeK2(mia, adr, val);

    return 0;
}
static int ReadK1(lua_State *L) // adr
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, readK1(mia, adr));
    return 1;
}
static int ReadK2(lua_State *L) // adr
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, readK2(mia, adr));
    return 1;
}

static int ReadPM(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, readPM(mia, adr));
    return 1;
}
static int ReadAllPM(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);

    lua_newtable(L);
    for (uint16_t i = 0; i < PRIMARY_MEMORY_WORDS; i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, readPM(mia, i));
        lua_settable(L, -3);
    }

    return 1;
}
static int WritePM(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint16_t val = luaL_checkinteger(L, 3);

    writePM(mia, adr, val);

    return 0;
}
static int ReadUM(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, (int)readUM(mia, adr));
    return 1;
}
static int ReadAllUM(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);

    //lua_pushnumber(L, 69);
    lua_newtable(L);

    for (uint16_t i = 0; i < MICRO_MEMORY_WORDS; i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, readUM(mia, i));
        lua_settable(L, -3);
    }

    return 1;
}
static int WriteUM(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint32_t val = luaL_checkinteger(L, 3);

    writeUM(mia, adr, val);

    return 0;
}

static int GetMiaData(lua_State *L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    lua_pushinteger(L, mia->bus);
    lua_pushinteger(L, mia->pc);
    lua_pushinteger(L, mia->upc);
    lua_pushinteger(L, mia->supc);
    lua_pushinteger(L, mia->ir);
    lua_pushinteger(L, mia->asr);
    lua_pushinteger(L, mia->ar);
    lua_pushinteger(L, mia->hr);
    lua_pushinteger(L, mia->gr0);
    lua_pushinteger(L, mia->gr1);
    lua_pushinteger(L, mia->gr2);
    lua_pushinteger(L, mia->gr3);
    lua_pushinteger(L, mia->status.flags);
    lua_pushinteger(L, mia->lc);
    lua_pushinteger(L, (int)mia->uIR);
    return 15;
}

int InsertBreakPoint(lua_State* L)
{
    Mia* mia;
    mia = (Mia*)lua_touserdata(L, 1);
    uint16_t adr = luaL_checkinteger(L, 2);
    mia->breakPoints.push_back(adr);
    return 0;
}

int ReadUserInput(lua_State* L)
{
    std::string input;
    std::getline(std::cin, input);
    lua_pushstring(L, input.c_str());
    return 1;
}