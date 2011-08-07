////////////////////////
//To do prior to 1.0 release
//
////////////////////////
// Nitnoid
// Make color defs below constant... need to change associated externs too!

// Some time
// Add mouse coords to diagnostics screen, raw key codes

// Long term
// Admin select level w/ preview(?)

//Test:
// Do ships remember their spawn points?  How about robots?
// Does chat now work properly when ship is dead?  no
// Do LuaShip proxies remain constant over time (i.e. does 013 fix for Buvet.bot now work?)
// Make sure things work when ship is deleted.  Do we get nils back (and not crashes)?


// Ideas for interactive help/tutorial:
//=======================================
// Mine: Explodes when ships fly too close
// Beware: Enemy mines are hard to see!        (plant your own w/ the mine layer weapon)
// Teleport: takes you to another location on the map
// Friendly FF: Lets friendly ships pass
// Enemy FF: Lets enemy ships pass - destroy by shooting the base
// Neutral FF: Claim it for your team by repairing with the repair module
// Friendly Turret: Targets enemies, but won't hurt you (on purpose)
// Enemy Turret: Defends enemy teritory.  Destroy with multiple shots
// Neutral turret: Claim it for your team by repairing with the repair module
// Timer shows time left in game
// Heatlh indicator shows health left
// basic controls:  x/x/x/x to move; use 1,2,3 to select weapons; <c> shows overview map
// Messages will appear here -->
// See current game info by pressing [F2]



// Random point in zone, random zone, isInCaptureZone should return actual capture zone
// backport player count stuff

/*
XXX need to document timers, new luavec stuff XXX

/shutdown enhancements: on screen timer after msg dismissed, instant dismissal of local notice, notice in join menu, shutdown after level, auto shutdown when quitting and players connected

 */
/* Fixes after 015a
<h2>Bug Fixes</h2>
<ul>
<li>Fix team bitmatch suicide score. 
<li>Teleporter, added Delay option in levels for teleporters
<li>SoccerBallItem, added individual Pickup=yes or no
<li>Deprecated SoccerPickup parameter -- now stored as an option on the Specials line.  Will be completely removed in 017.  Easiest fix is to load
    level into editor and save; parameter will be properly rewritten.
<li>Reduced CPU usage for overlapping asteroids
<li>LUA added copyMoveFromObject, LUA getCurrLoaduot and getReqLoadout can now be used for ships
</ul>
*/



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

#ifdef _MSC_VER
#pragma warning (disable: 4996)     // Disable POSIX deprecation, certain security warnings that seem to be specific to VC++
#endif

#include "IniFile.h"

#include "tnl.h"
#include "tnlRandom.h"
#include "tnlGhostConnection.h"
#include "tnlNetInterface.h"
#include "tnlJournal.h"

#include "dataConnection.h"

#include "oglconsole.h"

#include "zapjournal.h"

#include <stdarg.h>
#include <math.h>
//#include <stdio.h>      // For logging to console

using namespace TNL;

#include "UI.h"
#include "UIGame.h"
#include "UINameEntry.h"
#include "UIMenus.h"
#include "UIEditor.h"
#include "UIErrorMessage.h"
#include "UIDiagnostics.h"
#include "UICredits.h"
#include "game.h"
#include "gameNetInterface.h"
#include "masterConnection.h"
#include "SoundSystem.h"
#include "sparkManager.h"
#include "input.h"
#include "keyCode.h"
#include "config.h"
#include "md5wrapper.h"
#include "version.h"
#include "Colors.h"
#include "Event.h"
#include "ScreenInfo.h"
#include "Joystick.h"
#include "stringUtils.h"

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

// For writeToConsole() functionality
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <shellapi.h>
#define USE_BFUP
#endif 

#ifdef TNL_OS_MAC_OSX
#include "Directory.h"
#include <unistd.h>
#endif

