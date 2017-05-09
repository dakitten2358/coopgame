#include "CoopGame.h"
#include "CoopGameOnlineSessionSettings.h"

FCoopGameOnlineSessionSettings::FCoopGameOnlineSessionSettings(bool isLan, bool isPresence, int32 maxNumPlayers)
{
	NumPublicConnections = maxNumPlayers;
	if (NumPublicConnections < 0)
		NumPublicConnections = 0;
	NumPrivateConnections = 0;
	bIsLANMatch = isLan;
	bShouldAdvertise = true;
	bAllowJoinInProgress = true;
	bAllowInvites = true;
	bUsesPresence = isPresence;
	bAllowJoinViaPresence = true;
	bAllowJoinViaPresenceFriendsOnly = true;
}

FCoopGameOnlineSessionSettings::~FCoopGameOnlineSessionSettings()
{}
