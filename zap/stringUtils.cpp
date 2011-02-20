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

#include "stringUtils.h"
#include "tnlPlatform.h"   // For Vector, types, and dSprintf
#include "tnlVector.h"
#include "tnlLog.h"

#include <stdio.h>
#include <stdarg.h>        // For va_args
#include <string>          // For... everything.  This is stringUtils, after all!
#include <sstream>         // For parseString
#include <sys/stat.h>      // For testing existence of folders
#ifdef TNL_OS_WIN32
#include <direct.h>        // For mkdir
#endif

using namespace std;
using namespace TNL;

namespace Zap
{

// Collection of useful string things

string ExtractDirectory( const string& path )
{
  return path.substr( 0, path.find_last_of( '\\' ) + 1 );
}

string ExtractFilename( const string& path )
{
  return path.substr( path.find_last_of( '\\' ) + 1 );
}


string itos(S32 i)
{
   char outString[12];  // 11 chars plus a null char, -2147483648
   dSprintf(outString, sizeof(outString), "%d", i);
   return outString;
}


string itos(U32 i)
{
   char outString[11];  // 10 chars plus a null char, 4294967295
   dSprintf(outString, sizeof(outString), "%u", i);
   return outString;
}


string itos(U64 i)
{
   char outString[21];  // 20 chars plus a null char, 18446744073709551615
   dSprintf(outString, sizeof(outString), "%u", i);
   return outString;
}


string itos(S64 i)
{
   char outString[21];  // 20 chars plus a null char, ?9223372036854775808
   dSprintf(outString, sizeof(outString), "%d", i);
   return outString;
}


string ftos(F32 f, S32 digits)
{
   char outString[100];
   dSprintf(outString, sizeof(outString), (string("%2.") + itos(digits) + "f").c_str(), f);
   return outString;
}


// From http://stackoverflow.com/questions/1087088/single-quote-issues-with-c-find-and-replace-function
string replaceString( const string &strString, const string &strOld, const string &strNew )
{
   string str = strString;    // Make working copy

    for( string::size_type nReplace = str.rfind( strOld ); nReplace != string::npos; nReplace = str.rfind( strOld, nReplace - 1 ) )
    {
        str.replace( nReplace, strOld.length(), strNew );
        if( nReplace == 0 )
            break;
    }
    return str;
}


//// From http://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
//bool caseInsensitiveStringCompare(const string &str1, const string &str2) {
//    if (str1.size() != str2.size()) {
//        return false;
//    }
//    for (string::const_iterator c1 = str1.begin(), c2 = str2.begin(); c1 != str1.end(); ++c1, ++c2) {
//        if (tolower(*c1) != tolower(*c2)) {
//            return false;
//        }
//    }
//    return true;
//}


// Convert string to lower case
string lcase(string strToConvert)
{
   for(U32 i = 0; i < strToConvert.length(); i++)
      strToConvert[i] = tolower(strToConvert[i]);
   return strToConvert;
}


// Convert string to upper case
string ucase(string strToConvert)
{
   for(U32 i = 0; i < strToConvert.length(); i++)
      strToConvert[i] = toupper(strToConvert[i]);
   return strToConvert;
}


// Concatenate all strings in words into one, startingWith default to zero
string concatenate(const Vector<string> &words, S32 startingWith)
{
   string concatenated = "";
   for(S32 i = startingWith; i < words.size(); i++)
      concatenated += (i == startingWith ? "" : " ") + words[i];

   return concatenated;
}


// TODO: Merge this with the one following
// Based on http://www.gamedev.net/community/forums/topic.asp?topic_id=320087
// Parses a string on whitespace, except when inside "s
Vector<string> parseString(const string &line)
{
  Vector<string> result;

  string          item;
  stringstream    ss(line);

  while(ss >> item){
    if (item[0]=='"')
    {
      S32 lastItemPosition = item.length() - 1;
      if (item[lastItemPosition]!='"') {
        // read the rest of the double-quoted item
        string restOfItem;
        getline(ss, restOfItem, '"');
        item += restOfItem;
      }
      // otherwise, we had a single word that was quoted. In any case, we now
      // have the item in quotes; remove them
      item = item.substr(1, lastItemPosition-1);
    }
    // item is "fully cooked" now
    result.push_back(item);

  }
  return result;
}


string listToString(Vector<string> &words, char seperator)
{
	string str = "";

   // Convert separator char to a c_string so it can be added below
   char sep[2];
   sep[0] = seperator;
   sep[1] = 0;    // Null terminate
      
   for(S32 i = 0; i < words.size(); i++)
        str += words[i] + ((i < words.size() - 1) ? sep : "");

   return str;
}


// Splits inputString into a series of words using the specified separator
void parseString(const char *inputString, Vector<string> &words, char seperator)
{
	char word[128];
	S32 wn = 0;       // Where we are in the word we're creating
	S32 isn = 0;      // Where we are in the inputString we're parsing

	words.clear();

	while(inputString[isn] != 0)
   {
		if(inputString[isn] == seperator) 
      {
			word[wn] = 0;    // Add terminating NULL
			if(wn > 0) 
            words.push_back(word);
			wn = 0;
		}
      else
      {
			if(wn < 126)   // Avoid overflows
         {
            word[wn] = inputString[isn]; 
            wn++; 
         }
		}
		isn++;
	}
    word[wn] = 0;
    if(wn > 0) 
       words.push_back(word);
}


// Safe fprintf ==> throws exception if writing fails
void s_fprintf(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    if(fprintf(stream, "%s", buffer) < 0)     // Uh-oh...
    {
       throw(SaveException("Error writing to file"));
    }
}


// Returns true if file or folder exists
// Ok, not strictly a string util, but do we really want a fileutils just for this??
bool fileExists(const string &path)
{
   struct stat st;
   return (stat(path.c_str(), &st) == 0);               // Does path exist?
}


bool makeSureFolderExists(const string &dir)
{
   if(!fileExists(dir))
   {
#ifdef TNL_OS_WIN32
      mkdir(dir.c_str());
#else
      mkdir(dir.c_str(), 0755);
#endif
      if(!fileExists(dir))
      {
         logprintf(LogConsumer::LogError, "Could not create folder %s", dir.c_str());
         return false;
      }
   }

   return true;
}
   

// Join without checking for blank parts
string strictjoindir(const string &part1, const string &part2)
{
   if(part1.length() == 0) return part2;      //avoid crash on zero length string.

   // Does path already have a trailing delimeter?  If so, we'll use that.
   if(part1[part1.length() - 1] == '\\' || part1[part1.length() - 1] == '/')
      return part1 + part2;

   // Otherwise, join with a delimeter.  This works on Win, OS X, and Linux.
   return part1 + "/" + part2;
}


// Three arg version
string strictjoindir(const string &part1, const string &part2, const string &part3)
{
   return strictjoindir(part1, strictjoindir(part2, part3));
}


string trim_right(const string &source, const string &t)
{
   string str = source;
   return str.erase(str.find_last_not_of(t) + 1);
}


string trim_left(const string &source, const string &t)
{
   string str = source;
   return str.erase(0, source.find_first_not_of(t));
}

string trim(const string &source, const string &t)
{
   return trim_left(trim_right(source, t), t);
}


};