namespace Zap
{
extern ClientGame *gClientGame1;
extern ClientGame *gClientGame2;

string gHostName;                // Server name used when hosting a game (default set in config.h, set in INI or on cmd line)
string gHostDescr;               // Brief description of host
const char *gWindowTitle = "Bitfighter";

// The following things can be set via command line parameters


#ifdef ZAP_DEDICATED
bool gDedicatedServer = true;    // This will allow us to omit the -dedicated parameter when compiled in dedicated mode
#else
bool gDedicatedServer = false;   // Usually, we just want to play.  If true, we'll be in server-only, no-player mode
#endif

// Handle any md5 requests
md5wrapper md5;


bool gShowAimVector = false;     // Do we render an aim vector?  This should probably not be a global, but until we find a better place for it...
bool gDisableShipKeyboardInput;  // Disable ship movement while user is in menus

CIniFile gINI("dummy");          // This is our INI file.  Filename set down in main(), but compiler seems to want an arg here.

CmdLineSettings gCmdLineSettings;

ConfigDirectories gConfigDirs;

IniSettings gIniSettings;

OGLCONSOLE_Console gConsole;     // For the moment, we'll just have one console for levelgens and bots.  This may change later.


// Some colors -- other candidates include global and local chat colors, which are defined elsewhere.  Include here?
Color gNexusOpenColor(0, 0.7, 0);
Color gNexusClosedColor(0.85, 0.3, 0);
Color gErrorMessageTextColor(1, 0.5, 0.5);
Color gNeutralTeamColor(0.8, 0.8, 0.8);         // Objects that are neutral (on team -1)
Color gHostileTeamColor(0.5, 0.5, 0.5);         // Objects that are "hostile-to-all" (on team -2)
Color gMasterServerBlue(0.8, 0.8, 1);           // Messages about successful master server statii
Color gHelpTextColor(Colors::green);

S32 gMaxPolygonPoints = 32;                     // Max number of points we can have in Nexuses, LoadoutZones, etc.

static const F32 MIN_SCALING_FACT = 0.15f;       // Limits minimum window size

string gServerPassword = "";
string gAdminPassword = "";
string gLevelChangePassword = "";

DataConnection *dataConn = NULL;

Vector<string> gMasterAddress;
Address gBindAddress(IPProtocol, Address::Any, 28000);      // Good for now, may be overwritten by INI or cmd line setting
// Above is equivalent to ("IP:Any:28000")

Vector<StringTableEntry> gLevelSkipList;  // Levels we'll never load, to create a semi-delete function for remote server mgt

ScreenInfo gScreenInfo;

ZapJournal gZapJournal;          // Our main journaling object

string gPlayerPassword;

struct ClientInfo;
ClientInfo gClientInfo;          // Info about the client used for establishing connection to server -- only needed on client side


void exitGame(S32 errcode)
{
#ifdef TNL_OS_XBOX
   extern void xboxexit();
   xboxexit();
#else
   exit(errcode);
#endif
}


// Exit the game, back to the OS
void exitGame()
{
   exitGame(0);
}


// If we can't load any levels, here's the plan...
void abortHosting_noLevels()
{
   if(gDedicatedServer)
   {
      logprintf(LogConsumer::LogError, "No levels found in folder %s.  Cannot host a game.", gConfigDirs.levelDir.c_str());
      logprintf(LogConsumer::ServerFilter, "No levels found in folder %s.  Cannot host a game.", gConfigDirs.levelDir.c_str());
      //printf("No levels were loaded from folder %s.  Cannot host a game.", gLevelDir.c_str());      ==> Does nothing
      exitGame(1);
   }
   else
   {
      ErrorMessageUserInterface *ui = gClientGame->getUIManager()->getErrorMsgUserInterface();

      ui->reset();
      ui->setTitle("HOUSTON, WE HAVE A PROBLEM");
      ui->setMessage(1, "No levels were loaded.  Cannot host a game.");
      ui->setMessage(3, "Check the LevelDir parameter in your INI file,");
      ui->setMessage(4, "or your command-line parameters to make sure");
      ui->setMessage(5, "you have correctly specified a folder containing");
      ui->setMessage(6, "valid level files.");
      ui->setMessage(8, "Trying to load levels from folder:");
      ui->setMessage(9, gConfigDirs.levelDir == "" ? "<<Unresolvable>>" : gConfigDirs.levelDir.c_str());
      ui->activate();
   }
   delete gServerGame;
   gServerGame = NULL;

   HostMenuUserInterface *ui = gClientGame->getUIManager()->getHostMenuUserInterface();
   ui->levelLoadDisplayDisplay = false;
   ui->levelLoadDisplayFadeTimer.clear();

   return;
}


// GCC thinks min isn't defined, VC++ thinks it is
#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#endif

// This is not a very good way of seeding the prng, but it should generate unique, if not cryptographicly secure, streams.
// We'll get 4 bytes from the time, up to 12 bytes from the name, and any left over slots will be filled with unitialized junk.
void seedRandomNumberGenerator(string name)
{
   U32 seconds = Platform::getRealMilliseconds();
   const S32 timeByteCount = 4;
   const S32 totalByteCount = 16;

   S32 nameBytes = min((S32)name.length(), totalByteCount - timeByteCount);     // # of bytes we get from the provided name

   unsigned char buf[totalByteCount];

   // Bytes from the time
   buf[0] = U8(seconds);
   buf[1] = U8(seconds >> 8);
   buf[2] = U8(seconds >> 16);
   buf[3] = U8(seconds >> 24);

   // Bytes from the name
   for(S32 i = 0; i < nameBytes; i++)
      buf[i + timeByteCount] = name.at(i);

   Random::addEntropy(buf, totalByteCount);     // May be some uninitialized bytes at the end of the buffer, but that's ok
}


U32 getServerMaxPlayers()
{
   U32 maxplay;
   if (gCmdLineSettings.maxPlayers > 0)
      maxplay = gCmdLineSettings.maxPlayers;
   else
      maxplay = gIniSettings.maxPlayers;

   if(maxplay > MAX_PLAYERS)
      maxplay = MAX_PLAYERS;

   return maxplay;
}

// Host a game (and maybe even play a bit, too!)
void initHostGame(Address bindAddress, Vector<string> &levelList, bool testMode)
{
   gServerGame = new ServerGame(bindAddress, getServerMaxPlayers(), gHostName.c_str(), testMode);

   gServerGame->setReadyToConnectToMaster(true);
   seedRandomNumberGenerator(gHostName);
   gClientInfo.id.getRandom();                    // Generate a player ID

   // Don't need to build our level list when in test mode because we're only running that one level stored in editor.tmp
   if(!testMode)
   {
      logprintf(LogConsumer::ServerFilter, "----------\nBitfighter server started [%s]", getTimeStamp().c_str());
      logprintf(LogConsumer::ServerFilter, "hostname=[%s], hostdescr=[%s]", gServerGame->getHostName(), gServerGame->getHostDescr());

      logprintf(LogConsumer::ServerFilter, "Loaded %d levels:", levelList.size());
   }

   // Parse all levels, make sure they are in some sense valid, and record some critical parameters
   if(levelList.size())
   {
      gServerGame->buildLevelList(levelList);     // Take levels in gLevelList and create a set of empty levelInfo records
      gServerGame->resetLevelLoadIndex();

      if(gClientGame)
         gClientGame->getUIManager()->getHostMenuUserInterface()->levelLoadDisplayDisplay = true;
   }
   else
   {
      abortHosting_noLevels();
      return;
   }

   // Do this even if there are no levels, so hostGame error handling will be triggered
   gServerGame->hostingModePhase = ServerGame::LoadingLevels;
}


// All levels loaded, we're ready to go
void hostGame()
{
   if(gConfigDirs.levelDir == "")      // Never did resolve a leveldir... no hosting for you!
   {
      abortHosting_noLevels();         // Not sure this would ever get called...
      return;
   }

   gServerGame->hostingModePhase = ServerGame::Hosting;

   for(S32 i = 0; i < gServerGame->getLevelNameCount(); i++)
      logprintf(LogConsumer::ServerFilter, "\t%s [%s]", gServerGame->getLevelNameFromIndex(i).getString(), 
            gServerGame->getLevelFileNameFromIndex(i).c_str());

   if(gServerGame->getLevelNameCount())                  // Levels loaded --> start game!
      gServerGame->cycleLevel(ServerGame::FIRST_LEVEL);  // Start with the first level

   else        // No levels loaded... we'll crash if we try to start a game
   {
      abortHosting_noLevels();
      return;
   }

   HostMenuUserInterface *ui = gClientGame->getUIManager()->getHostMenuUserInterface();

   ui->levelLoadDisplayDisplay = false;
   ui->levelLoadDisplayFadeTimer.reset();

   if(!gDedicatedServer)                  // If this isn't a dedicated server...
      joinGame(Address(), false, true);   // ...then we'll play, too!
}


// do the logic to draw the screen
void display()
{
   glFlush();

   UserInterface::renderCurrent();

   // Render master connection state if we're not connected
   // TODO: should this go elsewhere?
   if(gClientGame && gClientGame->getConnectionToMaster() &&
         gClientGame->getConnectionToMaster()->getConnectionState() != NetConnection::Connected)
   {
      glColor3f(1, 1, 1);
      UserInterface::drawStringf(10, 550, 15, "Master Server - %s", gConnectStatesTable[gClientGame->getConnectionToMaster()->getConnectionState()]);
   }

   // Swap the buffers. This this tells the driver to render the next frame from the contents of the
   // back-buffer, and to set all rendering operations to occur on what was the front-buffer.
   // Double buffering prevents nasty visual tearing from the application drawing on areas of the
   // screen that are being updated at the same time.
   SDL_GL_SwapBuffers();  // Use this if we convert to SDL
}


void gameIdle(U32 integerTime)
{
   if(UserInterface::current)
      UserInterface::current->idle(integerTime);

   if(!(gServerGame && gServerGame->hostingModePhase == ServerGame::LoadingLevels))    // Don't idle games during level load
   {
      if(gClientGame2)
      {
         gIniSettings.inputMode = InputModeJoystick;
         gClientGame1->mUserInterfaceData->get();
         gClientGame2->mUserInterfaceData->set();

         gClientGame = gClientGame2;
         gClientGame->idle(integerTime);

         gIniSettings.inputMode = InputModeKeyboard;
         gClientGame2->mUserInterfaceData->get();
         gClientGame1->mUserInterfaceData->set();
      }
      if(gClientGame1)
      {
         gClientGame = gClientGame1;
         gClientGame->idle(integerTime);
      }
      if(gServerGame)
         gServerGame->idle(integerTime);
   }
}

// This is the master idle loop that gets registered with GLUT and is called on every game tick.
// This in turn calls the idle functions for all other objects in the game.
void idle()
{
   if(gServerGame)
   {
      if(gServerGame->hostingModePhase == ServerGame::LoadingLevels)
         gServerGame->loadNextLevel();
      else if(gServerGame->hostingModePhase == ServerGame::DoneLoadingLevels)
         hostGame();
   }

/*
   static S64 lastTimer = Platform::getHighPrecisionTimerValue(); // accurate, but possible wrong speed when overclocking or underclocking CPU
   static U32 lastTimer2 = Platform::getRealMilliseconds();  // right speed
   static F64 unusedFraction = 0;
   static S32 timerElapsed2 = 0;

   S64 currentTimer = Platform::getHighPrecisionTimerValue();
   U32 currentTimer2 = Platform::getRealMilliseconds();

   if(lastTimer > currentTimer) lastTimer=currentTimer; //Prevent freezing when currentTimer overflow -- seems very unlikely
   if(lastTimer2 > currentTimer2) lastTimer2=currentTimer2;

   F64 timeElapsed = Platform::getHighPrecisionMilliseconds(currentTimer - lastTimer) + unusedFraction;
   S32 integerTime1 = S32(timeElapsed);

   unusedFraction = timeElapsed - integerTime1;
   lastTimer = currentTimer;
   timerElapsed2 = timerElapsed2 + S32(currentTimer2 - lastTimer2) - integerTime1;
   if(timerElapsed2 < 0)  // getHighPrecisionTimerValue going slower then getRealMilliseconds
   {
      integerTime1 += timerElapsed2;
      timerElapsed2 = 0;
   }
   if(timerElapsed2 > 200)  // getHighPrecisionTimerValue going faster then getRealMilliseconds
   {
      integerTime1 += timerElapsed2 - 200;
      timerElapsed2 = 200;
   }
   lastTimer2 = currentTimer2;
   integerTime += integerTime1;
   */

   static S32 integerTime = 0;   // static, as we need to keep holding the value that was set
   static U32 prevTimer = 0;

	U32 currentTimer = Platform::getRealMilliseconds();
	integerTime += currentTimer - prevTimer;
	prevTimer = currentTimer;

	if(integerTime < -500 || integerTime > 5000)
      integerTime = 10;

   U32 sleepTime = 1;

   if( (gDedicatedServer  && integerTime >= S32(1000 / gIniSettings.maxDedicatedFPS)) || 
         (!gDedicatedServer && integerTime >= S32(1000 / gIniSettings.maxFPS)) )
   {
      gameIdle(U32(integerTime));
      display();    // Draw the screen
      integerTime = 0;
      if(!gDedicatedServer)
         sleepTime = 0;      // Live player at the console, but if we're running > 100 fps, we can afford a nap
   }


   // So, what's with all the SDL code in here?  I looked at converting from GLUT to SDL, in order to get
   // a richer set of keyboard events.  Looks possible, but SDL appears to be missing some very handy
   // windowing code (e.g. the ability to resize or move a window) that GLUT has.  So until we find a
   // platform independent window library, we'll stick with GLUT, or maybe go to FreeGlut.
   // Note that moving to SDL will require our journaling system to be re-engineered.
   // Note too that SDL will require linking in SDL.lib and SDLMain.lib, and including the SDL.dll in the EXE folder.

   // SDL requires an active polling loop.  We could use something like the following:
   SDL_Event event;

   while(SDL_PollEvent(&event))
   {
      if (event.type == SDL_QUIT) // Handle quit here
         exitGame();

      Event::onEvent(&event);
   }
   // END SDL event polling


   // Sleep a bit so we don't saturate the system. For a non-dedicated server,
   // sleep(0) helps reduce the impact of OpenGL on windows.

   // If there are no players, set sleepTime to 40 to further reduce impact on the server.
   // We'll only go into this longer sleep on dedicated servers when there are no players.
   if(gDedicatedServer && gServerGame->isSuspended())
      sleepTime = 40;     // The higher this number, the less accurate the ping is on server lobby when empty, but the less power consumed.

   Platform::sleep(sleepTime);

   gZapJournal.processNextJournalEntry();    // Does nothing unless we're playing back a journal...

}  // end idle()


void dedicatedServerLoop()
{
   for(;;)        // Loop forever!
      idle();     // Idly!
}

////////////////////////////////////////
////////////////////////////////////////

// Our logfiles
StdoutLogConsumer gStdoutLog;     // Logs to console, when there is one

FileLogConsumer gMainLog;
FileLogConsumer gServerLog;       // We'll apply a filter later on, in main()

////////////////////////////////////////
////////////////////////////////////////


// Player has selected a game from the QueryServersUserInterface, and is ready to join
void joinGame(Address remoteAddress, bool isFromMaster, bool local)
{
   MasterServerConnection *connToMaster = gClientGame->getConnectionToMaster();
   if(isFromMaster && connToMaster && connToMaster->getConnectionState() == NetConnection::Connected)     // Request arranged connection
   {
      connToMaster->requestArrangedConnection(remoteAddress);
      gClientGame->getUIManager()->getGameUserInterface()->activate();
   }
   else                                                         // Try a direct connection
   {
      GameConnection *gameConnection = new GameConnection(gClientInfo);

      gClientGame->setConnectionToServer(gameConnection);

      if(local)   // We're a local client, running in the same process as the server... connect to that server
      {
         // Stuff on client side, so interface will offer the correct options.
         // Note that if we're local, the passed address is probably a dummy; check caller if important.
         gameConnection->connectLocal(gClientGame->getNetInterface(), gServerGame->getNetInterface());
         gameConnection->setIsAdmin(true);              // Local connection is always admin
         gameConnection->setIsLevelChanger(true);       // Local connection can always change levels

         GameConnection *gc = dynamic_cast<GameConnection *>(gameConnection->getRemoteConnectionObject());

         // Stuff on server side
         if(gc)                              
         {
            gc->setIsAdmin(true);            // Set isAdmin on server
            gc->setIsLevelChanger(true);     // Set isLevelChanger on server

            gc->s2cSetIsAdmin(true);                // Set isAdmin on the client
            gc->s2cSetIsLevelChanger(true, false);  // Set isLevelChanger on the client
            gc->setServerName(gServerGame->getHostName());     // Server name is whatever we've set locally

            gc->setAuthenticated(gClientInfo.authenticated);   // Tell the local host whether we're authenticated... no need to verify
         }
      }
      else        // Connect to a remote server, but not via the master server
         gameConnection->connect(gClientGame->getNetInterface(), remoteAddress);  

      gClientGame->getUIManager()->getGameUserInterface()->activate();
   }
   //if(gClientGame2 && gClientGame != gClientGame2)  // make both client connect for now, until menus works in both clients.
   //{
   //   gClientGame = gClientGame2;
   //   joinGame(remoteAddress, isFromMaster, local);
   //   gClientGame = gClientGame1;
   //}

}



// Disconnect from servers and exit game in an orderly fashion.  But stay connected to the master until we exit the program altogether
void endGame()
{
   // Cancel any in-progress attempts to connect
   if(gClientGame && gClientGame->getConnectionToMaster())
      gClientGame->getConnectionToMaster()->cancelArrangedConnectionAttempt();

   // Disconnect from game server
   if(gClientGame && gClientGame->getConnectionToServer())
      gClientGame->getConnectionToServer()->disconnect(NetConnection::ReasonSelfDisconnect, "");

   delete gServerGame;
   gServerGame = NULL;

   gClientGame->getUIManager()->getHostMenuUserInterface()->levelLoadDisplayDisplay = false;

   if(gDedicatedServer)
      exitGame();
}


extern void saveWindowMode(CIniFile *ini);
extern void saveWindowPosition(S32 x, S32 y);

// Run when we're quitting the game, returning to the OS
void onExit()
{
   endGame();

   if(gClientGame)
      delete gClientGame;     // Has effect of disconnecting from master
   if(gServerGame)
      delete gServerGame;     // Has effect of disconnecting from master

   OGLCONSOLE_Quit();
   SoundSystem::shutdown();
   Joystick::shutdownJoystick();

   // Save settings to capture window position
   saveWindowMode(&gINI);

   // TODO: reimplement window position saving with SDL
   //   if(gIniSettings.displayMode == DISPLAY_MODE_WINDOWED)
   //      saveWindowPosition(glutGet(GLUT_WINDOW_X), glutGet(GLUT_WINDOW_Y));

   saveSettingsToINI(&gINI);    // Writes settings to the INI, then saves it to disk

   NetClassRep::logBitUsage();
   TNL::logprintf("Bye!");

   SDL_QuitSubSystem(SDL_INIT_VIDEO);

   exitGame();
}


// If we're running in dedicated mode, these things need to be set as such.
void setParamsForDedicatedMode()
{
   gCmdLineSettings.clientMode = false;
   gCmdLineSettings.serverMode = true;
   gDedicatedServer = true;
}


// Processed params will be removed from argv
void readFolderLocationParams(Vector<StringPtr> &argv)
{
   // Process command line args  --> see http://bitfighter.org/wiki/index.php?title=Command_line_parameters
   for(S32 i = 0; i < argv.size(); i++)
   {
      S32 argc = argv.size();

      // Assume "args" starting with "-" are actually follow-on params
      bool hasAdditionalArg = (i != argc - 1 && argv[i + 1].getString()[0] != '-');     

      if(!stricmp(argv[i].getString(), "-rootdatadir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify the root data folder with the -rootdatadir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.rootDataDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }


      else if(!stricmp(argv[i].getString(), "-leveldir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify a levels subfolder with the -leveldir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.levelDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-inidir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify a the folder where your INI file is stored with the -inidir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.iniDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-logdir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify your log folder with the -logdir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.logDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-scriptsdir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify the folder where your Lua scripts are stored with the -scriptsdir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.luaDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-cachedir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify the folder where cache files are to be stored with the -cachedir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.cacheDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-robotdir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify the robots folder with the -robotdir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.robotDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-screenshotdir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify your screenshots folder with the -screenshotdir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.screenshotDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-sfxdir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify your sounds folder with the -sfxdir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.sfxDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }

      else if(!stricmp(argv[i].getString(), "-musicdir"))      // additional arg required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify your sounds folder with the -musicdir option");
            exitGame(1);
         }

         gCmdLineSettings.dirs.musicDir = argv[i+1].getString();
         argv.erase(i);
         argv.erase(i);
         i--;
      }
   }
}


// Read the command line params... if we're replaying a journal, we'll process those params as if they were actually there, while
// ignoring those params that were provided.
TNL_IMPLEMENT_JOURNAL_ENTRYPOINT(ZapJournal, readCmdLineParams, (Vector<StringPtr> argv), (argv))
{
   S32 argc = argv.size();

   // Process command line args  --> see http://bitfighter.org/wiki/index.php?title=Command_line_parameters
   for(S32 i = 0; i < argc; i+=2)
   {
      // Assume "args" starting with "-" are actually follow-on params
      bool hasAdditionalArg = (i != argc - 1 && argv[i + 1].getString()[0] != '-');     
      bool has2AdditionalArgs = hasAdditionalArg && (i != argc - 2);

      // Specify a master server
      if(!stricmp(argv[i], "-master"))        // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.masterAddress = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a master server address with -master option");
            exitGame(1);
         }
      }
      // Address to use when we're hosting
      else if(!stricmp(argv[i], "-hostaddr"))       // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.hostaddr = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a host address for the host to listen on (e.g. IP:Any:28000 or IP:192.169.1.100:5500)");
            exitGame(1);
         }
      }
      // Simulate packet loss 0 (none) - 1 (total)  [I think]
      else if(!stricmp(argv[i], "-loss"))          // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.loss = (F32)atof(argv[i+1]);
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a loss rate between 0 and 1 with the -loss option");
            exitGame(1);
         }
      }
      // Simulate network lag
      else if(!stricmp(argv[i], "-lag"))           // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.lag = atoi(argv[i+1]);
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a lag (in ms) with the -lag option");
            exitGame(1);
         }
      }

      else if(!stricmp(argv[i], "-forceUpdate"))
      {
         gCmdLineSettings.forceUpdate = true;
         i--;
      }

      // Run as a dedicated server
      else if(!stricmp(argv[i], "-dedicated"))     // additional arg optional
      {
         setParamsForDedicatedMode();

         if(hasAdditionalArg)
            gCmdLineSettings.dedicated = argv[i+1];
         else
            i--;     // Correct for the fact that we don't really have two args here...
      }
      // Specify user name
      else if(!stricmp(argv[i], "-name"))          // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.name = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must enter a nickname with the -name option");
            exitGame(1);
         }
      }
      // Specify user password
      else if(!stricmp(argv[i], "-password"))          // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.password = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must enter a password with the -password option");
            exitGame(1);
         }
      }

      // Specify password for accessing locked servers
      else if(!stricmp(argv[i], "-serverpassword"))      // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.serverPassword = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must enter a password with the -serverpassword option");
            exitGame(1);
         }
      }
      // Specify admin password for server
      else if(!stricmp(argv[i], "-adminpassword")) // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.adminPassword = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must specify an admin password with the -adminpassword option");
            exitGame(1);
         }
      }
      // Specify level change password for server
      else if(!stricmp(argv[i], "-levelchangepassword")) // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.levelChangePassword = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must specify an level-change password with the -levelchangepassword option");
            exitGame(1);
         }
      }

      // Specify list of levels...  all remaining params will be taken as level names
      else if(!stricmp(argv[i], "-levels"))     // additional arg(s) required
      {
         if(!hasAdditionalArg)
         {
            logprintf(LogConsumer::LogError, "You must specify one or more levels to load with the -levels option");
            exitGame(1);
         }

         // We'll overwrite our main level list directly, so if we're writing the INI for the first time,
         // we'll use the cmd line args to generate the INI Level keys, rather than the built-in defaults.
         for(S32 j = i+1; j < argc; j++)
            gCmdLineSettings.specifiedLevels.push_back(argv[j].getString());

         return;     // This param must be last, so no more args to process.  We can return.

      }
      // Specify name of the server as others will see it from the Join menu
      else if(!stricmp(argv[i], "-hostname"))   // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.hostname = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a server name with the -hostname option");
            exitGame(1);
         }
      }
      else if(!stricmp(argv[i], "-hostdescr"))   // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.hostdescr = argv[i+1];
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a description (use quotes) with the -hostdescr option");
            exitGame(1);
         }
      }
      // Change max players on server
      else if(!stricmp(argv[i], "-maxplayers")) // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.maxPlayers = atoi(argv[i+1]);
         else
         {
            logprintf(LogConsumer::LogError, "You must specify the max number of players on your server with the -maxplayers option");
            exitGame(1);
         }
      }
      // Start in window mode
      else if(!stricmp(argv[i], "-window"))     // no additional args
      {
         i--;  // compentsate for +=2 in for loop with single param
         gCmdLineSettings.displayMode = DISPLAY_MODE_WINDOWED;
      }
      // Start in fullscreen mode
      else if(!stricmp(argv[i], "-fullscreen")) // no additional args
      {
         i--;
         gCmdLineSettings.displayMode = DISPLAY_MODE_FULL_SCREEN_UNSTRETCHED;
      }
      // Start in fullscreen-stretch mode
      else if(!stricmp(argv[i], "-fullscreen-stretch")) // no additional args
      {
         i--;
         gCmdLineSettings.displayMode = DISPLAY_MODE_FULL_SCREEN_STRETCHED;
      }

      // Specify position of window
      else if(!stricmp(argv[i], "-winpos"))     // 2 additional args required
      {
         if(has2AdditionalArgs)
         {
            gCmdLineSettings.xpos = atoi(argv[i+1]);
            gCmdLineSettings.ypos = atoi(argv[i+2]);
            i++;  // compentsate for +=2 in for loop with single param (because we're grabbing two)
         }
         else
         {
            logprintf(LogConsumer::LogError, "You must specify the x and y position of the window with the -winpos option");
            exitGame(1);
         }
      }
      // Specify width of the game window
      else if(!stricmp(argv[i], "-winwidth")) // additional arg required
      {
         if(hasAdditionalArg)
            gCmdLineSettings.winWidth = atoi(argv[i+1]);
         else
         {
            logprintf(LogConsumer::LogError, "You must specify the width of the game window with the -winwidth option");
            exitGame(1);
         }
      }
      else if(!stricmp(argv[i], "-help"))       // no additional args
      {
         i--;
         logprintf("See http://bitfighter.org/wiki/index.php?title=Command_line_parameters for information");
         exitGame(0);
      }
      else if(!stricmp(argv[i], "-usestick")) // additional arg required
      {
         if(hasAdditionalArg)
            Joystick::UseJoystickNumber = atoi(argv[i+1]) - 1;  // zero-indexed     //  TODO: should be part of gCmdLineSettings
         else
         {
            logprintf(LogConsumer::LogError, "You must specify the joystick you want to use with the -usestick option");
            exitGame(1);
         }
      }
   }

   // Override some settings if we're compiling ZAP_DEDICATED
