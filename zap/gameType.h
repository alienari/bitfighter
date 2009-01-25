//-----------------------------------------------------------------------------------
//
// bitFighter - A multiplayer vector graphics space game
// Based on Zap demo released for Torque Network Library by GarageGames.com
//
// Derivative work copyright (C) 2008 Chris Eykamp
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

#ifndef _GAMETYPE_H_
#define _GAMETYPE_H_

#include "timer.h"
#include "sfx.h"
#include "voiceCodec.h"
#include "gameObject.h" 
#include <string>

namespace Zap
{

class FlagItem;
class GoalZone;
struct MenuItem;
class Item;


class ClientRef : public Object
{
public:
   StringTableEntry name;  // Name of client - guaranteed to be unique of current clients

   S32 teamId;
   
   S32 score;              // Individual score for current game
   S32 cumScore;           // Cumulative score over all games
   S32 totalScore;         // Total points scored by all players, including this one

   bool isAdmin;
   bool isLevelChanger;
   Timer respawnTimer;

   bool wantsScoreboardUpdates;  // Indicates whether the client has requested scoreboard streaming (e.g. pressing Tab key)
   bool readyForRegularGhosts;

   SafePtr<GameConnection> clientConnection;
   RefPtr<SFXObject> voiceSFX;
   RefPtr<VoiceDecoder> decoder;

   U32 ping;

   // Constructor
   ClientRef()
   {
      ping = 0;
      score = 0;
      cumScore = 0;
      totalScore = 0;
      readyForRegularGhosts = false;
      wantsScoreboardUpdates = false;
      teamId = 0;
      isAdmin = false;
   }
};

//////////

class GameType : public GameObject
{
private:
   virtual U32 getLowerRightCornerScoreboardOffsetFromBottom() { return 60; }      // Game-specific location for the bottom of the scoreboard on the lower-right corner
                                                                                   // (because games like hunters have more stuff down there we need to look out for)
   //bool mUsingFlagSpawnPoints;   // Controls how flags return to starting locations.  False = traditional method, each flag in its place, True = flags spawn at random point

public:
   static const S32 gMaxTeams = 9;                                   // Max teams allowed
   static const S32 gFirstTeamNumber = -2;                           // First team is "Hostile to All" with index -2
   static const U32 gMaxTeamCount = gMaxTeams - gFirstTeamNumber;    // Number of possible teams, including Neutral and Hostile to All
   static const char *validateGameType(const char *gtype);           // Returns a valid gameType, defaulting to gDefaultGameTypeIndex if needed

   virtual const char *getGameTypeString() { return "Zapmatch"; }                            // Will be overridden by other games
   virtual const char *getInstructionString() { return "Blast as many ships as you can!"; }    //    -- ditto --
   virtual bool isTeamGame() { return mTeams.size() > 1; }                                // Team game if we have teams.  Otherwise it's every man for himself.
   virtual bool isSpawnWithLoadoutGame() { return false; }                                // We do not spawn with our loadout, but instead need to pass through a loadout zone

   static void printRules();     // Dump game-rule info

   enum
   {
      RespawnDelay = 1500,
      SwitchTeamsDelay = 60000,   // Time between team switches (ms) -->  60000 = 1 minute
   };

   struct BarrierRec
   {
      Vector<F32> verts;
      F32 width;
      bool solid;
   };

   Vector<BarrierRec> mBarriers;
   Vector<RefPtr<ClientRef> > mClientList;
   ClientRef *mLocalClient;

   virtual ClientRef *allocClientRef() { return new ClientRef; }

   struct Team
   {
      StringTableEntry name;
      Color color;
      Vector<Point> spawnPoints;
      Vector<Point> flagSpawnPoints;   // List of places for team flags to spawn

      U32 numPlayers;
      S32 score;
      Team() { numPlayers = 0; score = 0; }
   };
   Vector<Team> mTeams;
   Vector<Point> mFlagSpawnPoints;     // List of non-team specific spawn points for flags

   StringTableEntry mLevelName;
   StringTableEntry mLevelDescription;
   StringTableEntry mLevelCredits;
   S32 minRecPlayers;         // Recommended min players for this level
   S32 maxRecPlayers;         // Recommended max players for this level

   struct ItemOfInterest
   {
      SafePtr<Item> theItem;
      U32 teamVisMask;
   };

   Vector<ItemOfInterest> mItemsOfInterest;

   void addItemOfInterest(Item *theItem);

   Timer mScoreboardUpdateTimer;
   Timer mGameTimer;             // Track when current game will end
   Timer mGameTimeUpdateTimer;
   Timer mLevelInfoDisplayTimer;
   Timer mInputModeChangeAlertDisplayTimer;
   bool mCanSwitchTeams;         // Player can switch teams when this is true, not when it is false

