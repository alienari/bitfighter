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

#ifndef JOYSTICKRENDER_H_
#define JOYSTICKRENDER_H_

#include "tnlTypes.h"
#include "keyCode.h"
#include "Point.h"
#include "Joystick.h"

namespace Zap
{

class JoystickRender
{
private:
   static const TNL::S32 roundButtonRadius = 9;
   static const TNL::S32 rectButtonWidth = 24;
   static const TNL::S32 rectButtonHeight = 17;
   static const TNL::S32 smallRectButtonWidth = 19;

public:
   JoystickRender();
   virtual ~JoystickRender();


   static void renderControllerButton(TNL::F32 x, TNL::F32 y, KeyCode keyCode, bool activated, TNL::S32 offset = 0);
   static TNL::S32 getControllerButtonRenderedSize(KeyCode keyCode);

   static void renderDPad(Point center, TNL::F32 radius, bool upActivated, bool downActivated, bool leftActivated,
         bool rightActivated, const char *msg1, const char *msg2);
   static void renderSmallRectButton(Point loc, const char *label, AlignType align, bool activated);
   static void renderRectButton(Point loc, const char *label, AlignType align, bool activated);
   static void renderRoundButton(Point loc, const char *label, AlignType align, bool activated);

   static inline void setButtonColor(bool activated);
};

} /* namespace Zap */

#endif /* JOYSTICKRENDER_H_ */
