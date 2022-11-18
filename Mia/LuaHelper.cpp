#include "LuaHelper.h"

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
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    clock(*mia);
    return 0;
}
static int RunMicroInstruction(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uClock(*mia, 0);
    updateFlags(*mia);

    return 0;
}

static int WriteK1(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint8_t val = luaL_checkinteger(L, 3);

    writeK1(*mia, adr, val);

    return 0;
}

static int WriteK2(lua_State *L) // adr, val
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint8_t val = luaL_checkinteger(L, 3);

    writeK2(*mia, adr, val);

    return 0;
}
static int ReadK1(lua_State *L) // adr
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, readK1(*mia, adr));
    return 1;
}
static int ReadK2(lua_State *L) // adr
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, readK2(*mia, adr));
    return 1;
}
static int ReadPM(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, readPM(*mia, adr));
    return 1;
}
static int ReadAllPM(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);

    lua_newtable(L);
    for (uint16_t i = 0; i < PRIMARY_MEMORY_WORDS; i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, readPM(*mia, i));
        lua_settable(L, -3);
    }

    return 1;
}
static int WritePM(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint16_t val = luaL_checkinteger(L, 3);

    writePM(*mia, adr, val);

    return 0;
}
static int ReadUM(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);

    lua_pushinteger(L, (int)readUM(*mia, adr));
    return 1;
}
static int ReadAllUM(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);

    //lua_pushnumber(L, 69);
    lua_newtable(L);

    for (uint16_t i = 0; i < MICRO_MEMORY_WORDS; i++)
    {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, readUM(*mia, i));
        lua_settable(L, -3);
    }

    return 1;
}
static int WriteUM(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint8_t adr = luaL_checkinteger(L, 2);
    uint32_t val = luaL_checkinteger(L, 3);

    writeUM(*mia, adr, val);

    return 0;
}

static int GetMiaData(lua_State *L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    lua_pushinteger(L, (*mia)->bus);
    lua_pushinteger(L, (*mia)->pc);
    lua_pushinteger(L, (*mia)->upc);
    lua_pushinteger(L, (*mia)->supc);
    lua_pushinteger(L, (*mia)->ir);
    lua_pushinteger(L, (*mia)->asr);
    lua_pushinteger(L, (*mia)->ar);
    lua_pushinteger(L, (*mia)->hr);
    lua_pushinteger(L, (*mia)->gr0);
    lua_pushinteger(L, (*mia)->gr1);
    lua_pushinteger(L, (*mia)->gr2);
    lua_pushinteger(L, (*mia)->gr3);
    lua_pushinteger(L, (*mia)->status.flags);
    lua_pushinteger(L, (*mia)->lc);
    lua_pushinteger(L, (*mia)->uIR);
    lua_pushinteger(L, (*mia)->clockCycles);
    return 16;
}

int InsertBreakPoint(lua_State* L)
{
    Mia** mia;
    mia = (Mia**)lua_touserdata(L, 1);
    uint16_t adr = luaL_checkinteger(L, 2);
    (*mia)->breakPoints.push_back(adr);
    return 0;
}

int ReadUserInput(lua_State* L)
{
    std::string input;
    std::getline(std::cin, input);
    lua_pushstring(L, input.c_str());
    return 1;
}