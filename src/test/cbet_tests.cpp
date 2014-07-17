#include "lotto/CLotto.h"
#include "core.h"
#include "base58.h"
#include "lotto/CBetData.h"
#include "lotto/CBet.h"
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>
#include "lotteryfile.h"

BOOST_AUTO_TEST_SUITE(cbet_tests)
void sort1(std::vector<unsigned char>&vTemp)
{
		std::vector<unsigned char>::iterator it = std::find(vTemp.begin(), vTemp.end(), lotto::gSplitFlag);
		if(it == vTemp.end()) return;
		std::vector<unsigned char> ftep(vTemp.begin(),it);
		std::vector<unsigned char> step(it+1,vTemp.end());
		sort(ftep.begin(),ftep.end());
		sort(step.begin(),step.end());
		vTemp.clear();
		vTemp.assign(ftep.begin(),ftep.end());
		vTemp.push_back(lotto::gSplitFlag);
		vTemp.insert(vTemp.end(),step.begin(),step.end());
}
BOOST_AUTO_TEST_CASE(cbet_data) {

	lotto::CBetData pLotto1;
	pLotto1.push_back(0);
	BOOST_CHECK_EQUAL(pLotto1.GetType(), 0);

	pLotto1.clear();
	pLotto1.push_back(1);
	BOOST_CHECK_EQUAL(pLotto1.GetType(), 1);

	pLotto1.clear();
	pLotto1.push_back(2);
	BOOST_CHECK_THROW(pLotto1.GetType(), runtime_error);

	pLotto1.clear();
	pLotto1.push_back(8);
	BOOST_CHECK_THROW(pLotto1.GetType(), runtime_error);

	std::vector<unsigned char> betdata;
	betdata.push_back(0);
	uint160 address("dfsdfdsf");
	betdata.insert(betdata.end(), address.begin(), address.end());
	lotto::CBetData pLotto;
	pLotto << betdata;
	BOOST_CHECK(pLotto.GetAddr() == address);

	char* str = "0498946";
	pLotto.assign(str, str + sizeof(str));
	BOOST_CHECK(pLotto.GetAddr() == uint160());

	str = "954560000000000000000000000000000000000000000000000000";
	std::vector<unsigned char> nvector = ParseHex(str);
	pLotto.insert(pLotto.begin(), nvector.begin(), nvector.end());
	BOOST_CHECK_THROW(pLotto.GetAddr(), runtime_error);

	str = "005040302010f0e0d0c0b0a09080706050403";
	nvector.clear();
	pLotto.clear();
	nvector = ParseHex(str);
	pLotto.insert(pLotto.begin(), nvector.begin(), nvector.end());
	BOOST_CHECK(pLotto.GetSelectV().size() == 0);

	str = "005040302010f0e0d0c0b0a090807060504030201";
	nvector = ParseHex(str);
	nvector.clear();
	pLotto.clear();
	pLotto.insert(pLotto.begin(), nvector.begin(), nvector.end());
	BOOST_CHECK(pLotto.GetSelectV().size() == 0);

	str = "0005040302010f0e0d0c0b0a090807060504030201050607";
	nvector.clear();
	pLotto.clear();
	nvector = ParseHex(str);
	pLotto.insert(pLotto.begin(), nvector.begin(), nvector.end());
	BOOST_CHECK(pLotto.GetSelectV().size() == 3);

	str = "905040302010f0e0d0c0b0a090807060504030201050607";
	nvector = ParseHex(str);
	pLotto.insert(pLotto.begin(), nvector.begin(), nvector.end());
	BOOST_CHECK_THROW(pLotto.GetSelectV(), runtime_error);

}
BOOST_AUTO_TEST_CASE(cbet_test) {

	lotto::CBet_15_6 bet;
	string str = "030405060709";
	std::vector<unsigned char> selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type =(lotto::BET_TYPE)0 ;
	bet.nValue = 5000000000;
	BOOST_CHECK(bet.IsValid());

	str = "05040703060909";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type = (lotto::BET_TYPE) 0;
	bet.nValue = 50000000;
	BOOST_CHECK(!bet.IsValid());

	str = "05040703060911";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type = (lotto::BET_TYPE) 0;
	bet.nValue = 50000000;
	BOOST_CHECK(!bet.IsValid());

	str = "0304050607090f";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type = (lotto::BET_TYPE) 0;
	bet.nValue = 50000000;
	BOOST_CHECK(bet.IsValid());

	str = "00050407030609";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type = (lotto::BET_TYPE) 0;
	bet.nValue = 50000000;
	BOOST_CHECK(!bet.IsValid());

	str = "05040703060910";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type = (lotto::BET_TYPE) 0;
	bet.nValue = 50000000;
	BOOST_CHECK(!bet.IsValid());

	str = "05040703060909";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type = (lotto::BET_TYPE) 0;
	bet.nValue = 50000000;
	BOOST_CHECK(!bet.IsValid());

	str = "05040703060909";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.type = (lotto::BET_TYPE) 0;
	bet.nValue = 10;
	BOOST_CHECK(!bet.IsValid());

	BOOST_CHECK(!bet.isWinning());
//	BOOST_CHECK_THROW(bet.CreateRewardMap(), runtime_error);
//	BOOST_CHECK_THROW(bet.getTotalBer(), runtime_error);
//	BOOST_CHECK(bet.GetMulit(0) == 0.0);
}
BOOST_AUTO_TEST_CASE(cbet_test15_61) {
	lotto::CBetMulity bet;
	string lucky = "0406070c0fff0308";
	std::vector<unsigned char> luckenum = ParseHex(lucky);
	bet.vLuckyStar.insert(bet.vLuckyStar.begin(), luckenum.begin(), luckenum.end());
	bet.nValue = 5000000000;
	bet.type = lotto::TYPE_MULILT;
	string str = "0006040d 130f110e 08121409 ff010407050008";
	std::vector<unsigned char> selectnum = ParseHex(str);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	BOOST_CHECK(!bet.IsValid());

	lotto::CBet_15_6 bet1;
	string str1 = "050407030609";
	std::vector<unsigned char> selectnum1 = ParseHex(str1);
	bet1.vch.insert(bet1.vch.begin(), selectnum1.begin(), selectnum1.end());
	bet1.type =(lotto::BET_TYPE)0 ;
	bet1.nValue = 5000000000;
	BOOST_CHECK(!bet1.IsValid());
//	bet.CreateRewardMap();
//	cout << "rewardmap= " << bet.rewardmap.size() << endl;
}
BOOST_AUTO_TEST_CASE(cbet_test15_6) {

	lotto::CBet_15_6 bet;
	string lucky = "0c0a05030901";
	std::vector<unsigned char> luckenum = ParseHex(lucky);
	bet.vLuckyStar.insert(bet.vLuckyStar.begin(), luckenum.begin(), luckenum.end());
	bet.nValue = 5000000;
	bet.type = lotto::TYPE_15_6;
	string str = "050407030608";
	std::vector<unsigned char> selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(bet.IsValid());

	str = "0504";
	selectnum.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(!bet.IsValid());

	str = "0102030405060708090a0b0c0e0f01";
	selectnum.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(!bet.IsValid());

	bet.type = lotto::TYPE_MULILT;
	BOOST_CHECK_THROW(bet.IsValid(), runtime_error);

	bet.type = lotto::typenull;
	BOOST_CHECK_THROW(bet.IsValid(), runtime_error);

	bet.type = lotto::TYPE_15_6;
	str = "010203040506";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK_EQUAL(bet.getTotalBer(), 1);

	str = "01020304050607";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK_EQUAL(bet.getTotalBer(), 7);

	str = "0102030405060708";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK_EQUAL(bet.getTotalBer(), 28);

	str = "0102030405060708";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.CreateRewardMap();
	BOOST_CHECK(!bet.isWinning());

	str = "04020607080b0d0e0f";
	selectnum.clear();
	bet.vch.clear();
	bet.rewardmap.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.CreateRewardMap();
	BOOST_CHECK(!bet.isWinning());

	selectnum.clear();
	bet.vch.clear();
	bet.rewardmap.clear();
	std::map<lotto::RWD_TYPE, int64_t> nReward;

	//---------------------	CreateRewardMap() and GetBetRewardMap() function--------------//
	//**************6***********************************//

	str = "020406070811";
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue =  5000000;
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a05030901";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue =  5000000;
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1],  5000000);

	str = "0c0a05030904";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000000;
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 5000000);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	//**************7***********************************//
	str = "0c0a0503090104";   //// 6 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000000;
	int64_t npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 6);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a0503090402";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 5);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 2);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a0503060402";     //// 4 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 3);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a0507060402";     //// 3 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a0807060402";     //// 2 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c08070604020b";     //// 1 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "08070604020b0d";     //// 0 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	//**************8***********************************//
	str = "0c0a050309010402";   /// 5
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 15);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 12);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a050309040206";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 15);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 3);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a050306040207";     //// 4 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 6);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a050706040208";     //// 3 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a08070604020b";     //// 2 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c08070604020b0d";     //// 1 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "08070604020b0e";     //// 0 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	//**************9***********************************//
	str = "0c0a05030901040206";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 45);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 18);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a05030904020607";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 30);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 4);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a05030604020708";     //// 4 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 10);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a0507060402080b";     //// 3 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3],0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a08070604020b0d";     //// 2 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c08070604020b0d0e";     //// 1 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "08070604020b0e0f";     //// 0 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	//**************10***********************************//
	str = "0c0a0503090104020607";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 90);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 24);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a0503090402060708";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 50);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 5);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a050306040207080b";     //// 4 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 15);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a0507060402080b0d";     //// 3 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a08070604020b0d0e";     //// 2 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c08070604020b0d0e0f";     //// 1 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	//**************11***********************************//
	str = "0c0a050309010402060708";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 150);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 30);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a05030904020607080b";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 75);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 6);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a050306040207080b0d";     //// 4 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 21);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a0507060402080b0d0e";     //// 3 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a08070604020b0d0e0f";     //// 2 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	//**************12***********************************//
	str = "0c0a0503090104020607080b";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 225);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 36);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a05030904020607080b0d";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 105);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 7);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a050306040207080b0d0e";     //// 4 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 28);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a0507060402080b0d0e0f";     //// 3 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	//**************13***********************************//
	str = "0c0a0503090104020607080b0d";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 315);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 42);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a05030904020607080b0d0e";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 140);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 8);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);

	str = "0c0a050306040207080b0d0e0f";     //// 4 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 36);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	//**************14***********************************//
	str = "0c0a0503090104020607080b0d0e";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 420);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 48);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

	str = "0c0a05030904020607080b0d0e0f";     //// 5 lucky
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 180);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 9);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	//**************15***********************************//
	str = "0c0a0503090104020607080b0d0e0f";
	selectnum.clear();
	selectnum = ParseHex(str);
	bet.vch.clear();
	bet.rewardmap.clear();
	sort(selectnum.begin(),selectnum.end());
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.nValue = 5000;
	npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 540);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 54);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);

}

