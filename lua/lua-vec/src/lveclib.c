/*
** Vector math library
** See Copyright Notice in lua.h
*/


#include <stdlib.h>
#include <math.h>

#define lveclib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

static int vec_new (lua_State *L) {
  float x = (float)lua_tonumber(L, 1);
  float y = (float)lua_tonumber(L, 2);
  lua_pushvec(L, x, y);
  return 1;
}

static int vec_dot (lua_State *L) {
  const float* v1 = luaL_checkvec(L, 1);
  const float* v2 = luaL_checkvec(L, 2);
  lua_pushnumber(L, v1[0]*v2[0] + v1[1]*v2[1]);
  return 1;
}

/* U x V = Ux*Vy-Uy*Vx */
static int vec_cross (lua_State *L) {
  const float* v1 = luaL_checkvec(L, 1);
  const float* v2 = luaL_checkvec(L, 2);
  lua_pushvec(L, v1[0] * v2[1] - v1[1] * v2[0], 0.0f);
  return 1;
}

static int vec_length (lua_State *L) {
  const float* v = luaL_checkvec(L, 1);
  lua_pushnumber(L, sqrtf(v[0]*v[0] + v[1]*v[1]));
  return 1;
}

// Bitfighter method
static int vec_lengthsquared (lua_State *L) {
  const float* v = luaL_checkvec(L, 1);
  lua_pushnumber(L, v[0]*v[0] + v[1]*v[1]);
  return 1;
}

static int vec_normalize (lua_State *L) {
  const float* v = luaL_checkvec(L, 1);
  float s = 1.0f / sqrtf(v[0]*v[0] + v[1]*v[1]);
  lua_pushvec(L, v[0]*s, v[1]*s);
  return 1;
}

// Bitfighter method
static int vec_angleto (lua_State *L) {
  const float* v1 = luaL_checkvec(L, 1);
  const float* v2 = luaL_checkvec(L, 2);
  lua_pushnumber(L, atan2(v2[1] - v1[1], v2[0] - v1[0]));
  return 1;
}

// Bitfighter method
static int vec_distanceto (lua_State *L) {
  const float* v1 = luaL_checkvec(L, 1);
  const float* v2 = luaL_checkvec(L, 2);

  lua_pushnumber(L, sqrt((v2[0] - v1[0])*(v2[0] - v1[0]) + (v2[1] - v1[1])*(v2[1] - v1[1])));
  return 1;
}

static int vec_distsquared (lua_State *L) {
  const float* v1 = luaL_checkvec(L, 1);
  const float* v2 = luaL_checkvec(L, 2);

  lua_pushnumber(L, (v2[0] - v1[0])*(v2[0] - v1[0]) + (v2[1] - v1[1])*(v2[1] - v1[1]));
  return 1;
}

// Added Bitfighter methods
static const luaL_Reg veclib[] = {
  {"new",           vec_new},
  {"dot",           vec_dot},
  {"cross",         vec_cross},
  {"length",        vec_length},
  {"lengthSquared", vec_lengthsquared},
  {"normalize",     vec_normalize},
  {"angleTo",       vec_angleto},
  {"distanceTo",    vec_distanceto},
  {"distSquared",   vec_distsquared},
  {NULL, NULL}
};


/*
** Open veclib
*/
LUALIB_API int luaopen_vec (lua_State *L) {
  luaL_register(L, LUA_VECLIBNAME, veclib);
  
  // numeric constants
  lua_pushvec(L, 0, 0);
  lua_setfield(L, -2, "zero");
  lua_pushvec(L, 1, 1);
  lua_setfield(L, -2, "one");

  lua_pop(L, 1);        // Remove table from stack
  return 1;
}

