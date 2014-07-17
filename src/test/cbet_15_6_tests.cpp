#include "lotto/CLotto.h"
#include "core.h"
#include "base58.h"
#include "lotto/CBetData.h"
#include "lotto/CBet.h"
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include <openssl/rand.h>
#include "CBet156.h"

BOOST_AUTO_TEST_SUITE(cbet_15_6)
//get the lucky data
//eg:15-6
std::vector<unsigned char> getvchLucky(int totalsize, int luckysize) {
	std::vector<unsigned char> vch;
	unsigned char randch;
	Assert(totalsize >= luckysize);
	while (vch.size() < luckysize) {
		RAND_bytes(&randch, sizeof(randch));
		if (randch < (totalsize + 1) && std::find(vch.begin(), vch.end(), randch) == vch.end()) {
			vch.push_back(randch);
		}
	}

	Assert(vch.size() == luckysize);

	return vch;
}

//get the select data
std::vector<unsigned char> getvchSelect(std::vector<unsigned char> vchlucky, int luckysize, int selectsize,int totalsize) {
	std::vector<unsigned char> vch;
	unsigned char randch;
	Assert(selectsize >= luckysize);
	Assert((selectsize - luckysize) <= 15);
	while (vch.size() < (selectsize - luckysize)) {
		RAND_bytes((unsigned char*)&randch, sizeof(randch));
//		 srand(time(NULL));
//		randch = rand()%totalsize + 1;
		if (std::find(vchlucky.begin(), vchlucky.end(), randch) == vchlucky.end() && randch < (totalsize +1)
				&& std::find(vch.begin(), vch.end(), randch) == vch.end()) {
			vch.push_back(randch);
			//cout<<"enter:"<<(int)randch<<endl;
		}
		//RAND_cleanup();
	}
	int i = 0;
	while (i < luckysize) {
		vch.push_back(vchlucky[i]);
		i++;
	}

	Assert(vch.size() == selectsize);

	return vch;
}

void TestCbet15_6(std::vector<unsigned char> luckenum, int luckysize, int selectsize, int64_t money,int totalsize) {
	std::vector<unsigned char> betdata;
	uint160 address("asdfghjkl"); //the addr

	std::vector<unsigned char> selectnum;
	selectnum = getvchSelect(luckenum, luckysize, selectsize,totalsize);

	LogTrace("spark", "the selectnum:%s\nthe luckenum:%s\n", VectorToString(selectnum), VectorToString(luckenum));

	betdata.push_back(0); //the 15_6
	betdata.insert(betdata.end(), address.begin(), address.end());
	std::vector<unsigned char> copyvector(betdata.begin(), betdata.end());
	betdata.insert(betdata.end(), selectnum.begin(), selectnum.end());
	lotto::CBetData pLotto;
	pLotto << betdata;

	lotto::CBet_15_6 bet;
	bet << pLotto;

	bet << luckenum;
	bet << money; //the bet money

	int64_t ktop3, ktop2, ktop1;
	ktop1 = (luckysize == 6) ? (1) : (0);
	ktop2 = ((luckysize >= 5) && ((selectsize - luckysize) >= 1)) ?
			(bet.MselectN(luckysize, 5) * bet.MselectN(selectsize - luckysize, 1)) : (0);
	ktop3 = ((luckysize >= 4) && ((selectsize - luckysize) >= 2)) ?
			(bet.MselectN(luckysize, 4) * bet.MselectN(selectsize - luckysize, 2)) : (0);
	ktop3 +=
			((luckysize >= 3) && ((selectsize - luckysize) >= 3)) ?
					(bet.MselectN(luckysize, 3) * bet.MselectN(selectsize - luckysize, 3)) : (0);
	ktop3 +=
			((luckysize >= 2) && ((selectsize - luckysize) >= 4)) ?
					(bet.MselectN(luckysize, 2) * bet.MselectN(selectsize - luckysize, 4)) : (0);
	int64_t npeerBetValue = bet.nValue / bet.getTotalBer();
//	printf("\nnpeerBetValue:%lld bet.nValue:%lld bet.getTotalBer():%lld\n", npeerBetValue, bet.nValue, bet.getTotalBer());
	bet.CreateRewardMap();
	std::map<lotto::RWD_TYPE, int64_t> nReward = bet.GetBetRewardMap();
//	printf("\nluckysize:%d selectsize:%d money:%d npeerBetValue:%lld\n", luckysize, selectsize, money, npeerBetValue);
//	printf("\nktop1:%lld ktop2:%lld ktop3:%lld\n", ktop1, ktop2, ktop3);
//	printf("\nthe rewardmap size = %d\n", bet.rewardmap.size());
//	BOOST_FOREACH(const PAIRTYPE(lotto::RWD_TYPE, int64_t)& tep , bet.rewardmap)
//	{
//		printf("\nrewardmap RWD_TYPE = %d  rewardmap RWD_TYPE = %lld\n", tep.first, tep.second);
//	}

	BOOST_CHECK_EQUAL(bet.IsValid(), (npeerBetValue > 1000)); //test the bet money
	BOOST_CHECK_EQUAL(bet.isWinning(), (ktop3 || ktop2 || ktop1)); //test whether win
	//test the result,the most part
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], ktop3 * npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], ktop2 * npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], ktop1 * npeerBetValue);
}

