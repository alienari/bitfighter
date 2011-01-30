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

#include "gameStats.h"
#include "gameWeapons.h"         // For WeaponType enum

#include "tnlMethodDispatch.h"
#include "tnlAssert.h"


// This is shared in both client and master.
// this read/write is usually the hardest part about struct, but this allows custom version handling.

using namespace TNL;
using namespace Zap;

namespace Zap
{
void logGameStats(VersionedGameStats *stats, S32 format)  // TODO: log game stats
	{
	GameStats *g = &stats->gameStats;
      for(S32 i = 0; i < g->teamStats.size(); i++)
      {
         TeamStats *gt = &g->teamStats[i];
         for(U32 j = 0; j < gt->playerStats.size(); j++)
         {
            PlayerStats *gp = &gt->playerStats[j];
            for(U32 k = 0; k < gp->weaponStats.size(); k++)
            {
               WeaponStats *gw = &gp->weaponStats[k];
            }
			}
		}
	}
}


#ifdef TNL_ENABLE_ASSERTS
bool VersionedGameStats_testing = false;
U32 VersionedGameStats_ReadSize;
U32 VersionedGameStats_WriteSize;
#define VersionedGameStats_write_start(s) {VersionedGameStats_WriteSize = s.getBitPosition();}
#define VersionedGameStats_write_end(s) {VersionedGameStats_WriteSize = s.getBitPosition() - VersionedGameStats_WriteSize;}
#define VersionedGameStats_read_start(s) {VersionedGameStats_ReadSize = s.getBitPosition();}
#define VersionedGameStats_read_end(s) {VersionedGameStats_ReadSize = s.getBitPosition() - VersionedGameStats_ReadSize; \
	TNLAssert((!VersionedGameStats_testing) || VersionedGameStats_WriteSize == VersionedGameStats_ReadSize, "VersionedGameStats Read and write size is not equal. They must be equal size to prevent network errors.")}
#else
#define VersionedGameStats_write_start(s)
#define VersionedGameStats_write_end(s)
#define VersionedGameStats_read_start(s)
#define VersionedGameStats_read_end(s)
#endif



namespace Types
{
   const U8 VersionedGameStats_CurrentVersion = 0;

   U8 readU8(TNL::BitStream &s)   { U8 val; read(s, &val); return val; }
   S8 readS8(TNL::BitStream &s)   { S8 val; read(s, &val); return val; }
   U16 readU16(TNL::BitStream &s) { U16 val; read(s, &val); return val; }
   S16 readS16(TNL::BitStream &s) { S16 val; read(s, &val); return val; }
   U32 readU32(TNL::BitStream &s) { U32 val; read(s, &val); return val; }
   S32 readS32(TNL::BitStream &s) { S32 val; read(s, &val); return val; }
   // for bool, use   s.readFlag();
   string readString(TNL::BitStream &s) { char val[256]; s.readString(val); return val; }
   void writeString(TNL::BitStream &s, const string &val) { s.writeString(val.c_str()); }