BOOST_AUTO_TEST_CASE(cbet_mulityclass) {
	lotto::CBetMulity bet;
	string lucky = "0102030405ff0102";
	std::vector<unsigned char> luckenum = ParseHex(lucky);
	bet.vLuckyStar.insert(bet.vLuckyStar.begin(), luckenum.begin(), luckenum.end());
	bet.nValue = 50000000;
	bet.type = lotto::TYPE_MULILT;
	string str = "0504070306ff0102";
	std::vector<unsigned char> selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bool ret = bet.IsValid();
	BOOST_CHECK(bet.IsValid());

	str = "0504";
	selectnum.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(!bet.IsValid());

	str = "0102030405060708090a0b0c0e0f161718192021ff0102";
	selectnum.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(!bet.IsValid());

	str = "05040703060102";
	selectnum.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(!bet.IsValid());

	str = "050407030630ff0102";
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(!bet.IsValid());

	str = "050407030630ff010a";
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK(!bet.IsValid());

	bet.type = lotto::TYPE_15_6;
	//BOOST_CHECK(!bet.IsValid());
	//BOOST_CHECK_THROW(bet.IsValid(), runtime_error);

	bet.type = lotto::typenull;
	//BOOST_CHECK_THROW(bet.IsValid(), runtime_error);

	bet.type = lotto::TYPE_MULILT;
	str = "0102030405ff0102";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK_EQUAL(bet.getTotalBer(), 1);

	str = "010203040506ff0102";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK_EQUAL(bet.getTotalBer(), 6);

	str = "0102030405060708ff0102";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	BOOST_CHECK_EQUAL(bet.getTotalBer(), 56);

	str = "0102030405ff0102";
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	bet.CreateRewardMap();
	BOOST_CHECK(bet.isWinning());

	str = "0607080905ff0304";
	selectnum.clear();
	bet.vch.clear();
	bet.rewardmap.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	bet.CreateRewardMap();
	BOOST_CHECK(!bet.isWinning());

	selectnum.clear();
	bet.vch.clear();
	bet.rewardmap.clear();
	std::map<lotto::RWD_TYPE, int64_t> nReward;
	//**************5+/2/3/4/5/6/7/8/9/10***********************************//

	str = "0102030405ff0102";    //// 5 + 2
	selectnum.clear();
	bet.vch.clear();
	selectnum = ParseHex(str);
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 50000000);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "0102030405ff0103";  /// 5 + 1
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	int64_t npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "0102030405ff0403";  /// 5 + 0
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "0102030405ff010203";  /// 5 + 2/5+1
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 2);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "0102030405ff010403";  /// 5 + 2/5+1
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 2);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "010203040506ff0102";
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue*5);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "010203040506ff0103";
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 5);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "010203040506ff010203";
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 7);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 10);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "010203040506ff010403";
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 2);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 11);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], npeerBetValue * 5);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "010203040506ff050403";
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 3);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], npeerBetValue * 15);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], 0);

	str = "01020304050607ff01020304";
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 14);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 51);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], npeerBetValue * 50);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], npeerBetValue * 10);

	str = "01020304050607ff01050304";
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], 0);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 3);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 33);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], npeerBetValue * 60);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], npeerBetValue * 30);

	str = "0102030405060708090aff0102"; // 5+2;4+1;3+1;2
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue * 1);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 25);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 100);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], npeerBetValue * 100);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], npeerBetValue * 1);

	str = "0102030405060708090aff01020304"; // 5+2;4+1;3+1;2;4;3
	selectnum = ParseHex(str);
	bet.vch.clear();
	sort1(selectnum);
	bet.vch.insert(bet.vch.begin(), selectnum.begin(), selectnum.end());
	bet.SplitSelectNum();
	npeerBetValue = bet.nValue / bet.getTotalBer();
	bet.CreateRewardMap();
	nReward = bet.GetBetRewardMap();
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], npeerBetValue * 1);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], npeerBetValue * 29);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], npeerBetValue * 201);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], npeerBetValue * 525);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], npeerBetValue * 501);

}
BOOST_AUTO_TEST_CASE(bet_statr) {
	string strhash = "0000000000000000e067a478024addfecdc93628978aa52d91fabd4929298a50"; // 0c0a05030901
	uint256 uHash(strhash);
	int64_t iRewardPool = 789494654564;
	std::vector<CTxOut> txOutV;

	CKey key[4];
	for (int i = 0; i < 4; i++) {
		key[i].MakeNewKey(i % 2);
	}
	string str = "0504070309";
	vector<unsigned char> betnum;
	txOutV.clear();
	CTxOut vout1, vout2;

	char buffer[5] = { 0 };
	sprintf((char*) buffer, "0%d", 6);
	str += buffer;
	betnum = ParseHex(str);
	cout << "selectnum count: str:" << 6 << str << endl;

	uint160 address("dfsdfdsf");
	std::vector<unsigned char> nbetstr;

	/// type
	nbetstr.push_back(0);
	/// address
	nbetstr.insert(nbetstr.end(), address.begin(), address.end());
	std::vector<unsigned char> ncopystack(nbetstr.begin(), nbetstr.end());
	//// the bet number
	nbetstr.insert(nbetstr.end(), betnum.begin(), betnum.end());

	vout1.nValue = 20 * CENT;
	vout1.scriptPubKey << OP_LOTTO;
	vout1.scriptPubKey << nbetstr;
	BOOST_TEST_MESSAGE("vout1.scriptPubKey:" + VectorToString(vout1.scriptPubKey));
	BOOST_TEST_MESSAGE("nbetstr:" + VectorToString(nbetstr));
	BOOST_TEST_MESSAGE("betnum:" + VectorToString(betnum));

	txOutV.push_back(vout1);

	lotto::CLotto plotoo(uHash, iRewardPool, txOutV);
	std::vector<CTxOut> retTxOut;
	int64_t retRewardPool = 0;

	plotoo.GetReWardOutput(retTxOut, retRewardPool);

	int64_t nValue = vout1.nValue *0;
	int64_t nCheckValue = iRewardPool - nValue;

	BOOST_FOREACH(const CTxOut& v,retTxOut) {
		BOOST_CHECK_EQUAL(nValue, v.nValue);
		txnouttype type;
		vector<CTxDestination> addresses;
		int nRequired;
		ExtractDestinations(v.scriptPubKey, type, addresses, nRequired);
		string sourceaddress = CBitcoinAddress(CKeyID(address)).ToString();
		string rewardaddress = CBitcoinAddress(addresses[0]).ToString();
		BOOST_CHECK(sourceaddress == rewardaddress);
	}

	BOOST_CHECK_EQUAL(nCheckValue, iRewardPool - retRewardPool);

	txOutV.clear();
	retTxOut.clear();
	retRewardPool = 0;
	std::vector<unsigned char> nbetstr1(ncopystack.begin(), ncopystack.end());
	string str1 = "05040703090608";
	betnum = ParseHex(str1);
	nbetstr1.insert(nbetstr1.end(), betnum.begin(), betnum.end());
	vout2.nValue = 50 * CENT;

	vout2.scriptPubKey << key[1].GetPubKey() << OP_CHECKSIG << OP_LOTTO << nbetstr1;
	txOutV.push_back(vout2);
	lotto::CLotto plotoo1(uHash, iRewardPool, txOutV);
	plotoo1.GetReWardOutput(retTxOut, retRewardPool);

	/// compute the reward
	nValue = ((vout2.nValue / 7) * 0) * 2;
	nCheckValue = iRewardPool - nValue;
	BOOST_FOREACH(const CTxOut& v,retTxOut) {
		BOOST_CHECK_EQUAL(nValue, v.nValue);
		txnouttype type;
		vector<CTxDestination> addresses;
		int nRequired;
		ExtractDestinations(v.scriptPubKey, type, addresses, nRequired);
		string sourceaddress = CBitcoinAddress(CKeyID(address)).ToString();
		string rewardaddress = CBitcoinAddress(addresses[0]).ToString();
		BOOST_CHECK(sourceaddress == rewardaddress);
	}

	BOOST_CHECK_EQUAL(nCheckValue, iRewardPool - retRewardPool);

	txOutV.clear();
	retTxOut.clear();
	retRewardPool = 0;
	CTxOut vout3;
	std::vector<unsigned char> nbetstr2(ncopystack.begin(), ncopystack.end());
	str1 = "05040703090102";
	betnum = ParseHex(str1);
	nbetstr2.insert(nbetstr2.end(), betnum.begin(), betnum.end());
	vout3.nValue = 50 * CENT;

	vout3.scriptPubKey << key[1].GetPubKey() << OP_CHECKSIG << OP_LOTTO << nbetstr2;
	txOutV.push_back(vout3);
	lotto::CLotto plotoo2(uHash, iRewardPool, txOutV);
	plotoo2.GetReWardOutput(retTxOut, retRewardPool);

	/// compute the reward
	nValue = ((vout3.nValue / 7) * 3) * 2;
	nCheckValue = iRewardPool - nValue;
	BOOST_FOREACH(const CTxOut& v,retTxOut) {
		BOOST_CHECK_EQUAL(nValue, v.nValue);
		txnouttype type;
		vector<CTxDestination> addresses;
		int nRequired;
		ExtractDestinations(v.scriptPubKey, type, addresses, nRequired);
		string sourceaddress = CBitcoinAddress(CKeyID(address)).ToString();
		string rewardaddress = CBitcoinAddress(addresses[0]).ToString();
		BOOST_CHECK(sourceaddress == rewardaddress);
	}

	BOOST_CHECK_EQUAL(nCheckValue, iRewardPool - retRewardPool);

	iRewardPool = 277200000;
	txOutV.clear();
	retTxOut.clear();
	retRewardPool = 0;
	CTxOut vout4;
	std::vector<unsigned char> nbetstr3(ncopystack.begin(), ncopystack.end());
	str1 = "0c0a0503090104020607080b";
	betnum = ParseHex(str1);
	nbetstr3.insert(nbetstr3.end(), betnum.begin(), betnum.end());
	vout4.nValue = 2772000;

	vout4.scriptPubKey << key[1].GetPubKey() << OP_CHECKSIG << OP_LOTTO << nbetstr3;
	txOutV.push_back(vout4);
	lotto::CLotto plotoo3(uHash, iRewardPool, txOutV);
	plotoo3.GetReWardOutput(retTxOut, retRewardPool);

	// compute the reward 3000 194040000 41580000 22176000
	double mu1 = 2;
	double mu2 = 17;
	double mu3 = 3000;
	nValue = ((vout4.nValue / 924) * 850) * mu1 + ((vout4.nValue / 924) * 36) * mu2 + ((vout4.nValue / 924) * 1) * mu3;
	nCheckValue = iRewardPool - nValue;
	BOOST_FOREACH(const CTxOut& v,retTxOut) {
		//BOOST_CHECK_EQUAL(nValue, v.nValue);
		txnouttype type;
		vector<CTxDestination> addresses;
		int nRequired;
		ExtractDestinations(v.scriptPubKey, type, addresses, nRequired);
		string sourceaddress = CBitcoinAddress(CKeyID(address)).ToString();
		string rewardaddress = CBitcoinAddress(addresses[0]).ToString();
		BOOST_CHECK(sourceaddress == rewardaddress);
	}

	//BOOST_CHECK_EQUAL(nCheckValue, iRewardPool - retRewardPool);

}

