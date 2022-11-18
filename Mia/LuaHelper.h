#ifndef MIA_LUAHELPER_H
#define MIA_LUAHELPER_H

#include "mia.h"
#include <string>
//#include <stdio.h>
#include <iostream>
#include <lua.hpp>

void registerLuaFunctions(lua_State *L);

static int RunASMInstruction(lua_State *L);
static int RunMicroInstruction(lua_State *L);

static int WriteK1(lua_State *L); // adr, val
static int WriteK2(lua_State *L); // adr, val
static int ReadK1(lua_State *L); // adr
static int ReadK2(lua_State *L); // adr

static int ReadPM(lua_State *L);
static int ReadAllPM(lua_State *L);
static int WritePM(lua_State *L);
static int ReadUM(lua_State *L);
static int ReadAllUM(lua_State *L);
static int WriteUM(lua_State *L);

static int GetMiaData(lua_State *L);

static int InsertBreakPoint(lua_State *L);

static int ReadUserInput(lua_State* L);

#endif //MIA_LUAHELPER_H
