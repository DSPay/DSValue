/*
 * CLotto.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#include "CLotto.h"
#include "key.h"
#include "script.h"
#include "CBetData.h"

namespace lotto {

CObjectFile DspayKeyFile;

CLotto::CLotto(uint256 uHash, int64_t iRewardPool, const std::vector<CTxOut>& txOutV) {
	vBet.clear();
	BetCoins.clear();
	mRewardScheme.clear();
	hash = uHash;
	rewardPool = iRewardPool;
	Init(txOutV);
}
void CLotto::Init(const std::vector<CTxOut>& txOutV) {
	if (!CreateLuckyStarMap())
		return;
	BOOST_FOREACH(const CTxOut& v, txOutV) {
		std::vector<unsigned char> vBetData;
		if (v.scriptPubKey.GetBetData(vBetData)) {
			boost::shared_ptr<CBet> betdata = CreateCbet(vBetData);
			*betdata << v.nValue << mLuckyStar[betdata.get()->type];
//			betdata.get()->print();
			betdata.get()->CreateRewardMap();
			vBet.push_back(betdata);
		} else {
			Assert(0);
		}

	}
}
CLotto::CLotto(uint256 uBitcoinHash, int lottoID, uint256 nsecrethash, int64_t iRewardPool,
		const std::vector<CTxOut>& txOutV) :
		key(lottoID, nsecrethash) {
	LogPrintf("start");
	vBet.clear();
	BetCoins.clear();
	mRewardScheme.clear();
	rewardPool = iRewardPool;
	hash = uBitcoinHash;

	vector<unsigned char> tep;
	if (DspayKeyFile.ReadClottoData(lottoID, nsecrethash, tep)) {
		hash = Hash(hash.begin(), hash.end(), tep.begin(), tep.end());
	} else {
		hash = uint256(0);
	}
	Init(txOutV);
	LogPrintf("end");
}
int CLotto::GetReWardOutput(std::vector<CTxOut> &vOuts, int64_t &llRemains) {
	LogPrintf("GetReWardOutput start");
	CreateRewardSchemeMap();
	CreateStatisticsRewardMap();
	BOOST_FOREACH(boost::shared_ptr<CBet> &v, vBet) {

		if (v.get()->isWinning()) {
			std::map<RWD_TYPE, int64_t> reward = v.get()->GetBetRewardMap();
			std::map<RWD_TYPE, int64_t>::iterator item;
			CTxOut tx;
			tx.nValue = 0;
			string tem = "";
			tem += tfm::format("\n\n BET_TYPE:%d",v.get()->type);
			tem += tfm::format("\n\n totalvalue:%ld,peerbetValut:%ld,total bet:%d",v.get()->nValue,v.get()->nValue/v.get()->getTotalBer(),v.get()->getTotalBer());
			tem += tfm::format("\n\n luckeynum:%d",memdump(v.get()->vLuckyStar));
			tem += tfm::format("\n\n selectnumber:%d",memdump(v.get()->vch));
			for (item = reward.begin(); item != reward.end(); item++) {
				if (mRewardScheme[v.get()->type].count(item->first) != 0) {
					tx.nValue += (int64_t) (mRewardScheme[v.get()->type][item->first] * item->second);
					tem += tfm::format("\n\n lotto::npeerBetValue1:%ld,%ld,value:%ld", mRewardScheme[v.get()->type][item->first],
												item->second, mRewardScheme[v.get()->type][item->first] * item->second);
				} else {
					tx.nValue += v.get()->GetMulit(item->first) * item->second;
					tem += tfm::format("\n\n RWD_TYPE:%d,the betmulit:%ld,%ld,value:%ld",item->first, v.get()->GetMulit(item->first),
							item->second, v.get()->GetMulit(item->first) * item->second);
				}

			}
			tx.scriptPubKey.SetDestination(CKeyID(v.get()->address));
			tem += tfm::format("\n\n lotto value:%ld",tx.nValue);
			LogTrace("minner", "\n\n---%s",tem);
			vOuts.push_back(tx);
			llRemains += tx.nValue;
//			LogPrintf("\nGetReWardOutput value:%ld  mRewardScheme %ld\n", item->second,
//					mRewardScheme[v.get()->type][item->first]);
//			LogPrintf("\ntype =%d rewardPool:%ld\n", item->first, tx.nValue);

		}
	}
	LogPrintf("GetReWardOutput end");
	return 0;

}

string CLotto::ComputePeerReward(string &nStr)
{
	std::map<BET_TYPE, std::map<RWD_TYPE, int64_t> > mReward;
	CreateRewardSchemeMap();
	CreateStatisticsRewardMap();
	int64_t peerValue = 0;
	int64_t Countbet = 0;
	nStr = "";
	BOOST_FOREACH(boost::shared_ptr<CBet> &v, vBet) {

		if (v.get()->isWinning()) {
			std::map<RWD_TYPE, int64_t> reward = v.get()->GetBetRewardMap();
			std::map<RWD_TYPE, int64_t>::iterator item;
			CTxOut tx;
			Countbet = v.get()->getTotalBer();
			peerValue = v.get()->nValue/Countbet;
			tx.nValue = 0;
			string tem = "";
			for (item = reward.begin(); item != reward.end(); item++) {
				if (mRewardScheme[v.get()->type].count(item->first) != 0) {
					mReward[v.get()->type][item->first]= (int64_t) (mRewardScheme[v.get()->type][item->first] * item->second);
				} else {
					mReward[v.get()->type][item->first]= v.get()->GetMulit(item->first) * item->second;

				}

			}
		}
	}
	string str = "";
	if(mReward.size()>0)
	{
		std::map<lotto::BET_TYPE, std::map<lotto::RWD_TYPE, int64_t> >::iterator item;
		item = mReward.begin();
		for(item = mReward.begin();item != mReward.end();item++)
		{
			string first= "";
			if(item->first == lotto::TYPE_15_6)
			{
				first = "TYPE_15_6:";
			}
			if(item->first == lotto::TYPE_MULILT)
			{
				first = "TYPE_MULILT:";
			}
			std::map<lotto::RWD_TYPE, int64_t> it = item->second;
			string second ="";
			BOOST_FOREACH(const PAIRTYPE(lotto::RWD_TYPE, int64_t)& tep2 , it)
			{

				int pcount = tep2.second/peerValue;
				second+=tfm::format("Prize level:%d,Total Tickets:%d,",tep2.first+1,pcount);
				double count = tep2.second/100000000.0;
				second +=tfm::format("Prize amount:%f, ",count);
			}
			if(second != "")
			{
				nStr +=tfm::format("%s,",first);
				nStr +=tfm::format("Draw No:%d,Winning No:%s,Amout per bet:%f,Total Tickets:%ld,",key.Index,HexStr(mLuckyStar[item->first].begin(),mLuckyStar[item->first].end()),peerValue/100000000.0,Countbet);
				str +=tfm::format("%s}",second);
			}
		}

	}

	return str;
}

int CLotto::CreateLuckyStarMap() {
	if (hash == uint256(0))
		return 0;
	uint256 hashtep = hash;

	Assert(hash != NULL);
	int temp = 0;
	do {
		vector<unsigned char> plukeyNum;
		hashtep = Hash(hash.begin(), hash.end());
		switch (temp) {
		case TYPE_15_6: {
			mLuckyStar[(BET_TYPE) temp].clear();
			do {
				CBigNum tem(hashtep);
				unsigned char temp = tem.getvch()[0];
				if (temp < 15) {
					temp += 1;
					std::vector<unsigned char>::iterator it = find(plukeyNum.begin(), plukeyNum.end(), temp);
					if (it == plukeyNum.end())
						plukeyNum.push_back(temp);
				}
				hashtep = Hash(hashtep.begin(), hashtep.end());
			} while (plukeyNum.size() < 6);
			sort(plukeyNum.begin(), plukeyNum.end());
			mLuckyStar[(BET_TYPE) temp] = plukeyNum;
			LogTrace("minner","\n bet15_6 mLuckyStar%s\n", VectorToString(plukeyNum));
			plukeyNum.clear();
		}
			break;
		case TYPE_MULILT: {
			mLuckyStar[(BET_TYPE) temp].clear();
			do {
				CBigNum tem(hashtep);
				unsigned char temp = tem.getvch()[0];
				if (temp < 20) {
					temp += 1;
					std::vector<unsigned char>::iterator it = find(plukeyNum.begin(), plukeyNum.end(), temp);
					if (it == plukeyNum.end())
						plukeyNum.push_back(temp);
				}
				hashtep = Hash(hashtep.begin(), hashtep.end());
			} while (plukeyNum.size() < 5);

			//mLuckyStar[(BET_TYPE) temp] = plukeyNum;
			sort(plukeyNum.begin(), plukeyNum.end());
			plukeyNum.push_back(lotto::gSplitFlag);
			do {
				CBigNum tem(hashtep);
				unsigned char temp = tem.getvch()[0];
				if (temp < 10) {
					temp += 1;
					std::vector<unsigned char>::iterator it = find(plukeyNum.begin(), plukeyNum.end(), temp);
					if (it == plukeyNum.end())
						plukeyNum.push_back(temp);
				}
				hashtep = Hash(hashtep.begin(), hashtep.end());
			} while (plukeyNum.size() < 8);
			std::vector<unsigned char>::iterator it = std::find(plukeyNum.begin(), plukeyNum.end(), lotto::gSplitFlag);
			sort(it + 1, plukeyNum.end());
			mLuckyStar[(BET_TYPE) temp] = plukeyNum;
			LogTrace("minner","\n betmulit mLuckyStar%s\n", VectorToString(plukeyNum));
			plukeyNum.clear();
		}
			break;
		default:
			//assert(0);
			break;
		}

		temp++;
	} while (temp < typenull);

	BOOST_FOREACH(const PAIRTYPE(BET_TYPE, vector<unsigned char>)& tep2 , mLuckyStar)
	//BOOST_FOREACH(const PAIRTYPE(RWD_TYPE, std::vector<unsigned char>) tep2 , mLuckyStar)
	{
		LogTrace("bess3", "key:%d value:%s\n", tep2.first, VectorToString(tep2.second));
	}
	return 1;
}

void CLotto::CreateRewardSchemeMap() {
	BOOST_FOREACH(boost::shared_ptr<CBet> &v, vBet) {
		if (v.get()->isWinning()) {
			std::map<RWD_TYPE, int64_t> reward = v.get()->GetBetRewardMap();
			std::map<RWD_TYPE, int64_t> &peermultiple = BetCoins[v.get()->type];
			for (std::map<RWD_TYPE, int64_t>::iterator item = reward.begin(); item != reward.end(); item++)
				peermultiple[item->first] += item->second;
		}
	}

}
int CLotto::CreateStatisticsRewardMap() {
	std::map<BET_TYPE, std::map<RWD_TYPE, int64_t> > reward_m;
	std::map<RWD_TYPE, int64_t> reward_total;
	mRewardScheme.clear();

	BOOST_FOREACH(boost::shared_ptr<CBet> &v, vBet) {
		if (v.get()->isWinning()) {
			std::map<RWD_TYPE, int64_t> reward = v.get()->GetBetRewardMap();
			std::map<RWD_TYPE, int64_t>::iterator item;
			for (item = reward.begin(); item != reward.end(); item++) {
				double multi = v.get()->GetMulit(item->first);
				reward_total[item->first] = reward_total[item->first] + multi * item->second;
				reward_m[v.get()->type].insert(make_pair(item->first, item->second * multi)); //  std::map<item->first,item->second * multi>);
			}

		}
	}

	int64_t availablereward = rewardPool * 8 / 10; //80%

	for (int eRwdType = TOP_1; eRwdType < TOP_null; eRwdType++) {
		int64_t _maxreward = 0;
		RWD_TYPE rt = RWD_TYPE(eRwdType);

		if (eRwdType == TOP_null - 1) {
			_maxreward = availablereward;
		} else {
			_maxreward = availablereward * 8 / 10; //80%
		}
		availablereward = availablereward - _maxreward;
		if (reward_total[rt] > _maxreward) {
			std::map<BET_TYPE, std::map<RWD_TYPE, int64_t> >::iterator it;
			for (it = reward_m.begin(); it != reward_m.end(); it++) {
				if (BetCoins[it->first].count(rt) > 0 && it->second.count(rt) > 0) {
					LogTrace("print1", "\n\n lotto::CreateStatisticsRewardMap:%ld,%ld,:%ld:%ld.", _maxreward,
							BetCoins[it->first][rt], reward_total[rt],it->second[rt]);
					mRewardScheme[it->first].insert(
							make_pair(rt, _maxreward / BetCoins[it->first][rt] / reward_total[rt] * it->second[rt]));
				}
			}
		} else //caculate 1st reward and left available reward
		{
//			std::map<BET_TYPE, std::map<RWD_TYPE, int64_t> >::iterator it;
//			for (it = reward_m.begin(); it != reward_m.end(); it++) {
//				if (BetCoins[it->first].count(rt) > 0 && it->second.count(rt) > 0) {
//					mRewardScheme[it->first].insert(make_pair(rt, it->second[rt] / BetCoins[it->first][rt]));
//				}
//			}
			availablereward += _maxreward - reward_total[rt];
		}
	}
//	printf("TOP_1: %lf\n", mRewardScheme[TOP_1]);
//	printf("TOP_2: %lf\n", mRewardScheme[TOP_2]);
//	printf("TOP_3: %lf\n", mRewardScheme[TOP_3]);
	return 1;
}
bool CLotto::IsValid(std::vector<unsigned char>& pLotto,int64_t nValue) {
	lotto::CBetData opLotto;
	opLotto << pLotto;
	boost::shared_ptr<CBet> p(new CBet());
	bool isVflag = false;
	switch (opLotto.GetType()) {
	case TYPE_15_6: {
		boost::shared_ptr<CBet_15_6> p(new CBet_15_6());
		(*p <<nValue<< opLotto);
		isVflag = p.get()->IsValid();
	}
		break;
	case TYPE_MULILT: {
		boost::shared_ptr<CBetMulity> p(new CBetMulity());
		(*p << nValue<<opLotto);
		isVflag = p.get()->IsValid();
	}
		break;
	default:
		Assert(0);
		break;
	}
	return isVflag;
}

boost::shared_ptr<CBet> CLotto::CreateCbet(const std::vector<unsigned char> pLotto) {
	lotto::CBetData opLotto;
	opLotto << pLotto;
	boost::shared_ptr<CBet> p(new CBet);
	//opLotto.print();
	switch (opLotto.GetType()) {
	case TYPE_15_6: {
		boost::shared_ptr<CBet> p = boost::shared_ptr<CBet>(new CBet_15_6());
		(*p << opLotto);
		return p;
	}
	case TYPE_MULILT: {
		boost::shared_ptr<CBetMulity> p = boost::shared_ptr<CBetMulity>(new CBetMulity());
		(*p << opLotto);
		p.get()->SplitSelectNum();
		return (boost::shared_ptr<CBet>) p;
	}
	default:
		Assert(0);
		break;
	}
	return p;
}
bool CLotto::print() const {
	std::vector<boost::shared_ptr<CBet> > vBet;
	LogPrintf("CLotto :std::vector<boost::shared_ptr<CBet> > size%d\n", vBet.size());
	BOOST_FOREACH(boost::shared_ptr<CBet> & tep1 , vBet) {
		tep1.get()->print();
	}
	LogPrintf("hash; %s\n", hash.GetHex());
	LogPrintf("rewardPool; %x\n", rewardPool);

	LogPrintf("std::map<RWD_TYPE, int64_t> mRewardScheme ; size:%d\n", mRewardScheme.size());
//	BOOST_FOREACH(const PAIRTYPE(BET_TYPE, std::map<RWD_TYPE, int64_t>)& tep1 , mRewardScheme) {
////		BOOST_FOREACH(const PAIRTYPE(RWD_TYPE, int64_t)& tep2 ,tep1.se)
////		LogPrintf("key:%s value:%d", tep1.first, tep1.second);
//	}

	LogPrintf("std::map<BET_TYPE, std::vector<unsigned char> > mLuckyStar ; size:%d\n", mLuckyStar.size());
	BOOST_FOREACH(const PAIRTYPE(BET_TYPE, vector<unsigned char>)& tep2 , mLuckyStar)
	//BOOST_FOREACH(const PAIRTYPE(RWD_TYPE, std::vector<unsigned char>) tep2 , mLuckyStar)
	{
		LogPrintf("key:%d value:%s\n", tep2.first, VectorToString(tep2.second));
	}

	LogPrintf("std::map<BET_TYPE, std::map<RWD_TYPE, int64_t> > BetCoins ; size:%d  \n Others are not print\n",
			BetCoins.size());

	return true;
}
} /* namespace lotto */

