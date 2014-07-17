/*
 * CBet.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CBET_H_
#define CBET_H_

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "CBetData.h"
#include "main.h"
namespace lotto {
using namespace std;

const unsigned char gSplitFlag = 0xFF;
class CBet {



public:
	BET_TYPE type;
	uint160 address;
	std::vector<unsigned char> vch;

	//only in memory
	std::vector<unsigned char> vLuckyStar;
	std::vector<unsigned char> vaddLuckyStar;
	/// record the reward
	std::map<RWD_TYPE, int64_t> rewardmap;
	int64_t nValue;

public:

	CBet() {
		SetNull();
	}

	virtual ~CBet() {
	}

	//input CBetData
	CBet& operator<<(CBetData &betdata) {
		type = betdata.GetType();
		address = betdata.GetAddr();
		vch = betdata.GetSelectV();
		return *this;
	}
	//input LuckyStar
	CBet& operator<<(const vector<unsigned char> &vluckly) {
		Assert(vLuckyStar.empty());
		vLuckyStar.assign(vluckly.begin(), vluckly.end());
		return *this;
	}
	//input vavul
	CBet& operator<<(int64_t val) {
		nValue = val;
		return *this;
	}

	static int64_t factorial(int64_t n) {
		if (n <= 1)
			return 1;
		else
			return n * factorial(n - 1);
	}
	/* select n from m*/
	static int MselectN(int m, int n) {
		//printf("Mselectm = %d n =%d:\n",m,n);
		Assert(m >= 0 && n >= 0);
		if(n == 0) return 1;
		if(m < n) return 0;
		if (m < 1 || m == n)
			return 1;
		return factorial(m) / (factorial(m - n) * factorial(n));
	}
	std::map<RWD_TYPE, int64_t>& GetBetRewardMap(void) {
		return rewardmap;
	}

	int virtual getTotalBer() const {
		throw error("inValid bet data");
	}

	void SetNull() {
		vch.clear();
		rewardmap.clear();
		address = 0;
		nValue = 0;
		type = typenull;
	}

	virtual bool IsValid() const;
	bool isWinning() const{return rewardmap.size() !=0;}
    virtual void CreateRewardMap()  {throw error("inValid bet data"); }
    virtual int64_t GetMulit(int type) const{ return 0;}
    virtual bool print()const;
};

} /* namespace lotto */

#endif /* CBET_H_ */
