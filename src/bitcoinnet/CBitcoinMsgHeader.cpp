/*
 * CBitcoinMsgHeader.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#include "CBitcoinMsgHeader.h"
#include "bitcoinconfig.h"


namespace Bitcoin
{
	CBitcoinMsgHeader::CBitcoinMsgHeader():CMessageHeader()
	{
		memcpy(pchMessageStart, BitcoinMessageStart, MESSAGE_START_SIZE);
	}

	CBitcoinMsgHeader::CBitcoinMsgHeader(const char* pszCommand, unsigned int nMessageSizeIn)
		: CMessageHeader(pszCommand, nMessageSizeIn)
	{
		memcpy(pchMessageStart, BitcoinMessageStart, MESSAGE_START_SIZE);
	}

	bool CBitcoinMsgHeader::IsValid() const
	{
	    // Check start string
	    if (memcmp(pchMessageStart, BitcoinMessageStart, MESSAGE_START_SIZE) != 0)
	        return false;

	    // Check the command string for errors
	    for (const char* p1 = pchCommand; p1 < pchCommand + COMMAND_SIZE; p1++)
	    {
	        if (*p1 == 0)
	        {
	            // Must be all zeros after the first zero
	            for (; p1 < pchCommand + COMMAND_SIZE; p1++)
	                if (*p1 != 0)
	                    return false;
	        }
	        else if (*p1 < ' ' || *p1 > 0x7E)
	            return false;
	    }

	    // Message size
	    if (nMessageSize > MAX_SIZE)
	    {
	        LogPrintf("CMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand(), nMessageSize);
	        return false;
	    }
	    return true;
	}
} /* namespace Bitcoin */
