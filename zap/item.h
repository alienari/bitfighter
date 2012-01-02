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

#ifndef _ITEM_H_
#define _ITEM_H_

#include "gameObject.h"       // Parent class
#include "EditorObject.h"     // Parent class
#include "luaObject.h"        // Parent class

#include "Timer.h"

namespace Zap
{

// A note on terminology here: an "object" is any game object, whereas an "item" is a point object that the player will interact with
// Item is now parent class of MoveItem, EngineeredItem, PickupItem

class Item : public GameObject, public EditorItem, public LuaItem
{
   typedef GameObject Parent;

protected:
   F32 mRadius;

   enum MaskBits {
      InitialMask = Parent::FirstFreeMask << 0,
      ItemChangedMask = Parent::FirstFreeMask << 1,
      ExplodedMask    = Parent::FirstFreeMask << 2,
      FirstFreeMask   = Parent::FirstFreeMask << 3
   };

public:
   Item(const Point &pos = Point(0,0), F32 radius = 1);      // Constructor

   virtual Point getActualPos() const;
   virtual void setActualPos(const Point &p);

   virtual bool getCollisionCircle(U32 stateIndex, Point &point, F32 &radius) const;

   virtual bool processArguments(S32 argc, const char **argv, Game *game);

   virtual U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   virtual void unpackUpdate(GhostConnection *connection, BitStream *stream);

   F32 getRadius();
   virtual void setRadius(F32 radius);

   virtual void renderItem(const Point &pos);      // Generic renderer -- will be overridden
   virtual void render();

   // EditorItem interface
   virtual void renderEditor(F32 currentScale);
   virtual F32 getEditorRadius(F32 currentScale);
   virtual string toString(F32 gridSize) const;

   // LuaItem interface
   virtual S32 getLoc(lua_State *L);
   virtual S32 getRad(lua_State *L);
   virtual S32 getVel(lua_State *L);
   virtual S32 getTeamIndx(lua_State *L);
   virtual S32 isInCaptureZone(lua_State *L);      // Non-moving item is never in capture zone, even if it is!
   virtual S32 isOnShip(lua_State *L);             // Is item being carried by a ship? NO!
   virtual S32 getCaptureZone(lua_State *L);
   virtual S32 getShip(lua_State *L);
   virtual GameObject *getGameObject();            // Return the underlying GameObject
};


////////////////////////////////////////
////////////////////////////////////////

class Core : public Item
{

typedef Item Parent;

private:
   static const U32 CoreStartWidth = 50;
   static const U32 CoreMinWidth = 10;
   static const U32 CoreStartingHitPoints = 15;

   bool hasExploded;
   U32 mHitPoints;

public:
   Core();     // Constructor  
   Core *clone() const;

   void renderItem(const Point &pos);
   bool getCollisionPoly(Vector<Point> &polyPoints) const;
   bool getCollisionCircle(U32 state, Point &center, F32 &radius) const;
   bool collide(GameObject *otherObject);

   F32 calcCoreWidth() const;

   void damageObject(DamageInfo *theInfo);
   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);
   void onItemExploded(Point pos);

   void setRadius(F32 radius);

   TNL_DECLARE_CLASS(Core);

   ///// Editor methods
   const char *getEditorHelpString();
   const char *getPrettyNamePlural();
   const char *getOnDockName();
   const char *getOnScreenName();

   F32 getEditorRadius(F32 currentScale);
   void renderDock();

   ///// Lua interface
public:
   Core(lua_State *L);    // Constructor

   static const char className[];

   static Lunar<Core>::RegType methods[];

   S32 getClassID(lua_State *L);

   S32 getHitPoints(lua_State *L);   // Index of current asteroid size (0 = initial size, 1 = next smaller, 2 = ...) (returns int)
   void push(lua_State *L);
};



};

#endif