#ifdef ZAP_DEDICATED
   setParamsForDedicatedMode();
#endif
}


void InitSdlVideo()
{
   // Information about the current video settings.
   const SDL_VideoInfo* info = NULL;

   // Flags we will pass into SDL_SetVideoMode.
   S32 flags = 0;

   // Init!
   SDL_Init(0);

   // First, initialize SDL's video subsystem.
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
   {
      // Failed, exit.
      logprintf(LogConsumer::LogFatalError, "SDL Video initialization failed: %s", SDL_GetError());
      exitGame();
   }

   // Let's get some video information.
   info = SDL_GetVideoInfo();

   if(!info)
   {
      // This should probably never happen.
      logprintf(LogConsumer::LogFatalError, "SDL Video query failed: %s", SDL_GetError());
      exitGame();
   }

   // Find the desktop width/height and initialize the ScreenInfo object with it
   gScreenInfo.init(info->current_w, info->current_h);

   // Now, we want to setup our requested
   // window attributes for our OpenGL window.
   // We want *at least* 5 bits of red, green
   // and blue. We also want at least a 16-bit
   // depth buffer.
   //
   // The last thing we do is request a double
   // buffered window. '1' turns on double
   // buffering, '0' turns it off.
   //
   // Note that we do not use SDL_DOUBLEBUF in
   // the flags to SDL_SetVideoMode. That does
   // not affect the GL attribute state, only
   // the standard 2D blitting setup.

   SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
   SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
   SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
   SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
   SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

   SDL_WM_SetCaption(gWindowTitle, gWindowTitle);  // Icon name is same as window title -- set here so window will be created with proper name
   SDL_Surface* icon = SDL_LoadBMP("zap_win_icon.bmp");     // <=== TODO: put a real bmp here...
   SDL_WM_SetIcon(icon, NULL);

   // We want to request that SDL provide us with an OpenGL window, possibly in a fullscreen video mode.
   // Note the SDL_DOUBLEBUF flag is not required to enable double buffering when setting an OpenGL
   // video mode. Double buffering is enabled or disabled using the SDL_GL_DOUBLEBUFFER attribute.
   flags = SDL_OPENGL | SDL_HWSURFACE;

   // We don't need a size to initialize
   if(SDL_SetVideoMode(0, 0, 0, flags) == NULL)
   {
      logprintf(LogConsumer::LogWarning, "Unable to create hardware OpenGL window, falling back to software");

      flags = SDL_OPENGL;
      gScreenInfo.setHardwareSurface(false);

      if (SDL_SetVideoMode(0, 0, 0, flags) == NULL)
      {
         logprintf(LogConsumer::LogFatalError, "Unable to create OpenGL window: %s", SDL_GetError());
         exitGame();
      }
   }
   else
      gScreenInfo.setHardwareSurface(true);
}