BOOST_AUTO_TEST_CASE(bet_mulity) {
	/// 0c0a050309+0107
	string strhash = "0000000000000000e067a478024addfecdc93628978aa52d91fabd4929298a50"; // 0c0a05030901
	uint256 uHash(strhash);
	int64_t iRewardPool = 789494654564;
	std::vector<CTxOut> txOutV;

	CKey key[4];
	for (int i = 0; i < 4; i++) {
		key[i].MakeNewKey(i % 2);
	}
	string str = "0504070309ff0801";
	vector<unsigned char> betnum;
	txOutV.clear();
	CTxOut vout1, vout2;
	int i = 6;
	char buffer[5] = { 0 };
	sprintf((char*) buffer, "0%d", i);
	str += buffer;
	betnum = ParseHex(str);
	cout << "selectnum count: str:" << i << str << endl;

	uint160 address("dfsdfdsf");
	std::vector<unsigned char> nbetstr;

	/// type
	nbetstr.push_back(1);
	/// address
	nbetstr.insert(nbetstr.end(), address.begin(), address.end());
	std::vector<unsigned char> ncopystack(nbetstr.begin(), nbetstr.end());
	//// the bet number
	nbetstr.insert(nbetstr.end(), betnum.begin(), betnum.end());

	vout1.nValue = 20 * CENT;
	vout1.scriptPubKey << OP_LOTTO;
	vout1.scriptPubKey << nbetstr;
	BOOST_TEST_MESSAGE("vout1.scriptPubKey:" + VectorToString(vout1.scriptPubKey));
	BOOST_TEST_MESSAGE("nbetstr:" + VectorToString(nbetstr));
	BOOST_TEST_MESSAGE("betnum:" + VectorToString(betnum));

	txOutV.push_back(vout1);

	lotto::CLotto plotoo(uHash, iRewardPool, txOutV);
	std::vector<CTxOut> retTxOut;
	int64_t retRewardPool = 0;

	plotoo.GetReWardOutput(retTxOut, retRewardPool);

	int64_t nValue = (vout1.nValue / 3) * 332.805464 * 2 + (vout1.nValue / 3) * 15;
	int64_t nCheckValue = iRewardPool - nValue;

	BOOST_FOREACH(const CTxOut& v,retTxOut) {
		BOOST_CHECK_EQUAL(nValue, v.nValue);
		txnouttype type;
		vector<CTxDestination> addresses;
		int nRequired;
		ExtractDestinations(v.scriptPubKey, type, addresses, nRequired);
		string sourceaddress = CBitcoinAddress(CKeyID(address)).ToString();
		string rewardaddress = CBitcoinAddress(addresses[0]).ToString();
		BOOST_CHECK(sourceaddress == rewardaddress);
	}
	BOOST_CHECK_EQUAL(nCheckValue, iRewardPool - retRewardPool);

}
BOOST_AUTO_TEST_CASE(createlottofile)
{
	CLotData tem("lottofile.dat");
	tem.CreateClottoFile("testKeyfile.dat",1024);
}
BOOST_AUTO_TEST_CASE(bet_lottery)
{
	CLottoFileKey key2;
	ReadObjFromFile("testKeyfile.dat",2, key2);

	std::vector<CTxOut> txOutV;
	CKey key[4];
	for (int i = 0; i < 4; i++) {
		key[i].MakeNewKey(i % 2);
	}
	string str = "050407030906";
	vector<unsigned char> betnum;
	CTxOut vout1, vout2;

	betnum = ParseHex(str);
	uint160 address("dfsdfdsf");
	std::vector<unsigned char> nbetstr;

	/// type
	nbetstr.push_back(0);
	/// address
	nbetstr.insert(nbetstr.end(), address.begin(), address.end());
	std::vector<unsigned char> ncopystack(nbetstr.begin(), nbetstr.end());
	//// the bet number
	nbetstr.insert(nbetstr.end(), betnum.begin(), betnum.end());

	vout1.nValue = 20 * CENT;
	vout1.scriptPubKey << OP_LOTTO;
	vout1.scriptPubKey << nbetstr;
	txOutV.push_back(vout1);

	int64_t iRewardPool = 200000000;
	uint256 bithash = uint256(1);
	lotto::CLotto plotoo(bithash,key2.getIndex(),key2.getKey()[0], iRewardPool, txOutV);
	std::vector<CTxOut> retTxOut;
	int64_t remind = 0;
	plotoo.GetReWardOutput(retTxOut,remind);

	int64_t rewardValue = iRewardPool*0.08;
	BOOST_FOREACH(const CTxOut& v,retTxOut) {
		BOOST_CHECK_EQUAL(rewardValue, v.nValue);
		txnouttype type;
		vector<CTxDestination> addresses;
		int nRequired;
		ExtractDestinations(v.scriptPubKey, type, addresses, nRequired);
		string sourceaddress = CBitcoinAddress(CKeyID(address)).ToString();
		string rewardaddress = CBitcoinAddress(addresses[0]).ToString();
		BOOST_CHECK(sourceaddress == rewardaddress);
	}

	BOOST_CHECK_EQUAL(iRewardPool- rewardValue, iRewardPool - remind);
}
BOOST_AUTO_TEST_SUITE_END()