void TestCbetMuliti(std::vector<unsigned char> luckenum,std::vector<unsigned char> luckenumadd, int luckysize,int addlucky, int selectsize,int nextselcsize, int64_t money,int totalsize) {
	std::vector<unsigned char> betdata;
	uint160 address("asdfghjkl"); //the addr

	std::vector<unsigned char> selectnum;
	std::vector<unsigned char> add;
	selectnum = getvchSelect(luckenum, luckysize, selectsize,totalsize);
	//LogTrace("spark", "luckenum:%s\n",  VectorToString(luckenumadd));
	add = getvchSelect(luckenumadd, addlucky, nextselcsize,10);
	luckenum.push_back(0xff);
	luckenum.insert(luckenum.end(),luckenumadd.begin(),luckenumadd.end());
	selectnum.push_back(0xff);
	selectnum.insert(selectnum.end(),add.begin(),add.end());
	LogTrace("spark", "the selectnum:%s\nthe luckenum:%s\n", VectorToString(selectnum), VectorToString(luckenum));

	betdata.push_back(1); //the 15_6
	betdata.insert(betdata.end(), address.begin(), address.end());
	std::vector<unsigned char> copyvector(betdata.begin(), betdata.end());
	betdata.insert(betdata.end(), selectnum.begin(), selectnum.end());
	lotto::CBetData pLotto;
	pLotto << betdata;

	lotto::CBetMulity bet;
	bet << pLotto;

	bet << luckenum;
	bet << money; //the bet money

	int64_t ktop3 =0, ktop2 = 0 , ktop1 = 0,ktop4 = 0,ktop5 = 0;
	ktop1 = (luckysize == 5 && addlucky == 2) ? (1) : (0);
	if((luckysize >= 5&& addlucky >=1))
	{
		cout<<"1"<<endl;
		ktop2 = ((nextselcsize - addlucky >= 1)) ?
			(bet.MselectN(addlucky, 1)*bet.MselectN(nextselcsize - addlucky, 1)) : (0);
	}
	if((luckysize >= 4&& (selectsize - luckysize >=1) && addlucky >=1)){
		cout<<"2"<<endl;
	ktop3 = ((nextselcsize - addlucky >= 1)) ?
			(bet.MselectN(luckysize, 4)*bet.MselectN(selectsize - luckysize, 1)*bet.MselectN(addlucky, 1)*bet.MselectN(nextselcsize - addlucky, 1)) : bet.MselectN(luckysize, 4)*bet.MselectN(selectsize - luckysize, 1)*(bet.MselectN(addlucky, 2));
	}
	cout<<"3"<<endl;
	ktop3 +=((luckysize >= 5 &&(nextselcsize - addlucky) >= 2)) ?
			(bet.MselectN(nextselcsize - addlucky, 2)) : (0);
	cout<<"4"<<endl;
	ktop4 =((luckysize >= 4) && ((selectsize - luckysize) >= 1)&& (nextselcsize - addlucky >=2)) ?
			(bet.MselectN(luckysize, 4)*bet.MselectN(selectsize - luckysize, 1)*bet.MselectN(nextselcsize - addlucky, 2))  : (0);

	if((luckysize >= 3&& (selectsize - luckysize >=2) && addlucky >=1)){
		cout<<"5"<<endl;
		ktop4 += (((nextselcsize - addlucky) >= 1)) ?
			(bet.MselectN(luckysize, 3)*bet.MselectN(selectsize - luckysize, 2)*bet.MselectN(addlucky, 1)*bet.MselectN(nextselcsize - addlucky, 1)) : bet.MselectN(luckysize, 3)*bet.MselectN(selectsize - luckysize, 2)*(bet.MselectN(addlucky, 2));
	}
	cout<<"6"<<endl;
	ktop5 = ((luckysize >= 3 && (selectsize - luckysize) >=2 &&(nextselcsize - addlucky) >= 2)) ?
			(bet.MselectN(luckysize, 3)*bet.MselectN(selectsize - luckysize, 2)*bet.MselectN(nextselcsize - addlucky, 2)) : (0);
	cout<<"7"<<endl;
	ktop5 +=((luckysize >= 2 && (selectsize - luckysize) >=3 )) ?
			(bet.MselectN(luckysize, 2)*bet.MselectN(selectsize - luckysize, 3)*bet.MselectN(nextselcsize, 2)) : (0);
	cout<<"8"<<endl;
	ktop5 +=((addlucky >= 2 && (selectsize - luckysize) >=5 )) ?
			(bet.MselectN(selectsize - luckysize, 5)) : (0);

	int64_t npeerBetValue = bet.nValue / bet.getTotalBer();

	bet.SplitSelectNum();
	bet.CreateRewardMap();
	std::map<lotto::RWD_TYPE, int64_t> nReward = bet.GetBetRewardMap();


	BOOST_CHECK_EQUAL(bet.IsValid(), (npeerBetValue > 1000)); //test the bet money
	BOOST_CHECK_EQUAL(bet.isWinning(), (ktop3 || ktop2 || ktop1||ktop5||ktop4)); //test whether win
	//test the result,the most part
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_5], ktop5 * npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_4], ktop4 * npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_3], ktop3 * npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_2], ktop2 * npeerBetValue);
	BOOST_CHECK_EQUAL(nReward[lotto::TOP_1], ktop1 * npeerBetValue);
}

