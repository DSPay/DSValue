/*
 * CBitcoinNetMessage.h
 *
 *  Created on: May 29, 2014
 *      Author: ranger.shi
 */

#ifndef CBITCOINNETMESSAGE_H_
#define CBITCOINNETMESSAGE_H_

#include "CBitcoinMsgHeader.h"

namespace Bitcoin {

	class CBitcoinNetMessage
	{
		public:
			CBitcoinNetMessage(int nTypeIn, int nVersionIn) :
				hdrbuf(nTypeIn, nVersionIn), vRecv(nTypeIn, nVersionIn)
			{
		        hdrbuf.resize(24);
		        in_data = false;
		        nHdrPos = 0;
		        nDataPos = 0;
			}
			bool complete() const
			{
				if (!in_data)
					return false;
				return (hdr.nMessageSize == nDataPos);
			}
			void SetVersion(int nVersionIn)
			{
				hdrbuf.SetVersion(nVersionIn);
				vRecv.SetVersion(nVersionIn);
			}
		public:
		    int readHeader(const char *pch, unsigned int nBytes);
		    int readData(const char *pch, unsigned int nBytes);
		public:
		    bool in_data;                   // parsing header (false) or data (true)
		    CDataStream hdrbuf;             // partially received header
		    CBitcoinMsgHeader hdr;         // complete header
		    unsigned int nHdrPos;
		    CDataStream vRecv;              // received message data
		    unsigned int nDataPos;
	};

} /* namespace Bitcoin */

#endif /* CBitcoinNETMESSAGE_H_ */
