/*
 * Input.h
 *
 *  Created on: Jun 12, 2011
 *      Author: noonespecial
 */
#ifndef INPUT_H_
#define INPUT_H_

#include "tnlTypes.h"
#include "keyCode.h"
#include "ConfigEnum.h"

#include "SDL/SDL.h"

using namespace TNL;

namespace Zap {

class ClientGame;

class Event 
{
private:
   static void setMousePos(S32 x, S32 y, DisplayMode mode);
   static void updateJoyAxesDirections(U32 axisMask, S16 value);

   static void keyCodeUp(KeyCode keyCode);
   static void keyCodeDown(KeyCode keyCode, char ascii);

public:
   Event();
   virtual ~Event();

   static void onEvent(ClientGame *game, SDL_Event* event);

   static void onInputFocus();
   static void onInputBlur();
   static void onKeyDown(ClientGame *game, SDLKey key, SDLMod mod, U16 unicode);
   static void onKeyUp(SDLKey key, SDLMod mod, U16 unicode);
   static void onMouseFocus();
   static void onMouseBlur();
   static void onMouseMoved(S32 x, S32 y, DisplayMode mode);
   static void onMouseWheel(bool Up, bool Down);  //Not implemented
   static void onMouseButtonDown(S32 x, S32 y, KeyCode keyCode, DisplayMode mode);
   static void onMouseButtonUp(S32 x, S32 y, KeyCode keyCode, DisplayMode mode);
   static void onJoyAxis(U32 joystickType, U8 which, U8 axis, S16 value);
   static void onJoyButtonDown(U8 which, U8 button, U32 joystickType);
   static void onJoyButtonUp(U8 which, U8 button, U32 joystickType);
   static void onJoyHat(U8 which, U8 hat, U8 directionMask);
   static void onJoyBall(U8 which, U8 ball, S16 xrel, S16 yrel);
   static void onMinimize();
   static void onRestore();
   static void onResize(S32 w, S32 h, F32 winSizeFact);
   static void onExpose();
   static void onExit();
   static void onUser(U8 type, S32 code, void* data1, void* data2);
};

}
#endif /* INPUT_H_ */
