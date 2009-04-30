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

#include "UIChat.h"
#include "input.h"
#include "masterConnection.h"
#include "UINameEntry.h"
#include "UIMenus.h"

#include "../glut/glutInclude.h"
#include <stdarg.h>

namespace Zap
{

extern void glColor(Color c, float alpha = 1);

// Constructor
ChatUserInterface::ChatUserInterface()
{
   setMenuID(GlobalChatUI);
   menuTitle = "Global Chat";

   //menuSubTitleColor = Color(1,1,1);
   menuFooter = "Type your message | ENTER to send | ESC exits";

   mColorPtr = 0;

   // Clear out any existing chat messages
   for(U32 i = 0; i < MessageDisplayCount; i++)
   {
      mMessages[i][0] = 0;
      mNicks[i][0] = 0;
   }
   memset(mChatBuffer, 0, sizeof(mChatBuffer));
}

// We received a new incoming chat message...  Add it to the list
void ChatUserInterface::newMessage(const char *nick, bool isPrivate, const char *message, ...)
{
   // Make room for the new message
   for(S32 i = MessageDisplayCount - 1; i > 0; i--)
   {
      strcpy(mMessages[i], mMessages[i-1]);
      strcpy(mNicks[i], mNicks[i-1]);
      mDisplayMessageColor[i] = mDisplayMessageColor[i-1];
      mIsPrivate[i] = mIsPrivate[i-1];
   }
   // Add it to the list
   va_list args;
   va_start(args, message);
   dVsprintf(mMessages[0], sizeof(mMessages[0]), message, args);
   dVsprintf(mNicks[0], sizeof(mNicks[0]), nick, args);
   mIsPrivate[0] = isPrivate;

   // And choose a color
   if (strcmp(nick, gNameEntryUserInterface.getText()) == 0)   // Is this message from us?
      mDisplayMessageColor[0] = Color(1,1,1);                  // If so, use white
   else                                                        // Otherwise...
   {
      if (!mNickColors.count(nick))                            // ...see if we have a color for this nick
         mNickColors[nick] = getNextColor();                   // If not, get a new one

      mDisplayMessageColor[0] = mNickColors[nick];
   }
}


// Retrieve the next available chat text color
Color ChatUserInterface::getNextColor()
{
   enum {
      NumColors = 19,
   };

   static Color colorList[NumColors] = {
      Color(0.55,0.55,0),     Color(1,0.55,0.55),
      Color(0,0.6,0),         Color(0.68,1,0.25),
      Color(0,0.63,0.63),     Color(0.275,0.51,0.71),
      Color(1,1,0),           Color(0.5,0.81,0.37),
      Color(0,0.75,1),        Color(0.93,0.91,0.67),
      Color(1,0.5,1),         Color(1,0.73,0.53),
      Color(0.86,0.078,1),    Color(0.78,0.08,0.52),
      Color(0.93,0.5,0),      Color(0.63,0.32,0.18),
      Color(0.5,1,1),         Color(1,0.73,1),
      Color(0.48,0.41,0.93)
   };

   mColorPtr++;
   if (mColorPtr == NumColors)
      mColorPtr = 0;
   return colorList[mColorPtr];
}

// Also put hooks in all UIs
// Also remember instruction screen, selected menu option
///////////////

void ChatUserInterface::idle(U32 timeDelta)
{
   updateCursorBlink(timeDelta);
}

void ChatUserInterface::render()
{
   if (prevUIs.size())           // If there is an underlying menu...
   {
      prevUIs.last()->render();  // ...render it...

      glColor4f(0, 0, 0, 0.75);  // ... and dim it out a bit, nay, a lot
      glEnable(GL_BLEND);
      glBegin(GL_POLYGON);
      glVertex2f(0, 0);
      glVertex2f(canvasWidth, 0);
      glVertex2f(canvasWidth, canvasHeight);
      glVertex2f(0, canvasHeight);
      glEnd();
      glDisable(GL_BLEND);
   }

   // Draw title, subtitle, and footer
   glColor3f(1, 1, 1);
   drawCenteredString(vertMargin, 30, menuTitle);

   string subtitle = "Not currently connected to game server";
   
   if(gClientGame && gClientGame->getConnectionToServer())
   {
      string name = gClientGame->getConnectionToServer()->getServerName();
      if(name == "")
         subtitle = "";
      else
         subtitle = "Connected to game server \"" + gClientGame->getConnectionToServer()->getServerName() + "\"";
   }

   glColor3f(0, 1, 0);
   drawCenteredString(vertMargin + 35, 18, subtitle.c_str());

   S32 vertFooterPos = canvasHeight - vertMargin - 20;
   drawCenteredString(vertFooterPos, 18, menuFooter);

   // Render incoming chat msgs
   glColor3f(1,1,1);

   U32 y = UserInterface::vertMargin + 60;
   for(S32 i = MessageDisplayCount - 1; i >= 0; i--)
   {
      if(mMessages[i][0])
      {
         glColor(mDisplayMessageColor[i]);

         drawString(UserInterface::horizMargin, y, GlobalChatFontSize, mNicks[i]);
         S32 nickWidth = getStringWidth(GlobalChatFontSize, mNicks[i]);
         if (mIsPrivate[i])
         {
            drawString(UserInterface::horizMargin + nickWidth, y, GlobalChatFontSize, "*");
            nickWidth += getStringWidth(GlobalChatFontSize, "*");
         }
         drawString(UserInterface::horizMargin + nickWidth, y, GlobalChatFontSize, " -> ");
         nickWidth += getStringWidth(GlobalChatFontSize, " -> ");
         drawString(UserInterface::horizMargin + nickWidth, y, GlobalChatFontSize, mMessages[i]);

         y += GlobalChatFontSize + 4;
      }
   }

   // Render outgoing chat message composition line
   const char promptStr[] = "> ";

   S32 horizChatPos = UserInterface::horizMargin + getStringWidth(GlobalChatFontSize, promptStr);
   S32 vertChatPos = vertFooterPos - 45;

   glColor3f(0,1,1);
   drawString(UserInterface::horizMargin, vertChatPos, GlobalChatFontSize, promptStr);

   glColor3f(1,1,1);
   drawStringf(horizChatPos, vertChatPos, GlobalChatFontSize, "%s%s", mChatBuffer, cursorBlink ? "_" : " ");

   // Give user notice that there is no connection to master, and thus chatting is ineffectual
   if(!(gClientGame && gClientGame->getConnectionToMaster() && gClientGame->getConnectionToMaster()->getConnectionState() == NetConnection::Connected))
   {
      glColor3f(1, 0, 0);
      drawCenteredString(200, 20, "Not connected to Master Server");
      drawCenteredString(230, 20, "Chat messages cannot be relayed");
   }
}


void ChatUserInterface::onKeyDown(KeyCode keyCode, char ascii)
{
   if (keyCode == keyOUTGAMECHAT)
   {
      UserInterface::playBoop();
      onEscape();
   }
   else if (keyCode == KEY_ENTER)                 // Submits message
   {
      UserInterface::playBoop();
      issueChat();
   }
   else if (keyCode == KEY_ESCAPE)               // Exits chat
   {
      UserInterface::playBoop();
      onEscape();
   }
   else if (keyCode == KEY_DELETE || keyCode == KEY_BACKSPACE)       // Do backspacey things
   {
      if(mChatCursorPos > 0)
      {
         mChatCursorPos--;
         for(U32 i = mChatCursorPos; mChatBuffer[i]; i++)
            mChatBuffer[i] = mChatBuffer[i+1];
      }
   }
   else if(ascii)                               // Other keys - add key to message
   {
      for(U32 i = sizeof(mChatBuffer) - 2; i > mChatCursorPos; i--)  // If inserting...
         mChatBuffer[i] = mChatBuffer[i-1];                          // ...move chars forward

      // Limit chat messages to the size that can be displayed on the screen

      S32 nickWidth = getStringWidthf(GlobalChatFontSize, "%s%s", gNameEntryUserInterface.getText(), " -> " );    //TODO: Put " -> " into a constant
      if((mChatCursorPos < sizeof(mChatBuffer) - 2 )  && nickWidth + (S32) getStringWidthf(GlobalChatFontSize, "%s%c", mChatBuffer, ascii) < UserInterface::canvasWidth - 2 * horizMargin )
      {
         mChatBuffer[mChatCursorPos] = ascii;
         mChatCursorPos++;
      }
   }
}

// Send chat message
void ChatUserInterface::issueChat()
{
   if(mChatBuffer[0])
   {
      // Send message
      MasterServerConnection *conn = gClientGame->getConnectionToMaster();
      if(conn)
         conn->c2mSendChat(mChatBuffer);

      // And display it locally
      newMessage(gNameEntryUserInterface.getText(), false, mChatBuffer);
   }
   cancelChat();
}

// Clear current message
void ChatUserInterface::cancelChat()
{
   memset(mChatBuffer, 0, sizeof(mChatBuffer));
   mChatCursorPos = 0;
}


extern bool gDisableShipKeyboardInput;

void ChatUserInterface::onActivate()
{
   gDisableShipKeyboardInput = true;       // Keep keystrokes from getting to game
}

void ChatUserInterface::onEscape()
{
   UserInterface::reactivatePrevUI();   //gMainMenuUserInterface
}

ChatUserInterface gChatInterface;

};

