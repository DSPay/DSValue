/*
 * CBetMulity.cpp
 *
 *  Created on: Jun 7, 2014
 *      Author: ranger.shi
 */

#include "CBetMulity.h"
#include <boost/foreach.hpp>
#include <algorithm>    // std::find#include <vector>

namespace lotto {


static const unsigned char READ_MAX_NO = 20;
static const unsigned char READ_MAX_NICE_NO = 5;
static const unsigned char BLUE_MAX_NICE_NO = 2;
static const unsigned char BLUE_MAX_NO = 10;
CBetMulity::CBetMulity() {
	multiplying[0] = 210000;
	multiplying[1] = 1000;
	multiplying[2] = 30;
	multiplying[3] = 5;
	multiplying[4] = 2;

//	CBet::CBet();
}
void CBetMulity::SplitSelectNum() {
	std::vector<unsigned char>::iterator it = std::find(vch.begin(), vch.end(), lotto::gSplitFlag);
	//cout<<"CBetMulity selectnumber:"<<VectorToString(vch)<<endl;
	if (it != vch.end()) {
		vRed.assign(vch.begin(), it);
		vBlue.assign(it + 1, vch.end());
		sort(vRed.begin(), vRed.end());
		sort(vBlue.begin(), vBlue.end());
	} else {
		LogTrace("CBetMulity selectnum is error:%s", __FUNCTION__);
	}
}
bool CBetMulity::IsValid() const {

	//20 select 5  and 10 select 2
	assert(type == TYPE_MULILT);
	std::vector<unsigned char> vselect = vch;
	if (vch.size() < READ_MAX_NICE_NO + BLUE_MAX_NICE_NO  || vch.size() > 31) // 5 + 2 +1  gSplitFlag
		return false;

	std::vector<unsigned char>::iterator it = std::find(vselect.begin(), vselect.end(), lotto::gSplitFlag);
	if (it == vselect.end()) //must have gSplitFlag
		return false;

	int len = it - vselect.begin();
	if (len < READ_MAX_NICE_NO) // first atlist have five no
		return false;

	if ((vselect.size() - len - 1) < BLUE_MAX_NICE_NO) //secend must have more than 2
		return false;

	if (it != std::find_if(vselect.begin(), it , boost::bind(std::greater<unsigned char>(), _1, READ_MAX_NO + 1)))
		return false;

	if (it != std::find_if(vselect.begin(), it , boost::bind(std::less<unsigned char>(), _1, 1)))
		return false;

	if (vselect.end() != std::find_if(it + 1, vselect.end(), boost::bind(std::greater<unsigned char>(), _1, BLUE_MAX_NO + 1)))
		return false;

	if (vselect.end() != std::find_if(it + 1, vselect.end(), boost::bind(std::less<unsigned char>(), _1, 1)))
		return false;

	if (unique(vselect.begin(), it) != it || unique(it, vselect.end()) != vselect.end()) {
		return false;
	}

	///// the number is paixu or not
	std::vector<unsigned char> pretep(vselect.begin(), it);
	std::vector<unsigned char> tep(vselect.begin(), it );

	sort(tep.begin(), tep.end());
	if (tep != pretep)
		return false;
	pretep.clear();
	tep.clear();
	tep.assign(it + 1, vselect.end());
	pretep.assign(it + 1, vselect.end());
	sort(tep.begin(), tep.end());
	if (tep != pretep)
		return false;
	if (type >= typenull)
		return false;

	if ((CTransaction::nMinBet* getTotalBer() >= nValue))
		return false;

	return true;
}

void CBetMulity::CalcRewardBet(int redCount, int blueCount) {

	//cout<<"CalcRewardBet"<<preluckyCount<<nextluckycount<<endl;
	int64_t npeerBetValue = nValue / getTotalBer();
	int64_t temp = 0;
	//5+2
	if (redCount >= 5 && blueCount >= 2)
		rewardmap[TOP_1] = 1;

	//5 + 1   4+2

	temp = MselectN(redCount, 4) * MselectN(vRed.size() - redCount, 1) * MselectN(blueCount, 2)
			* MselectN(vBlue.size() - blueCount, 0)
			+ MselectN(redCount, 5) * MselectN(vRed.size() - redCount, 0) * MselectN(blueCount, 1)
					* MselectN(vBlue.size() - blueCount, 1);
	if (temp > 0) {
		rewardmap[TOP_2] = temp;
	}
	// 4+1 5+0  3+2
	temp = MselectN(redCount, 4) * MselectN(vRed.size() - redCount, 1) * MselectN(blueCount, 1)
			* MselectN(vBlue.size() - blueCount, 1)
			+ MselectN(redCount, 5) * MselectN(vRed.size() - redCount, 0) * MselectN(blueCount, 0)
					* MselectN(vBlue.size() - blueCount, 2)
			+ MselectN(redCount, 3) * MselectN(vRed.size() - redCount, 2) * MselectN(blueCount, 2)
					* MselectN(vBlue.size() - blueCount, 0);
	if (temp > 0) {
		rewardmap[TOP_3] = temp;
	}
	// 4+0 3+1 2+2
	temp = MselectN(redCount, 4) * MselectN(vRed.size() - redCount, 1) * MselectN(blueCount, 0)
			* MselectN(vBlue.size() - blueCount, 2)
			+ MselectN(redCount, 3) * MselectN(vRed.size() - redCount, 2) * MselectN(blueCount, 1)
					* MselectN(vBlue.size() - blueCount, 1)
			+ MselectN(redCount, 2) * MselectN(vRed.size() - redCount, 3) * MselectN(blueCount, 2)
					* MselectN(vBlue.size() - blueCount, 0);
	if (temp > 0) {
		rewardmap[TOP_4] = temp;
	}
	//3+0 2+1 0+2
	temp = MselectN(redCount, 3) * MselectN(vRed.size() - redCount, 2) * MselectN(blueCount, 0)
				* MselectN(vBlue.size() - blueCount, 2)
				+ MselectN(redCount, 2) * MselectN(vRed.size() - redCount, 3) * MselectN(blueCount, 1)
						* MselectN(vBlue.size() - blueCount, 1)
				+ MselectN(redCount, 0) * MselectN(vRed.size() - redCount, 5) * MselectN(blueCount, 2)
						* MselectN(vBlue.size() - blueCount, 0);
		if (temp > 0) {
			rewardmap[TOP_5] = temp;
		}

	if (rewardmap.count(TOP_1) > 0) {
		rewardmap[TOP_1] *= npeerBetValue;
		LogTrace2("print", "\n\n CBetMulity::TOP_1:%ld", rewardmap[TOP_1]);
}
	if (rewardmap.count(TOP_2) > 0) {
		rewardmap[TOP_2] *= npeerBetValue;
		LogTrace2("print", "\n\n CBetMulity::TOP_2:%ld", rewardmap[TOP_2]);
}
	if (rewardmap.count(TOP_3) > 0) {
		rewardmap[TOP_3] *= npeerBetValue;
		LogTrace2("print", "\n\n CBetMulity::TOP_3:%ld", rewardmap[TOP_3]);
}
	if (rewardmap.count(TOP_4) > 0) {
		rewardmap[TOP_4] *= npeerBetValue;
		LogTrace2("print", "\n\n CBetMulity::TOP_4:%ld", rewardmap[TOP_4]);
}
	if (rewardmap.count(TOP_5) > 0) {
		rewardmap[TOP_5] *= npeerBetValue;
		LogTrace2("print", "\n\n CBetMulity::TOP_5:%ld", rewardmap[TOP_5]);}
	LogTrace2("print",
		"\n\n CBetMulity::npeerBetValue:%ld", npeerBetValue);


}
void CBetMulity::CreateRewardMap() {
	const std::vector<unsigned char>&luckly = vLuckyStar;
	std::vector<unsigned char>::iterator it = std::find(vLuckyStar.begin(), vLuckyStar.end(), lotto::gSplitFlag);
	if (it == vLuckyStar.end()) {
		LogTrace("CBetMulity lucky is error:%s", __FUNCTION__);
		return;
	}
	if (vRed.empty() || vBlue.empty()) {
		LogTrace("CBetMulity selectnum is error:%s", __FUNCTION__);
		return;
	}
	std::vector<unsigned char> preLucky(vLuckyStar.begin(), it);
	std::vector<unsigned char> additinualLucky(it + 1, vLuckyStar.end());
	sort(preLucky.begin(), preLucky.end());
	sort(additinualLucky.begin(), additinualLucky.end());
	int count = 0, addnum = 0;
	BOOST_FOREACH(const unsigned char& no,vRed) {
		if (find(preLucky.begin(), preLucky.end(), no) != preLucky.end()) {
			count++;
		}
	}
//	cout<<"CBetMulity vLuckyStar:"<<VectorToString(vLuckyStar)<<endl;
//	cout<<"CBetMulity preLucky:"<<VectorToString(preLucky)<<endl;
//	cout<<"CBetMulity additinualLucky:\n"<<VectorToString(additinualLucky)<<endl;
	BOOST_FOREACH(const unsigned char& no,vBlue) {
		if (find(additinualLucky.begin(), additinualLucky.end(), no) != additinualLucky.end()) {
			addnum++;
		}
	}

	rewardmap.clear();
	CalcRewardBet(count, addnum);
}

int64_t CBetMulity::GetMulit(int type) const {
	assert(type >= 0 && type < sizeof(multiplying) / sizeof(multiplying[0]));
	return multiplying[type];
}

int CBetMulity::getTotalBer() const {
	std::vector<unsigned char>::const_iterator it = find(vch.begin(), vch.end(), lotto::gSplitFlag);
	return MselectN(it - vch.begin(), READ_MAX_NICE_NO) * MselectN(vch.end() - (it + 1), BLUE_MAX_NICE_NO);
}

CBetMulity::~CBetMulity() {

}
bool CBetMulity::print() const {
	std::string tem = "";
	tem += tfm::format("\n\n CBetMulity::\nmultiplying[5] %f, %f, %f,%f,%f \n", multiplying[0], multiplying[1],
			multiplying[2], multiplying[3], multiplying[4]);
	tem += tfm::format("CBetMulity preluckenum:%s\n", VectorToString(vRed));
	tem += tfm::format("CBetMulity addinationanum:%s\n", VectorToString(vBlue));
	tem += tfm::format("%s", "bass class:\n");
	LogTrace2("print", "%s", tem);
	return CBet::print();
}

}