// Now integrate INI settings with those from the command line and process them
void processStartupParams()
{
   // These options can only be set on cmd line
   if(!gCmdLineSettings.server.empty())
      gBindAddress.set(gCmdLineSettings.server);

   if(!gCmdLineSettings.dedicated.empty())
      gBindAddress.set(gCmdLineSettings.dedicated);

   gClientInfo.simulatedPacketLoss = gCmdLineSettings.loss;
   gClientInfo.simulatedLag = gCmdLineSettings.lag;

   // Enable some logging...
   gMainLog.setMsgType(LogConsumer::LogConnectionProtocol, gIniSettings.logConnectionProtocol);
   gMainLog.setMsgType(LogConsumer::LogNetConnection, gIniSettings.logNetConnection);
   gMainLog.setMsgType(LogConsumer::LogEventConnection, gIniSettings.logEventConnection);
   gMainLog.setMsgType(LogConsumer::LogGhostConnection, gIniSettings.logGhostConnection);

   gMainLog.setMsgType(LogConsumer::LogNetInterface, gIniSettings.logNetInterface);
   gMainLog.setMsgType(LogConsumer::LogPlatform, gIniSettings.logPlatform);
   gMainLog.setMsgType(LogConsumer::LogNetBase, gIniSettings.logNetBase);
   gMainLog.setMsgType(LogConsumer::LogUDP, gIniSettings.logUDP);

   gMainLog.setMsgType(LogConsumer::LogFatalError, gIniSettings.logFatalError); 
   gMainLog.setMsgType(LogConsumer::LogError, gIniSettings.logError); 
   gMainLog.setMsgType(LogConsumer::LogWarning, gIniSettings.logWarning); 
   gMainLog.setMsgType(LogConsumer::LogConnection, gIniSettings.logConnection); 
   gMainLog.setMsgType(LogConsumer::LogLevelLoaded, gIniSettings.logLevelLoaded); 
   gMainLog.setMsgType(LogConsumer::LogLuaObjectLifecycle, gIniSettings.logLuaObjectLifecycle); 
   gMainLog.setMsgType(LogConsumer::LuaLevelGenerator, gIniSettings.luaLevelGenerator); 
   gMainLog.setMsgType(LogConsumer::LuaBotMessage, gIniSettings.luaBotMessage); 
   gMainLog.setMsgType(LogConsumer::ServerFilter, gIniSettings.serverFilter); 


   // These options can come either from cmd line or INI file
   //if(gCmdLineSettings.name != "")
   //   gNameEntryUserInterface.setString(gCmdLineSettings.name);
   //else if(gIniSettings.name != "")
   //   gNameEntryUserInterface.setString(gIniSettings.name);
   //else
   //   gNameEntryUserInterface.setString(gIniSettings.lastName);


   if(gCmdLineSettings.name != "")
      gClientInfo.name = gCmdLineSettings.name;
   else if(gIniSettings.name != "")
      gClientInfo.name = gIniSettings.name;
   else
      gClientInfo.name = gIniSettings.lastName;

   gClientInfo.authenticated = false;

   if(gCmdLineSettings.password != "")
      gPlayerPassword = gCmdLineSettings.password;
   else if(gIniSettings.password != "")
      gPlayerPassword = gIniSettings.password;
   else
      gPlayerPassword = gIniSettings.lastPassword;


   
   if(gCmdLineSettings.serverPassword != "")
      gServerPassword = gCmdLineSettings.serverPassword;
   else if(gIniSettings.serverPassword != "")
      gServerPassword = gIniSettings.serverPassword;
   // else rely on gServerPassword default of ""

   if(gCmdLineSettings.adminPassword != "")
      gAdminPassword = gCmdLineSettings.adminPassword;
   else if(gIniSettings.adminPassword != "")
      gAdminPassword = gIniSettings.adminPassword;
   // else rely on gAdminPassword default of ""   i.e. no one can do admin tasks on the server

   if(gCmdLineSettings.levelChangePassword != "")
      gLevelChangePassword = gCmdLineSettings.levelChangePassword;
   else if(gIniSettings.levelChangePassword != "")
      gLevelChangePassword = gIniSettings.levelChangePassword;
   // else rely on gLevelChangePassword default of ""   i.e. no one can change levels on the server

   gConfigDirs.resolveLevelDir(); 


   if(gIniSettings.levelDir == "")                      // If there is nothing in the INI,
      gIniSettings.levelDir = gConfigDirs.levelDir;     // write a good default to the INI


   if(gCmdLineSettings.hostname != "")
      gHostName = gCmdLineSettings.hostname;
   else
      gHostName = gIniSettings.hostname;

   if(gCmdLineSettings.hostdescr != "")
      gHostDescr = gCmdLineSettings.hostdescr;
   else
      gHostDescr = gIniSettings.hostdescr;

   if(gCmdLineSettings.hostaddr != "")
      gBindAddress.set(gCmdLineSettings.hostaddr);
   else if(gIniSettings.hostaddr != "")
      gBindAddress.set(gIniSettings.hostaddr);
   // else stick with default defined earlier


   if(gCmdLineSettings.displayMode != DISPLAY_MODE_UNKNOWN)
      gIniSettings.displayMode = gCmdLineSettings.displayMode;    // Simply clobber the gINISettings copy

   if(gCmdLineSettings.xpos != -9999)
      gIniSettings.winXPos = gCmdLineSettings.xpos;
   if(gCmdLineSettings.ypos != -9999)
      gIniSettings.winYPos = gCmdLineSettings.ypos;
   if(gCmdLineSettings.winWidth > 0)
      gIniSettings.winSizeFact = max((F32) gCmdLineSettings.winWidth / (F32) gScreenInfo.getGameCanvasWidth(), MIN_SCALING_FACT);

   string strings;
   if(gCmdLineSettings.masterAddress != "")
      strings = gCmdLineSettings.masterAddress;
   else
      strings = gIniSettings.masterAddress;    // This will always have a value
   Vector<string> stringList;
   parseString(strings.c_str(), gMasterAddress, ',');


   if(gCmdLineSettings.name != "")                       // We'll clobber the INI file setting.  Since this
      gIniSettings.name = gCmdLineSettings.name;         // setting is never saved, we won't mess up our INI

   if(gCmdLineSettings.password != "")                   // We'll clobber the INI file setting.  Since this
      gIniSettings.password = gCmdLineSettings.password; // setting is never saved, we won't mess up our INI


   // Note that we can be in both clientMode and serverMode (such as when we're hosting a game interactively)

   if(gCmdLineSettings.clientMode)                // Create ClientGame object
   {
      gClientGame1 = new ClientGame(Address());   //   Let the system figure out IP address and assign a port
      gClientGame = gClientGame1;

       // Put any saved filename into the editor file entry thingy
      gClientGame->getUIManager()->getLevelNameEntryUserInterface()->setString(gIniSettings.lastEditorName);
   }
   //gClientGame2 = new ClientGame(Address());   //  !!! 2-player split-screen game in same game.


   // Not immediately starting a connection...  start out with name entry or main menu
   if(!gDedicatedServer)
   {
      if(gIniSettings.name == "")
      {
         if(gClientGame2)
         {
            gClientGame = gClientGame2;
            gClientGame1->mUserInterfaceData->get();
            gClientGame->getUIManager()->getNameEntryUserInterface()->activate();
            gClientGame2->mUserInterfaceData->get();
            gClientGame1->mUserInterfaceData->set();
            gClientGame = gClientGame1;
         }
         gClientGame->getUIManager()->getNameEntryUserInterface()->activate();
         seedRandomNumberGenerator(gIniSettings.lastName);
         gClientInfo.id.getRandom();                           // Generate a player ID
      }
      else
      {
         if(gClientGame2)
         {
            gClientGame = gClientGame2;
            gClientGame1->mUserInterfaceData->get();

            gClientGame2->mUserInterfaceData->get();
            gClientGame1->mUserInterfaceData->set();
            gClientGame = gClientGame1;
         }
         gClientGame->getUIManager()->getMainMenuUserInterface()->activate();

         gClientGame->setReadyToConnectToMaster(true);         // Set elsewhere if in dedicated server mode
         seedRandomNumberGenerator(gIniSettings.name);
         gClientInfo.id.getRandom();                           // Generate a player ID
      }
   }
}


