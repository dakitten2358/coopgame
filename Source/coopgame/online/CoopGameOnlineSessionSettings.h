#pragma once

#include "OnlineSessionSettings.h"

class FCoopGameOnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FCoopGameOnlineSessionSettings(bool isLan = false, bool isPresence = false, int32 maxNumPlayers = 4);
	virtual ~FCoopGameOnlineSessionSettings();
};