   S32 mTeamScoreLimit;
   bool mBetweenLevels;          // We'll need to prohibit certain things (like team changes) when game is in an "intermediate" state
   bool mGameOver;               // Set to true when an end condition is met

   enum {
      MaxPing = 999,
      DefaultGameTime = 10 * 60 * 1000,
      DefaultTeamScoreLimit = 8,
      LevelInfoDisplayTime = 6000,
   };

   // Some games have extra game parameters.  We need to create a structure to communicate those parameters to the editor so
   // it can make an intelligent decision about how to handle them.  Note that, for now, all such parameters are assumed to be S32.
   struct ParameterDescription
   {
      const char *name;
      const char *units;
      const char *help;
      S32 value;     // Default value for this parameter
      S32 minval;    // Min value for this param
      S32 maxval;    // Max value for this param
   };

   // Potentially scoring events
   enum ScoringEvent {
      KillEnemy,              // all games
      KillSelf,               // all games
      KillTeammate,           // all games

      CaptureFlag,
      CaptureZone,            // zone control -> gain zone
      UncaptureZone,          // zone control -> lose zone
      HoldFlagInZone,         // htf
      RabbitHoldsFlag,        // rabbit, called every second
      RabbitKilled,           // rabbit
      RabbitKills,            // rabbit
      ReturnFlagsToNexus,     // hunters game
      ReturnFlagToZone,       // retrieve -> flag returned to zone
      LostFlag,               // retrieve -> enemy took flag
      ReturnTeamFlag,  	      // ctf -> holds enemy flag, touches own flag
      ScoreGoalEnemyTeam,     // soccer
      ScoreGoalHostileTeam,   // soccer
      ScoreGoalOwnTeam,       // soccer -> score on self
   };

   enum ScoringGroup {
      IndividualScore,
      TeamScore,
   };

   virtual Vector<U32> getScoringEventList();
   virtual S32 getEventScore(ScoringGroup scoreGroup, ScoringEvent scoreEvent, S32 data);
   static string GameType::getScoringEventDescr(ScoringEvent event);
   
   static Vector<RangedU32<0, MaxPing> > mPingTimes; // Static vector used for constructing update RPCs
   static Vector<SignedInt<24> > mScores;

   GameType();    // Constructor
   void countTeamPlayers();

   Color getClientColor(const StringTableEntry &clientName)
   {
      ClientRef *cl = findClientRef(clientName);
      if(cl)
         return mTeams[cl->teamId].color;
      return Color();
   }

   ClientRef *findClientRef(const StringTableEntry &name);

   void processArguments(S32 argc, const char **argv);
   virtual Vector<GameType::ParameterDescription> describeArguments();

   virtual bool processLevelItem(S32 argc, const char **argv);
   void onAddedToGame(Game *theGame);

   void idle(GameObject::IdleCallPath path);

   void gameOverManGameOver();
   void checkForWinningScore(S32 score);     // Check if player or team has reachede the winning score
   virtual void onGameOver();

   virtual void serverAddClient(GameConnection *theClient);
   virtual void serverRemoveClient(GameConnection *theClient);

   virtual bool objectCanDamageObject(GameObject *damager, GameObject *victim);
   virtual void controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject);

   virtual void spawnShip(GameConnection *theClient);
   virtual void changeClientTeam(GameConnection *theClient, S32 team);     // Change player to team indicated, -1 = cycle teams

   virtual void renderInterfaceOverlay(bool scoreboardVisible);
   void renderObjectiveArrow(GameObject *target, Color c, F32 alphaMod = 1.0f);
   void renderObjectiveArrow(Point p, Color c, F32 alphaMod = 1.0f);

   void renderTimeLeft();
   void renderTalkingClients();

   void addTime(U32 time);    // Extend the game by time (in ms)

   virtual void clientRequestLoadout(GameConnection *client, const Vector<U32> &loadout);
   virtual void updateShipLoadout(GameObject *shipObject); // called from LoadoutZone when a Ship touches the zone
   void setClientShipLoadout(ClientRef *cl, const Vector<U32> &loadout);

   virtual Color getShipColor(Ship *s);         // Get the color of a ship

   virtual Color getTeamColor(S32 team);        // Get the color of a team, based on index
   Color getTeamColor(GameObject *theObject);   // Get the color of a team, based on object

   S32 getTeam(const char *playerName);         // Given a player, return their team

   // game type flag methods for CTF, Rabbit, Football
   virtual void addFlag(FlagItem *theFlag) {}
   virtual void flagDropped(Ship *theShip, FlagItem *theFlag) {}
   virtual void shipTouchFlag(Ship *theShip, FlagItem *theFlag) {}

   virtual void addZone(GoalZone *theZone) {}
   virtual void shipTouchZone(Ship *theShip, GoalZone *theZone) {}

