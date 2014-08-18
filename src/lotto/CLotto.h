/*
 * CLotto.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CLOTTO_H_
#define CLOTTO_H_
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include "CBetData.h"
#include "CBet.h"
#include "core.h"
#include "CBet156.h"
#include "CBetMulity.h"
#include "lotteryfile.h"
#include "CEncryptKey.h"
namespace lotto {
extern const unsigned char gSplitFlag;
extern CObjectFile DspayKeyFile;
class CLotto {

	std::vector<boost::shared_ptr<CBet> > vBet;
	uint256 hash;
	int64_t rewardPool;
	CLottoFileKey key;
	std::map<BET_TYPE, std::map<RWD_TYPE, int64_t> > mRewardScheme;
	std::map<BET_TYPE, std::vector<unsigned char> > mLuckyStar;
	std::map<BET_TYPE, std::map<RWD_TYPE, int64_t> > BetCoins;

public:
	int GetReWardOutput(std::vector<CTxOut>& vOuts, int64_t& llRemains);
	CLotto(uint256 uHash, int64_t iRewardPool,const std::vector<CTxOut>& txOutV);
	CLotto(uint256 uBitcoinHash, int lottoID,uint256 nsecrethash,int64_t iRewardPool,const std::vector<CTxOut>& txOutV);
	~CLotto()
	{
		vBet.clear();
		BetCoins.clear();
		mRewardScheme.clear();
	}
	boost::shared_ptr<CBet> CreateCbet(const std::vector<unsigned char> pLotto);
	static bool IsValid(std::vector<unsigned char>& pLotto,int64_t nValue);
	void Init(const std::vector<CTxOut>& txOutV);
	bool print()const ;
	std::map<BET_TYPE, std::vector<unsigned char> >&GetLuckyNum(){ return mLuckyStar;};
	string ComputePeerReward(string &nStr);
private:
	int CreateLuckyStarMap();
	void CreateRewardSchemeMap();
	int CreateStatisticsRewardMap();
private:
	CLotto(){ rewardPool = 0;};
};

} /* namespace lotto */

#endif /* CLOTTO_H_ */