// Call this function when running game in console mode; causes output to be dumped to console, if it was run from one
// Loosely based on http://www.codeproject.com/KB/dialog/ConsoleAdapter.aspx
bool writeToConsole()
{

#if defined(WIN32) && (_WIN32_WINNT >= 0x0500)
   // _WIN32_WINNT is needed in case of compiling for old windows 98 (this code won't work for windows 98)
   if(!AttachConsole(-1))
      return false;

   try
   {
      int m_nCRTOut= _open_osfhandle((intptr_t) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
      if(m_nCRTOut == -1)
         return false;

      FILE *m_fpCRTOut = _fdopen( m_nCRTOut, "w" );

      if( !m_fpCRTOut )
         return false;

      *stdout = *m_fpCRTOut;

      //// If clear is not done, any cout statement before AllocConsole will 
      //// cause, the cout after AllocConsole to fail, so this is very important
      // But here, we're not using AllocConsole...
      //std::cout.clear();
   }
   catch ( ... )
   {
      return false;
   } 
#endif    
   return true;
}


extern FileType getResourceType(const char *);

// This method may remove args from theArgv
void processCmdLineParams(Vector<TNL::StringPtr> &theArgv)
{
   // Process some command line args that need to be handled early, like journaling options
   for(S32 i = 0; i < theArgv.size(); i++)
   {
      bool hasAdditionalArg = (i != theArgv.size() - 1 && theArgv[i + 1].getString()[0] != '-');     

      if(!stricmp(theArgv[i].getString(), "-rules"))            // Print current rules and exit; nothing more to do
      {
         writeToConsole();
         GameType::printRules();
         exitGame(0);
      }

      // -sendres/getres <server> <password> <file> <resource type>
      if(!stricmp(theArgv[i].getString(), "-sendres") || !stricmp(theArgv[i].getString(), "-getres"))  
      {
         writeToConsole();
         if(theArgv.size() <= i + 4)     // Too few arguments
         {
            printf("Usage: bitfighter %s <server address> <password> <file> <resource type>\n", theArgv[i].getString());
            exitGame(1);
         }

         bool sending = (!stricmp(theArgv[i].getString(), "-sendres"));

         Address address(theArgv[i+1].getString());
         if(!address.isValid())
         {
            printf("Invalid address: Use format IP:nnn.nnn.nnn.nnn:port\n");
            exitGame(1);
         }

         string password = md5.getSaltedHashFromString(theArgv[i+2].getString());
         const char *fileName = theArgv[i+3].getString();

         FileType fileType = getResourceType(theArgv[i+4].getString());
         if(fileType == INVALID_RESOURCE_TYPE)
         {
            printf("Invalid resource type: Please sepecify BOT, LEVEL, or LEVELGEN\n");
            exitGame(1);
         }

         dataConn = new DataConnection(sending ? SEND_FILE : REQUEST_FILE, password, fileName, fileType);

         NetInterface *netInterface = new NetInterface(Address());
         dataConn->connect(netInterface, address);

         bool started = false;

         while(!started || dataConn && dataConn->isEstablished())
         {
            if(dataConn && dataConn->isEstablished())
            {
               if(!dataConn->mDataSender.isDone())
                  dataConn->mDataSender.sendNextLine();

               started = true;
            }

            netInterface->checkIncomingPackets();
            netInterface->processConnections();
            Platform::sleep(1);              // Don't eat CPU power
            if((!started) && (!dataConn))
            {
               printf("Failed to connect");
               started = true;      // Get out of this loop
            }
         }

         delete netInterface;
         delete dataConn;

         exitGame(0);
      }

      else if(!stricmp(theArgv[i].getString(), "-jsave"))      // Write game to journal
      {
         if(hasAdditionalArg)
            gZapJournal.record(theArgv[i+1].getString());
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a file with the jsave option");
            exitGame(1);
         }

         theArgv.erase(i);
         theArgv.erase(i);
         i--;
      }

      else if(!stricmp(theArgv[i].getString(), "-jplay"))      // Replay game from journal
      {
         if(hasAdditionalArg)
            gZapJournal.load(theArgv[i+1].getString());
         else
         {
            logprintf(LogConsumer::LogError, "You must specify a file with the jload option");
            exitGame(1);
         }

         theArgv.erase(i);
         theArgv.erase(i);
         i--;
      }

   }  // End of first pass of processing command line args

   gZapJournal.readCmdLineParams(theArgv);   // Process normal command line params, read INI, and start up
}


