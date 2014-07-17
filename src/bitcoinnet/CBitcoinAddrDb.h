/*
 * CBitcoinAddrDb.h
 *
 *  Created on: Jun 4, 2014
 *      Author: ranger.shi
 */

#ifndef CBITCOINADDRDB_H_
#define CBITCOINADDRDB_H_

#include "addrman.h"
#include <string>

namespace Bitcoin {

class CBitcoinAddrDb
{
	public:
		CBitcoinAddrDb(const char* fileName = "BitcoinPeers.dat");
	    bool Write(const CAddrMan& addr);
	    bool Read(CAddrMan& addr);
	private:
	    std::string 	m_addrFileName;
};

} /* namespace Bitcoin */

#endif /* CBitcoinADDRDB_H_ */