BOOST_AUTO_TEST_CASE(cbet_15_6_test) {

	TestCbet15_6(getvchLucky(15, 6), 0, 6, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 0, 7, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 0, 8, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 0, 9, 5000000,15);

	TestCbet15_6(getvchLucky(15, 6), 1, 6, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 1, 7, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 1, 8, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 1, 9, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 1, 10, 5000000,15);

	TestCbet15_6(getvchLucky(15, 6), 2, 6, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 2, 7, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 2, 8, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 2, 9, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 2, 10, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 2, 11, 5000000,15);

	TestCbet15_6(getvchLucky(15, 6), 3, 6, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 3, 7, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 3, 8, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 3, 9, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 3, 10, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 3, 11, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 3, 12, 5000000,15);

	TestCbet15_6(getvchLucky(15, 6), 4, 6, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 4, 7, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 4, 8, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 4, 9, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 4, 10, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 4, 11, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 4, 12, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 4, 13, 5000000,15);

	TestCbet15_6(getvchLucky(15, 6), 5, 6, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 7, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 8, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 9, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 10, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 11, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 12, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 13, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 5, 14, 5000000,15);

	TestCbet15_6(getvchLucky(15, 6), 6, 6, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 7, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 8, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 9, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 10, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 11, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 12, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 13, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 14, 5000000,15);
	TestCbet15_6(getvchLucky(15, 6), 6, 15, 5000000,15);
}

BOOST_AUTO_TEST_CASE(cbet_mulit_test)
{
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,6,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,7,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,8,2,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),0,0,8,5,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,0,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,0,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,0,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,0,8,5,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,1,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,1,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,1,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),1,1,8,5,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,0,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,0,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,0,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,0,8,5,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,1,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,1,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,1,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),2,1,8,5,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,9,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,10,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,12,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,0,13,6,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,9,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,9,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,12,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,1,13,8,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,9,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,10,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,12,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,13,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,14,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),3,2,15,4,5000000,20);


	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,8,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,8,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,12,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,13,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,14,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,0,15,4,5000000,20);


	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,9,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,9,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,12,9,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,13,9,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,14,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,1,15,4,5000000,20);


	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,9,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,10,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,12,9,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,13,10,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,14,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,15,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,16,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),4,2,17,3,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,8,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,8,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,12,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,13,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,14,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,0,15,4,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,9,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,10,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,12,9,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,13,9,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,14,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,15,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,16,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,17,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,18,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,19,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,1,20,4,5000000,20);

	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,5,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,6,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,7,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,8,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,9,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,10,7,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,11,8,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,12,9,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,13,10,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,14,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,15,4,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,16,6,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,17,3,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,18,2,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,19,5,5000000,20);
	TestCbetMuliti(getvchLucky(20,5),getvchLucky(10, 2),5,2,20,4,5000000,20);

}
BOOST_AUTO_TEST_SUITE_END()