void setupLogging()
{
   // Specify which events each logfile will listen for
   S32 events = LogConsumer::AllErrorTypes | LogConsumer::LogConnection | LogConsumer::LuaLevelGenerator | LogConsumer::LuaBotMessage;

   gMainLog.init(joindir(gConfigDirs.logDir, "bitfighter.log"), "w");
   //gMainLog.setMsgTypes(events);  ==> set from INI settings     
   gMainLog.logprintf("------ Bitfighter Log File ------");

   gStdoutLog.setMsgTypes(events);   // writes to stdout

   gServerLog.init(joindir(gConfigDirs.logDir, "bitfighter_server.log"), "a");
   gServerLog.setMsgTypes(LogConsumer::AllErrorTypes | LogConsumer::ServerFilter | LogConsumer::StatisticsFilter); 
   gStdoutLog.logprintf("Welcome to Bitfighter!");
}

// Actually put us in windowed or full screen mode.  Pass true the first time this is used, false subsequently.
// This has the unfortunate side-effect of triggering a mouse move event.  
void actualizeScreenMode(bool changingInterfaces)
{

   // TODO: reimplement window positioning - difficult with SDL since it doesn't have much access to the
   // window manager; however, it may be possible to do position upon start-up, but not save when exiting

   //   if(gIniSettings.oldDisplayMode == DISPLAY_MODE_WINDOWED && !first)
   //   {
   //      gIniSettings.winXPos = glutGet(GLUT_WINDOW_X);
   //      gIniSettings.winYPos = glutGet(GLUT_WINDOW_Y);
   //
   //      gINI.SetValueI("Settings", "WindowXPos", gIniSettings.winXPos, true);
   //      gINI.SetValueI("Settings", "WindowYPos", gIniSettings.winYPos, true);
   //   }

   if(changingInterfaces)
      gClientGame->getUIManager()->getPrevUI()->onPreDisplayModeChange();
   else
      UserInterface::current->onPreDisplayModeChange();

   DisplayMode displayMode = gIniSettings.displayMode;

   gScreenInfo.resetGameCanvasSize();     // Set GameCanvasSize vars back to their default values

   // When we're in the editor, let's take advantage of the entire screen unstretched
   if(UserInterface::current->getMenuID() == EditorUI && 
         (gIniSettings.displayMode == DISPLAY_MODE_FULL_SCREEN_STRETCHED || gIniSettings.displayMode == DISPLAY_MODE_FULL_SCREEN_UNSTRETCHED))
   {
      // Smaller values give bigger magnification; makes small things easier to see on full screen
      F32 magFactor = 0.85f;      

      // For screens smaller than normal, we need to readjust magFactor to make sure we get the full canvas height crammed onto
      // the screen; otherwise our dock will break.  Since this mode is only used in the editor, we don't really care about
      // screen width; tall skinny screens will work just fine.
      magFactor = max(magFactor, (F32)gScreenInfo.getGameCanvasHeight() / (F32)gScreenInfo.getPhysicalScreenHeight());

      gScreenInfo.setGameCanvasSize(S32(gScreenInfo.getPhysicalScreenWidth() * magFactor), S32(gScreenInfo.getPhysicalScreenHeight() * magFactor));

      displayMode = DISPLAY_MODE_FULL_SCREEN_STRETCHED; 
   }

   S32 sdlVideoFlags = 0;
   S32 sdlWindowWidth, sdlWindowHeight;
   F64 orthoLeft = 0, orthoRight = 0, orthoTop = 0, orthoBottom = 0;

   // Always use OpenGL
   sdlVideoFlags = gScreenInfo.isHardwareSurface() ? SDL_OPENGL | SDL_HWSURFACE : SDL_OPENGL;

   // Set up variables according to display mode
   switch (displayMode)
   {
   case DISPLAY_MODE_FULL_SCREEN_STRETCHED:
      sdlWindowWidth = gScreenInfo.getPhysicalScreenWidth();
      sdlWindowHeight = gScreenInfo.getPhysicalScreenHeight();
      sdlVideoFlags |= SDL_FULLSCREEN;

      orthoRight = gScreenInfo.getGameCanvasWidth();
      orthoBottom = gScreenInfo.getGameCanvasHeight();
      break;

   case DISPLAY_MODE_FULL_SCREEN_UNSTRETCHED:
      sdlWindowWidth = gScreenInfo.getPhysicalScreenWidth();
      sdlWindowHeight = gScreenInfo.getPhysicalScreenHeight();
      sdlVideoFlags |= SDL_FULLSCREEN;

      orthoLeft = -1 * (gScreenInfo.getHorizDrawMargin());
      orthoRight = gScreenInfo.getGameCanvasWidth() + gScreenInfo.getHorizDrawMargin();
      orthoBottom = gScreenInfo.getGameCanvasHeight() + gScreenInfo.getVertDrawMargin();
      orthoTop = -1 * (gScreenInfo.getVertDrawMargin());
      break;

   default:  //  DISPLAY_MODE_WINDOWED
      sdlWindowWidth = (S32) floor((F32)gScreenInfo.getGameCanvasWidth()  * gIniSettings.winSizeFact + 0.5f);
      sdlWindowHeight = (S32) floor((F32)gScreenInfo.getGameCanvasHeight() * gIniSettings.winSizeFact + 0.5f);
      sdlVideoFlags |= SDL_RESIZABLE;

      orthoRight = gScreenInfo.getGameCanvasWidth();
      orthoBottom = gScreenInfo.getGameCanvasHeight();
      break;
   }

   // Set the SDL screen size and change to it
   if(SDL_SetVideoMode(sdlWindowWidth, sdlWindowHeight, 0, sdlVideoFlags) == NULL)
         logprintf(LogConsumer::LogFatalError, "Setting display mode failed: %s", SDL_GetError());

   // Now save the new window dimensions in ScreenInfo
   gScreenInfo.setWindowSize(sdlWindowWidth, sdlWindowHeight);

   glClearColor( 0, 0, 0, 0 );

   glViewport(0, 0, sdlWindowWidth, sdlWindowHeight);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // The best understanding I can get for glOrtho is that these are the coordinates you want to appear at the four corners of the
   // physical screen. If you want a "black border" down one side of the screen, you need to make left negative, so that 0 would
   // appear some distance in from the left edge of the physical screen.  The same applies to the other coordinates as well.
   glOrtho(orthoLeft, orthoRight, orthoBottom, orthoTop, 0, 1);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Do the scissoring
   if (displayMode == DISPLAY_MODE_FULL_SCREEN_UNSTRETCHED)
   {
      glScissor(gScreenInfo.getHorizPhysicalMargin(),    // x
            gScreenInfo.getVertPhysicalMargin(),     // y
            gScreenInfo.getDrawAreaWidth(),          // width
            gScreenInfo.getDrawAreaHeight());        // height

      glEnable(GL_SCISSOR_TEST);    // Turn on clipping to keep the margins clear
   }
   else
      glDisable(GL_SCISSOR_TEST);


   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glLineWidth(gDefaultLineWidth);

   if(gIniSettings.useLineSmoothing)
   {
      glEnable(GL_LINE_SMOOTH);
      //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      glEnable(GL_BLEND);
   }


   UserInterface::current->onDisplayModeChange();     // Notify the UI that the screen has changed mode
}


