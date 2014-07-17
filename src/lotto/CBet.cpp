/*
 * CBet.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#include "CBet.h"
#include "boost/format.hpp"
namespace lotto {
//const unsigned char gSplitFlag = 0xFF;

bool CBet::IsValid() const {
		//check is unique data
	    std::vector<unsigned char> tep = vch;
		//sort(tep.begin(), tep.end());
	    if (unique(tep.begin(), tep.end()) != tep.end()) {
			return false;
		}
		if (type >= typenull)
			return false;
		if (CTransaction::nMinBet* getTotalBer() >= nValue)
				return false;
		return true;
	}
bool CBet::print() const{
	std::string tem ="";
	tem += tfm::format("\n\nBET_TYPE type: %d\n", type);
	tem += tfm::format("uint160 address: %d\n", address.GetHex());
	tem += tfm::format("std::vector<unsigned char> vch :%s\n",VectorToString(vch));
	tem += tfm::format("std::vector<unsigned char> vLuckyStar :%s\n",VectorToString(vLuckyStar));
	tem += tfm::format("std::vector<unsigned char> rewardmap :%s\n","not print");
	tem += tfm::format("nValue :%d\n\n",nValue);
	LogTrace2("print","%s",tem);
    return true;
}
} /* namespace lotto */


