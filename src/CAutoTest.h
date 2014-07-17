/*
 * CAutoTest.h
 *
 *  Created on: 2014Äê7ÔÂ5ÈÕ
 *      Author: ranger.shi
 */

#ifndef CAUTOTEST_H_
#define CAUTOTEST_H_
#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "net.h"
#include "netbase.h"
#include "util.h"
#include "wallet.h"
#include "walletdb.h"
#include "script.h"
#include <stdint.h>
#include <fstream>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "lotto/lotteryfile.h"
#include "CSyncDataDb.h"
#include <boost/algorithm/string.hpp>
#include "miner.h"
#include "CLotto.h"
#include "CBetData.h"
using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;
namespace test {

class CAutoTest {
	string revAddr;
	int cycles;
	int gaptimems;
	std::string privateKey;
	std::string lottoPrivateKey;
	map<int,boost::shared_ptr<boost::thread> > threadmap;
	int RandInt(int max, int min)const {
		int randch;
		RAND_bytes((unsigned char*) &randch, sizeof(randch));
		randch %= max;
		if (randch < min)
			randch += min;
		return randch;
	}

	int SelectSize(int total,int min)const
	{
		while(true)
		{
			 int randch = GetRandInt(total);
			 if(randch > min) return randch;
		}

	}
	int SelectSize(int total)const
	{
		unsigned char randch;
		while (true ) {
			RAND_bytes((unsigned char*)&randch, sizeof(randch));
			if (randch <= total)
				return (int)randch;
		}
		return 0;
	}
	string getvchSelect(int selectsize,int totalsize)const {
		std::vector<unsigned char> vch;
		unsigned char randch;
		string ret = "";
		while (vch.size() < selectsize ) {
			RAND_bytes((unsigned char*)&randch, sizeof(randch));
			if (randch < (totalsize +1)&&randch>=1&&std::find(vch.begin(), vch.end(), randch) == vch.end()) {
				vch.push_back(randch);
				ret += tfm::format("%02x",randch);
			}
		}
		Assert(vch.size() == selectsize);

		return ret;
	}



public:
    void startThread(int type);
	string getAddress()const;
	double getBanlance()const;
	void SendRandBet(int cycles, int gaptimems,string reciveaddr);
	void SendRandTransation(int cycles, int gaptimems,string reciveaddr);
	bool Inital(std::string& revAddir,int cycles, int gaptimems, string &privateKey, string &lottoPrivateKey);
	int getBestBlockHigh()const;
	void CheckAndSendLottoKey(int cycles, int gaptimems, string privateKey, string lottoPrivateKey)const;
	void setgengerate(int cycles, int gaptimems) const;
	void stopThread();
	CAutoTest(){
     cycles =1;
	 gaptimems = 1000;
	 privateKey = "123456";
	 lottoPrivateKey = "123456789";
	};
	~CAutoTest(){};
};

} /* namespace test */

#endif /* CAUTOTEST_H_ */
