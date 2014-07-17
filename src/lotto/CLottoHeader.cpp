/*
 * CLottoHeader.cpp
 *
 *  Created on: May 29, 2014
 *      Author: ranger.shi
 */

#include "CLottoHeader.h"
#include <boost/foreach.hpp>
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;

namespace lotto {

CLottoHeader::CLottoHeader(const CLottoHeader &obj)
{
	this->nVersion = obj.nVersion;
	this->mBitcoinHash.insert(obj.mBitcoinHash.begin(), obj.mBitcoinHash.end());
	this->nLottoID = obj.nLottoID;
	this->uLottoKey = obj.uLottoKey;
	this->llPool = obj.llPool;
}

CLottoHeader& CLottoHeader::operator=(const CLottoHeader &obj){
	if(this == &obj)
		return *this;
	this->nVersion = obj.nVersion;
	this->mBitcoinHash.insert(obj.mBitcoinHash.begin(), obj.mBitcoinHash.end());
	this->nLottoID = obj.nLottoID;
	this->uLottoKey = obj.uLottoKey;
	this->llPool = obj.llPool;
	return *this;
}

uint256 CLottoHeader::GetLastBitcoinHash() const
{
	if(!mBitcoinHash.empty())
	{
		std::map<int, uint256>::const_reverse_iterator iterEnd = mBitcoinHash.rbegin();
		return iterEnd->second;
	}
	return uint256(0);
}

int CLottoHeader::GetLastBitcoinHeight() const
{
	if(!mBitcoinHash.empty())
	{
		std::map<int, uint256>::const_reverse_iterator  iterEnd = mBitcoinHash.rbegin();
		return iterEnd->first;
	}
	return 0;
}

std::string CLottoHeader::ToString() const
{

	//sprintf(psz + i*2, "%02x", ((unsigned char*)pn)[sizeof(pn) - i - 1]);
	std::string str="";

	char buffer[20];
	sprintf(buffer,"%d",nVersion);
	str += "(";
	str += "nVersion:";
	str += buffer;
	BOOST_FOREACH(const PAIRTYPE(int, uint256)& item, mBitcoinHash)
	{
		str += ",height:";
//		str += itoa(item.first, buffer, 10);
		sprintf(buffer,"%d",item.first);
		str += buffer;
		str += ",hash:"+ item.second.ToString();
	}
	str += ",llpool:";
//	str += itoa(llPool,buffer,16);
	sprintf(buffer,"%lu",llPool);
	str += buffer;
	str += ",nLottoID:";
	sprintf(buffer, "%d", nLottoID);
	str += buffer;
//	str += itoa(nLottoID, buffer, 10);
	str += ",uLottoKey:";
	str += uLottoKey.ToString();
	str += ")";

	return str;
}

uint256 CLottoHeader::GetFirstBitcoinHash() const
{
	if(!mBitcoinHash.empty())
	{
		std::map<int, uint256>::const_iterator iterBegin = mBitcoinHash.begin();
		return iterBegin->second;
	}
	return uint256(0);
}

int CLottoHeader::GetFirstBitcoinHeight() const
{
	if(!mBitcoinHash.empty())
	{
		std::map<int, uint256>::const_iterator iterBegin = mBitcoinHash.begin();
		return iterBegin->first;
	}
	return 0;
}

} /* namespace lotto */