   void queryItemsOfInterest();
   void performScopeQuery(GhostConnection *connection);
   virtual void performProxyScopeQuery(GameObject *scopeObject, GameConnection *connection);

   void onGhostAvailable(GhostConnection *theConnection);
   TNL_DECLARE_RPC(s2cSetLevelInfo, (StringTableEntry levelName, StringTableEntry levelDesc, S32 teamScoreLimit, StringTableEntry levelCreds));
   TNL_DECLARE_RPC(s2cAddBarriers, (Vector<F32> barrier, F32 width, bool solid));
   TNL_DECLARE_RPC(s2cAddTeam, (StringTableEntry teamName, F32 r, F32 g, F32 b));
   TNL_DECLARE_RPC(s2cAddClient, (StringTableEntry clientName, bool isMyClient, bool isAdmin));
   TNL_DECLARE_RPC(s2cClientJoinedTeam, (StringTableEntry clientName, RangedU32<0, gMaxTeams> teamIndex));
   TNL_DECLARE_RPC(s2cClientBecameAdmin, (StringTableEntry clientName));
   TNL_DECLARE_RPC(s2cClientBecameLevelChanger, (StringTableEntry clientName));

   TNL_DECLARE_RPC(s2cSyncMessagesComplete, (U32 sequence));
   TNL_DECLARE_RPC(c2sSyncMessagesComplete, (U32 sequence));

   TNL_DECLARE_RPC(s2cSetGameOver, (bool gameOver));
   TNL_DECLARE_RPC(s2cSetTimeRemaining, (U32 timeLeft));

   TNL_DECLARE_RPC(s2cCanSwitchTeams, (bool allowed));

   TNL_DECLARE_RPC(s2cRemoveClient, (StringTableEntry clientName));

   void updateScore(ClientRef *client, S32 team, ScoringEvent event, S32 data = 0);
   void updateScore(ClientRef *client, ScoringEvent event, S32 data = 0);
   void updateScore(S32 team, ScoringEvent event, S32 data = 0);

   TNL_DECLARE_RPC(s2cSetTeamScore, (RangedU32<0, gMaxTeams> teamIndex, U32 score));

   TNL_DECLARE_RPC(c2sRequestScoreboardUpdates, (bool updates));
   TNL_DECLARE_RPC(s2cScoreboardUpdate, (Vector<RangedU32<0, MaxPing> > pingTimes, Vector<SignedInt<24> > scores));
   virtual void updateClientScoreboard(ClientRef *theClient);

   TNL_DECLARE_RPC(c2sAdvanceWeapon, ());
   TNL_DECLARE_RPC(c2sSelectWeapon, (RangedU32<0, ShipWeaponCount> index));

   // Handle additional game-specific menu options for the client and the admin
   virtual void addClientGameMenuOptions(Vector<MenuItem> &menuOptions);
   virtual void processClientGameMenuOption(U32 index);                        // Param used only to hold team, at the moment

   virtual void addAdminGameMenuOptions(Vector<MenuItem> &menuOptions);
   
   TNL_DECLARE_RPC(c2sAddTime, (U32 time));                                    // Admin is adding time to the game
   TNL_DECLARE_RPC(c2sChangeTeams, (S32 team));                                // Player wants to change teams

   TNL_DECLARE_RPC(c2sSendChat, (bool global, StringPtr message));             // In-game chat
   TNL_DECLARE_RPC(c2sSendChatSTE, (bool global, StringTableEntry ste));       // Quick-chat

   void sendChatDisplayEvent(ClientRef *cl, bool global, NetEvent *theEvent);  // In-game chat

   TNL_DECLARE_RPC(s2cDisplayChatMessage, (bool global, StringTableEntry clientName, StringPtr message));
   TNL_DECLARE_RPC(s2cDisplayChatMessageSTE, (bool global, StringTableEntry clientName, StringTableEntry message));

   TNL_DECLARE_RPC(s2cKillMessage, (StringTableEntry victim, StringTableEntry killer));

   TNL_DECLARE_RPC(c2sVoiceChat, (bool echo, ByteBufferPtr compressedVoice));
   TNL_DECLARE_RPC(s2cVoiceChat, (StringTableEntry client, ByteBufferPtr compressedVoice));

   TNL_DECLARE_CLASS(GameType);
};

#define GAMETYPE_RPC_S2C(className, methodName, args, argNames) \
   TNL_IMPLEMENT_NETOBJECT_RPC(className, methodName, args, argNames, NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)

#define GAMETYPE_RPC_C2S(className, methodName, args, argNames) \
   TNL_IMPLEMENT_NETOBJECT_RPC(className, methodName, args, argNames, NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhostParent, 0)

};

#endif

