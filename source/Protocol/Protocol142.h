
// Protocol142.h

// Interfaces to the cProtocol142 class representing the release 1.4.2 protocol (#47)





#pragma once

#include "Protocol132.h"
#include "../../CryptoPP/modes.h"
#include "../../CryptoPP/aes.h"





class cProtocol142 :
	public cProtocol132
{
	typedef cProtocol132 super;
public:

	cProtocol142(cClientHandle * a_Client);
	virtual ~cProtocol142();

	// Sending commands (alphabetically sorted):
	virtual void SendSoundParticleEffect(int a_EffectID, int a_SrcX, int a_SrcY, int a_SrcZ, int a_Data) override;
	virtual void SendTimeUpdate         (Int64 a_WorldTime) override;

	// Specific packet parsers:
	virtual int ParseLogin              (void) override;
} ;



