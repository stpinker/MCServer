#pragma once
#include "BlockHandler.h"


class cBlockGravelHandler : public cBlockHandler
{
public:
	cBlockGravelHandler(BLOCKTYPE a_BlockID)
		: cBlockHandler(a_BlockID)
	{
	}

	virtual AString GetStepSound(void) override
	{
		return "step.gravel";
	}
	
};