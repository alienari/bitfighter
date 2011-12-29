//-----------------------------------------------------------------------------------
//
// Bitfighter - A multiplayer vector graphics space game
// Based on Zap demo released for Torque Network Library by GarageGames.com
//
// Derivative work copyright (C) 2008-2009 Chris Eykamp
// Original work copyright (C) 2004 GarageGames.com, Inc.
// Other code copyright as noted
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful (and fun!),
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//------------------------------------------------------------------------------------

#ifndef _LUALEVELGENERATOR_H_
#define _LUALEVELGENERATOR_H_

#include "luaObject.h"
#include "gameLoader.h"
#include "tnlLog.h"
#include "oglconsole.h"    // For logging to the console

using namespace std;

namespace Zap
{

class GridDatabase;

class LuaLevelGenerator: public LuaScriptRunner, public LuaObject
{
private:
   GridDatabase *mGridDatabase;

   OGLCONSOLE_Console mConsole;

   LevelLoader *mCaller;
   F32 mGridSize;

   Point getPointFromTable(lua_State *L, int tableIndex, int key, const char *methodName);

public:
   LuaLevelGenerator(const string &scriptName, const string &scriptDir, const Vector<string> &scriptArgs, F32 gridsize, GridDatabase *gridDatabase, 
                     LevelLoader *caller);   // C++ constructor

   LuaLevelGenerator(lua_State *L);      // Lua constructor
   virtual ~LuaLevelGenerator();         // Destructor

   void setPointerToThis();
   void registerClasses();
   void onScriptInitialized();
   bool loadScript();
   bool runScript();      
   string getScriptName();

   bool runGetArgs(string &menuTitle, Vector<MenuItem *> &menuItems);

   void logError(const char *format, ...);

   static const char className[];

   static Lunar<LuaLevelGenerator>::RegType methods[];

   // Lua methods
   virtual S32 addWall(lua_State *L);           // Add a wall
   virtual S32 addItem(lua_State *L);           // Add item using a series of parameters
   virtual S32 addLevelLine(lua_State *L);      // Add raw level line
   virtual S32 setGameTime(lua_State *L);       // Set the time for this level
   virtual S32 pointCanSeePoint(lua_State *L);  // Returns if a point has LOS to another point, given what's on the map so far


   S32 logprint(lua_State *L);
   S32 getMachineTime(lua_State *L) { return returnInt(L, Platform::getRealMilliseconds()); }
   S32 getGridSize(lua_State *L);
   S32 getPlayerCount(lua_State *L);

   // Implement LevelLoader abstract method
   void processLevelLoadLine(int argc, U32 id, const char **argv, GridDatabase *database, bool inEditor, const string &levelFileName);
};


};

#endif