void setJoystick(ControllerTypeType jsType)
{
   // Set joystick type if we found anything other than None or Unknown
   // Otherwise, it makes more sense to remember what the user had last specified

   if (jsType != NoController && jsType != UnknownController && jsType != GenericController)
      gIniSettings.joystickType = jsType;
   // else do nothing and leave the value we read from the INI file alone

   // Set primary input to joystick if any controllers were found, even a generic one
   if(jsType == NoController || jsType == UnknownController)
      gIniSettings.inputMode = InputModeKeyboard;
   else
      gIniSettings.inputMode = InputModeJoystick;
}


// Function to handle one-time update tasks
// Use this when upgrading, and changing something like the name of an INI parameter.  The old version is stored in
// gIniSettings.version, and the new version is in BUILD_VERSION.
void checkIfThisIsAnUpdate()
{
   if(gIniSettings.version == BUILD_VERSION)
      return;

   // Wipe out all comments; they will be replaced with any updates
   gINI.deleteHeaderComments();
   gINI.deleteAllSectionComments();

   // version specific changes
   // 015a
   if(gIniSettings.version < 1836)
      gIniSettings.useLineSmoothing = true;

   // after 015a
   if(gIniSettings.version < 1840 && gIniSettings.maxBots == 127)
      gIniSettings.maxBots = 10;
}


