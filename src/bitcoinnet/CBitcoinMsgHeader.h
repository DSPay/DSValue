/*
 * CBitcoinMsgHeader.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CBITCOINMSGHEADER_H_
#define CBITCOINMSGHEADER_H_

#include "protocol.h"
#include "util.h"

namespace Bitcoin {
	class CBitcoinMsgHeader: public CMessageHeader
	{
	public:
		CBitcoinMsgHeader();
		CBitcoinMsgHeader(const char* pszCommand, unsigned int nMessageSizeIn);
		bool IsValid() const;
	};

} /* namespace Bitcoin */

#endif /* CBitcoinMSGHEADER_H_ */
