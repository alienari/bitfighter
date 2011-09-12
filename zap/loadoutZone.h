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

#ifndef _LOADOUTZONE_H_6742_
#define _LOADOUTZONE_H_6742_

#include "gameObject.h"
#include "gameType.h"
#include "gameNetInterface.h"
#include "gameObjectRender.h"
#include "polygon.h"

namespace Zap
{

class LoadoutZone : public EditorPolygon
{
private:
   typedef GameObject Parent;

public:
   LoadoutZone();    // C++ constructor
   LoadoutZone *clone() const;

   void render();
   S32 getRenderSortValue();
   bool processArguments(S32 argc, const char **argv, Game *game);
   void onAddedToGame(Game *theGame);

   bool getCollisionPoly(Vector<Point> &polyPoints) const;     // More precise boundary for precise collision detection
   bool collide(GameObject *hitObject);
   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   /////
   // Editor methods
   const char *getEditorHelpString() { return "Area to finalize ship modifications.  Each team should have at least one."; }
   const char *getPrettyNamePlural() { return "Loadout zones"; }
   const char *getOnDockName() { return "Loadout"; }
   const char *getOnScreenName() { return "Loadout"; }
   string toString(F32 gridSize) const;

   void renderEditor(F32 currentScale);
   void renderDock();

   /////
   // Lua interface

   LoadoutZone(lua_State *L) { /* Do nothing */ };    //  Lua constructor
   GameObject *getGameObject() { return this; }          // Return the underlying GameObject

   static const char className[];                        // Class name as it appears to Lua scripts
   static Lunar<LoadoutZone>::RegType methods[];

   S32 getClassID(lua_State *L) { return returnInt(L, LoadoutZoneTypeNumber); }

   TNL_DECLARE_CLASS(LoadoutZone);
private:
   void push(lua_State *L) {  Lunar<LoadoutZone>::push(L, this); }
};

};


#endif