/*
Copyright (c) 2009, William M Brandt (aka 'Taekvideo')
All rights reserved.
Email: taekvideo@gmail.com (feel free to contact me with any questions, concerns, or suggestions)

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistribution of any files containing source code covered by this license must retain the above copyright notice, 
this list of conditions, and the following disclaimer.
* Neither the name William M Brandt nor the pseudonym 'Taekvideo' may be used to endorse or 
promote products derived from this software without specific prior written permission.
* It's not required, but I would appreciate being included in your credits.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "authenticator.h"
#include "phpbbhash.h"
#include <string.h>
#include <ctype.h>
#include "mysql++/lib/mysql++.h"

using namespace mysqlpp;
using namespace std;

Authenticator::Authenticator(){
	connection = NULL;
	securityLevel = 1;
}

Authenticator::Authenticator(string server, string username, string password, string database, string tablePrefix, int securityLevel){
	connection = NULL;
	initialize(server, username, password, database, tablePrefix, securityLevel);
}

Authenticator::~Authenticator(){
	if (connection) delete connection;
}

void Authenticator::initialize(string server, string username, string password, string database, string tablePrefix, int securityLevel){
	sqlServer = server;
	sqlUsername = username;
	sqlPassword = password;
	sqlDatabase = database;
	prefix = tablePrefix;
	setSecurityLevel(securityLevel);
	try{
		connection = new TCPConnection(server.c_str(),database.c_str(),username.c_str(),password.c_str());
	}
	catch (mysqlpp::ConnectionFailed e){
		connection = new TCPConnection();
	}
}

bool Authenticator::authenticate(string username, string password){
	int errorCode;
	return (authenticate(username, password, errorCode));
}

bool Authenticator::authenticate(string username, string password, int &errorCode){
	try{
		for (unsigned int i=0; i<username.length(); i++) username[i] = tolower(username[i]);
		if (!isSqlSafe(username)){
			errorCode = 3; //invalid username, possible sql injection attempt.
			return false;
		}
		if (!connection->connected()){
			connection->connect(sqlServer.c_str(), sqlDatabase.c_str(), sqlUsername.c_str(), sqlPassword.c_str());
			if (!connection->connected()){
				errorCode = 0;	//unable to connect to mysql server
				return false;
			}
		}
		Query qry = connection->query("");
		string qryText = "SELECT username_clean, user_password FROM " + prefix + "users WHERE username_clean = '" + username + "'";
		StoreQueryResult results = qry.store(qryText.c_str(),qryText.length());

		if (results.num_rows() == 0){
			errorCode = 1; //username not found
			return false;
		}
		PHPBB3Password hasher;
		if (!hasher.check_hash(password, results[0]["user_password"].c_str())){
			errorCode = 2; //invalid password
			return false;
		}
		return true; //password is correct
	}
	catch(mysqlpp::ConnectionFailed e){
		errorCode = 0; //unable to connect to mysql server
		return false;
	}
	catch (mysqlpp::BadQuery e){
		errorCode = 0; //unable to connect to mysql server
		return false;
	}
}

void Authenticator::setSecurityLevel(int level){
	securityLevel = level;
}

bool Authenticator::isSqlSafe(string s){
	string dangerousCharacters = "(\"*^';&></) ";
	switch (securityLevel){
		case 0:
			return true;
		case 1:
			for (unsigned int i=0; i<dangerousCharacters.length(); i++) if (strchr(s.c_str(),dangerousCharacters[i])) return false;
			return true;
		case 2:
		default:
			for (unsigned int i=0; i<s.length(); i++) if(!isalnum(s[i])) return false;
			return true;
	}
	
}
