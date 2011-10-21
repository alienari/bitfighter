//-----------------------------------------------------------------------------------
//
// Bitfighter - A multiplayer vector graphics space game
// Based on Zap demo relased for Torque Network Library by GarageGames.com
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

#ifndef _GO_FAST_H_
#define _GO_FAST_H_

#include "SimpleLine.h"    // For SimpleLine def
#include "gameObject.h"
#include "gameType.h"
#include "gameNetInterface.h"
#include "gameObjectRender.h"
#include "Colors.h"

namespace Zap
{
class EditorAttributeMenuUI;

class SpeedZone : public SimpleLine
{
   typedef SimpleLine Parent;

private:
   Vector<Point> mPolyBounds;
   U16 mSpeed;             // Speed at which ship is propelled, defaults to defaultSpeed
   bool mSnapLocation;     // If true, ship will be snapped to center of speedzone before being ejected
   
   // Take our basic inputs, pos and dir, and expand them into a three element
   // vector (the three points of our triangle graphic), and compute its extent
   void preparePoints();

   // How are things labeled in the editor? 
   const char *getVertLabel(S32 index) { return index == 0 ? "Location" : "Direction"; }
   const char *getInstructionMsg();

   static EditorAttributeMenuUI *mAttributeMenuUI;      // Menu for attribute editing; since it's static, don't bother with smart pointer

public:
   enum {
      halfWidth = 25,
      height = 64,
      defaultSnap = 0,     // 0 = false, 1 = true

      InitMask     = BIT(0),
      HitMask      = BIT(1),
   };

   SpeedZone();           // Constructor
   virtual ~SpeedZone();  // Destructor

   SpeedZone *clone() const;
   //void copyAttrs(SpeedZone *target);

   static const U16 minSpeed = 500;       // How slow can you go?
   static const U16 maxSpeed = 5000;      // Max speed for the goFast
   static const U16 defaultSpeed = 2000;  // Default speed if none specified

   U16 getSpeed() { return mSpeed; }
   void setSpeed(U16 speed) { mSpeed = speed; }

   bool getSnapping() { return mSnapLocation; }
   void setSnapping(bool snapping) { mSnapLocation = snapping; }

   F32 mRotateSpeed;
   U32 mUnpackInit;  // Some form of counter, to know that it is a rotating speed zone.

   static void generatePoints(const Point &pos, const Point &dir, F32 gridSize, Vector<Point> &points);
   void render();
   S32 getRenderSortValue();

   bool processArguments(S32 argc, const char **argv, Game *game);  // Create objects from parameters stored in level file
   string toString(F32 gridSize) const;

   void onAddedToGame(Game *theGame);
   void computeExtent();                                            // Bounding box for quick collision-possibility elimination

   bool getCollisionPoly(Vector<Point> &polyPoints) const;          // More precise boundary for precise collision detection
   bool collide(GameObject *hitObject);
   void collided(MoveObject *s, U32 stateIndex);
   void idle(GameObject::IdleCallPath path);
   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   ///// Editor methods 
   // Offset lets us drag an item out from the dock by an amount offset from the 0th vertex.  This makes placement seem more natural.
   Point getInitialPlacementOffset(F32 gridSize) { return Point(.15,0); }

   Color getEditorRenderColor() { return Colors::red; }

   void renderEditorItem();

   void onAttrsChanging() { /* Do nothing */ }
   void onGeomChanging() { onGeomChanged(); }
   void onItemDragging() { onGeomChanged(); }
   void onGeomChanged();

#ifndef ZAP_DEDICATED
   // These four methods are all that's needed to add an editable attribute to a class...
   EditorAttributeMenuUI *getAttributeMenu();
   void startEditingAttrs(EditorAttributeMenuUI *attributeMenu);    // Called when we start editing to get menus populated
   void doneEditingAttrs(EditorAttributeMenuUI *attributeMenu);     // Called when we're done to retrieve values set by the menu

   void renderAttributeString(F32 currentScale);
#endif


   void saveItem(FILE *f);

   // Some properties about the item that will be needed in the editor
   const char *getEditorHelpString() { return "Makes ships go fast in direction of arrow. [P]"; }  
   const char *getPrettyNamePlural() { return "GoFasts"; }
   const char *getOnDockName() { return "GoFast"; }
   const char *getOnScreenName() { return "GoFast"; }
   bool hasTeam() { return false; }
   bool canBeHostile() { return false; }
   bool canBeNeutral() { return false; }

   TNL_DECLARE_CLASS(SpeedZone);
};


};

#endif