#ifdef USE_BFUP
#include <direct.h>
#include <stdlib.h>
#include "stringUtils.h"      // For itos

// This block is Windows only, so it can do all sorts of icky stuff...
void launchUpdater(string bitfighterExecutablePathAndFilename)
{
   string updaterPath = ExtractDirectory(bitfighterExecutablePathAndFilename) + "\\updater";
   string updaterFileName = updaterPath + "\\bfup.exe";

   S32 buildVersion = gCmdLineSettings.forceUpdate ? 0 : BUILD_VERSION;

	S64 result = (S64) ShellExecuteA( NULL, NULL, updaterFileName.c_str(), itos(buildVersion).c_str(), updaterPath.c_str(), SW_SHOW );

   string msg = "";

   switch(result)
   {
   case 0:
      msg = "The operating system is out of memory or resources.";
      break;
   case ERROR_FILE_NOT_FOUND:
      msg = "The specified file was not found (tried " + updaterFileName + ").";
      break;
   case ERROR_PATH_NOT_FOUND:
      msg = "The specified path was not found (tried " + updaterFileName + ").";
      break;
   case ERROR_BAD_FORMAT:
      msg = "The .exe file is invalid (non-Win32 .exe or error in .exe image --> tried " + updaterFileName + ").";
      break;
   case SE_ERR_ACCESSDENIED:
      msg = "The operating system denied access to the specified file (tried " + updaterFileName + ").";
      break;
   case SE_ERR_ASSOCINCOMPLETE:
      msg = "The file name association is incomplete or invalid (tried " + updaterFileName + ").";;
      break;
   case SE_ERR_DDEBUSY:
      msg = "The DDE transaction could not be completed because other DDE transactions were being processed.";
      break;
   case SE_ERR_DDEFAIL:
      msg = "The DDE transaction failed.";
      break;
   case SE_ERR_DDETIMEOUT:
      msg = "The DDE transaction could not be completed because the request timed out.";
      break;
   case SE_ERR_DLLNOTFOUND:
      msg = "The specified DLL was not found.";
      break;
   case SE_ERR_NOASSOC:
      msg = "There is no application associated with the given file name extension.";
      break;
   case SE_ERR_OOM:
      msg = "There was not enough memory to complete the operation.";
      break;
   case SE_ERR_SHARE:
      msg = "A sharing violation occurred.";
      break;
   }

   if(msg != "")
      logprintf(LogConsumer::LogError, "Could not launch updater, returned error: %s", msg.c_str());
}
#endif

};  // namespace Zap


using namespace Zap;

////////////////////////////////////////
////////////////////////////////////////
// main()
////////////////////////////////////////
////////////////////////////////////////

#ifdef TNL_OS_XBOX
int zapmain(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{

#ifdef TNL_OS_MAC_OSX
   // Move to the application bundle's path (RDW)
   moveToAppPath();
#endif

   // Put all cmd args into a Vector for easier processing
   Vector<StringPtr> argVector;
   for(S32 i = 1; i < argc; i++)
      argVector.push_back(argv[i]);

   readFolderLocationParams(argVector);      // Reads folder location params, and removes them from argVector
   gConfigDirs.resolveDirs();                // Resolve all folders except for levels folder, resolved later

   // Before we go any further, we should get our log files in order.  Now we know where they'll be, as the 
   // only way to specify a non-standard location is via the command line, which we've now read.
   setupLogging();

   processCmdLineParams(argVector);          // Reads remaining params in two passes -- pre-journaling, and post-journaling

   // Load the INI file
   gINI.SetPath(joindir(gConfigDirs.iniDir, "bitfighter.ini"));
   gIniSettings.init();                      // Init struct that holds INI settings


   gZapJournal.processNextJournalEntry();    // If we're replaying a journal, this will cause the cmd line params to be read from the saved journal

   loadSettingsFromINI(&gINI);               // Read INI

   processStartupParams();                   // And merge command line params and INI settings
   Ship::computeMaxFireDelay();              // Look over weapon info and get some ranges, which we'll need before we start sending data


   if(gCmdLineSettings.serverMode)           // Only set when 1) compiled as a dedicated server; or 2) -dedicated param is specified
   {
      Vector<string> levels = LevelListLoader::buildLevelList();
      initHostGame(gBindAddress, levels, false);     // Start hosting
   }

   SoundSystem::init();  // Even dedicated server needs sound these days

   checkIfThisIsAnUpdate();


#ifndef ZAP_DEDICATED
   if(gClientGame)     // That is, we're starting up in interactive mode, as opposed to running a dedicated server
   {
      FXManager::init(gClientGame);                         // Get ready for sparks!!  C'mon baby!!
      Joystick::populateJoystickStaticData();               // Build static data needed for joysticks
      Joystick::initJoystick();                             // Initialize joystick system
      resetKeyStates();                                     // Reset keyboard state mapping to show no keys depressed
      ControllerTypeType controllerType = Joystick::autodetectJoystickType();
      setJoystick(controllerType);               // Will override INI settings, so process INI first

      // On OS X, make sure we're in the right directory
#ifdef TNL_OS_MAC_OSX
      moveToAppPath();
#endif

      InitSdlVideo();         // Get our main SDL rendering window all set up
      SDL_EnableUNICODE(1);   // Activate unicode ==> http://sdl.beuc.net/sdl.wiki/SDL_EnableUNICODE
      SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

      // Put 0,0 at the center of the screen
      //glTranslatef(gScreenInfo.getGameCanvasWidth() / 2, gScreenInfo.getGameCanvasHeight() / 2, 0);     

      atexit(onExit);
      actualizeScreenMode(false);            // Create a display window

      gConsole = OGLCONSOLE_Create();        // Create our console *after* the screen mode has been actualized

#ifdef USE_BFUP
      if(gIniSettings.useUpdater)
         launchUpdater(argv[0]);             // Spawn external updater tool to check for new version of Bitfighter -- Windows only
#endif

   }
#endif
   dedicatedServerLoop();  //    loop forever, running the idle command endlessly

   return 0;
}

