/*
 * CLottoHeader.h
 *
 *  Created on: May 29, 2014
 *      Author: ranger.shi
 */

#ifndef CLOTTOHEADER_H_
#define CLOTTOHEADER_H_
#include "serialize.h"
#include "uint256.h"
namespace lotto {

class CLottoHeader {
public:
	static const int CURRENT_VERSION = 2;
	int nVersion;
	std::map<int, uint256> mBitcoinHash;
	int64_t llPool;
	int nLottoID;
	uint256 uLottoKey;

	IMPLEMENT_SERIALIZE
	(
	     READWRITE(nVersion);
	     READWRITE(mBitcoinHash);
	     READWRITE(llPool);
	     READWRITE(nLottoID);
	     READWRITE(uLottoKey);
	)
public:
	CLottoHeader(const CLottoHeader &obj);
	CLottoHeader& operator=(const CLottoHeader &obj);
	uint256 GetLastBitcoinHash() const;
	int GetLastBitcoinHeight() const;
	uint256 GetFirstBitcoinHash() const;
	int GetFirstBitcoinHeight() const;
	std::string ToString() const;
	CLottoHeader():nVersion(1),llPool(0),nLottoID(1),uLottoKey(0) {
		mBitcoinHash.clear();
	}
	~CLottoHeader() {

	}
};

} /* namespace lotto */

#endif /* CLOTTOHEADER_H_ */