   /// Reads objects from a BitStream.
   void read(TNL::BitStream &s, VersionedGameStats *val)
   {
		VersionedGameStats_read_start(s);
      val->valid = false;
      U8 version = readU8(s);  // Read version number.
      val->version = version;
      GameStats *g = &val->gameStats;
      S32 playerCount = 0;

      g->isOfficial = s.readFlag();
      g->playerCount = readU16(s);  // players + robots
      g->duration = readU16(s);     // game length in seconds
      g->isTeamGame = s.readFlag();
      //g->isTied = s.readFlag();
      g->gameType = readString(s);
      g->levelName = readString(s);
   
      S32 teamCount = readU8(s);
		g->teamCount = teamCount; // is this needed?
      if(!s.isValid()) return;
      g->teamStats.setSize(teamCount);
      for(S32 i = 0; i < teamCount; i++)
      {
         TeamStats *gt = &g->teamStats[i];
         gt->name = readString(s);
         gt->score = readS32(s);
         gt->color_bin = s.readInt(24); // 24 bit color
			char c[24];
			dSprintf(c, sizeof(c), "%.2X%.2X%.2X", U32((gt->color_bin >> 16) & 0xFF), U32((gt->color_bin >> 8) & 0xFF), U32(gt->color_bin & 0xFF));
         gt->color = string(c);
         //gt->gameResult = "?";
         if(!s.isValid()) return;

         U32 playerSize = readU8(s);
         gt->playerStats.setSize(playerSize);
         for(U32 j = 0; j < playerSize; j++)
         {
            PlayerStats *gp = &gt->playerStats[j];
            gp->name = readString(s);
            gp->points = readS32(s);
            gp->kills = readU16(s);
            gp->deaths = readU16(s);
            gp->suicides = readU16(s);
            gp->switchedTeamCount = readU8(s);
            gp->switchedTeams = (gp->switchedTeamCount != 0);
            gp->isRobot = s.readFlag();
            gp->isAdmin = s.readFlag();
            gp->isLevelChanger = s.readFlag();
            gp->isAuthenticated = false; //s.readFlag();  // we may set this by comparing Nonce id.
            gp->nonce.read(&s);
            //gp->gameResult = "?";

            U32 weaponSize = readU8(s);
            gp->weaponStats.setSize(weaponSize);
            for(U32 k = 0; k < weaponSize; k++)
            {
               WeaponStats *gw = &gp->weaponStats[k];
               gw->weaponType = WeaponType(readU8(s));
               gw->shots = readU16(s);
               gw->hits = readU16(s);
            }
            playerCount++;
         }
      }
      if(playerCount == g->playerCount)  // make sure server don't lie to master.
         val->valid = true;

         // Stops here if TNL_ENABLE_ASSERTS is on, and write/read size is not matched.
      VersionedGameStats_read_end(s);
   }


   /// Writes objects into a BitStream. Server write and send to master.
   void write(TNL::BitStream &s, VersionedGameStats &val)
   {
		VersionedGameStats_write_start(s);
      write(s, U8(VersionedGameStats_CurrentVersion));       // send current version
      GameStats *g = &val.gameStats;

      // write(number) write 32 bit when number is S32
      // write(number) write 8 bit when number is U8
      // write(U16(number)) always write 16 bit when number is U8, S32, or any type.

      s.writeFlag(g->isOfficial);
      write(s, U16(g->playerCount));
      write(s, U16(g->duration));     // game length in seconds
      s.writeFlag(g->isTeamGame);
      //s.writeFlag(g->isTied);
      writeString(s, g->gameType);
      writeString(s, g->levelName);

      write(s, U8(g->teamStats.size())); // g->teamCount might not be needed
      for(S32 i = 0; i < g->teamStats.size(); i++)
      {
         TeamStats *gt = &g->teamStats[i];
         writeString(s, gt->name);
         write(s, S32(gt->score));
         s.writeInt(gt->color_bin,24); // 24 bit color

         write(s, U8(gt->playerStats.size()));

         for(S32 j = 0; j < gt->playerStats.size(); j++)
         {
            PlayerStats *gp = &gt->playerStats[j];
            writeString(s, gp->name);
            write(s, S32(gp->points));
            write(s, U16(gp->kills));
            write(s, U16(gp->deaths));
            write(s, U16(gp->suicides));
            write(s, U8(gp->switchedTeamCount));
            //gp->switchedTeams
            s.writeFlag(gp->isRobot);
            s.writeFlag(gp->isAdmin);
            s.writeFlag(gp->isLevelChanger);
            //gp->isAuthenticated;;  // we may set this by comparing Nonce id.
            gp->nonce.write(&s);

            write(s, U8(gp->weaponStats.size()));
            for(S32 k = 0; k < gp->weaponStats.size(); k++)
            {
               WeaponStats *gw = &gp->weaponStats[k];
               write(s, U8(gw->weaponType));
               write(s, U16(gw->shots));
               write(s, U16(gw->hits));
            }
         }
      }
		VersionedGameStats_write_end(s);
   }
}
