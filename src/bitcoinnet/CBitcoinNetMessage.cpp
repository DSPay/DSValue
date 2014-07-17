/*
 * CBitcoinNetMessage.cpp
 *
 *  Created on: May 29, 2014
 *      Author: ranger.shi
 */

#include "CBitcoinNetMessage.h"

namespace Bitcoin
{
	int CBitcoinNetMessage::readHeader(const char *pch, unsigned int nBytes)
	{
	    // copy data to temporary parsing buffer
	    unsigned int nRemaining = 24 - nHdrPos;
	    unsigned int nCopy = std::min(nRemaining, nBytes);

	    memcpy(&hdrbuf[nHdrPos], pch, nCopy);
	    nHdrPos += nCopy;

	    // if header incomplete, exit
	    if (nHdrPos < 24)
	        return nCopy;
	    // deserialize to CMessageHeader
	    try {
	        hdrbuf >> hdr;
	    }
	    catch (std::exception &e) {
	        return -1;
	    }
	    // reject messages larger than MAX_SIZE
	    if (hdr.nMessageSize > MAX_SIZE)
	            return -1;
	    // switch state to reading message data
	    in_data = true;
	    vRecv.resize(hdr.nMessageSize);
	    return nCopy;
	}
	int CBitcoinNetMessage::readData(const char *pch, unsigned int nBytes)
	{
	    unsigned int nRemaining = hdr.nMessageSize - nDataPos;
	    unsigned int nCopy = std::min(nRemaining, nBytes);

	    memcpy(&vRecv[nDataPos], pch, nCopy);
	    nDataPos += nCopy;

	    return nCopy;
	}

} /* namespace Bitcoin */
