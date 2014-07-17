#include "rpcserver.h"
#include "rpcclient.h"

#include "base58.h"
#include "wallet.h"
#include "CDebug.h"
#include <boost/algorithm/string.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace json_spirit;

extern Array createArgs(int nRequired, const char* address1=NULL, const char* address2=NULL);
extern Value CallRPC(string args);

extern CWallet* pwalletMain;

BOOST_AUTO_TEST_SUITE(rpc_wallet_tests)

BOOST_AUTO_TEST_CASE(rpc_addmultisig)
{
    LOCK(pwalletMain->cs_wallet);

    rpcfn_type addmultisig = tableRPC["addmultisigaddress"]->actor;

    // old, 65-byte-long:
    const char address1Hex[] = "0434e3e09f49ea168c5bbf53f877ff4206923858aab7c7e1df25bc263978107c95e35065a27ef6f1b27222db0ec97e0e895eaca603d3ee0d4c060ce3d8a00286c8";
    // new, compressed:
    const char address2Hex[] = "0388c2037017c62240b6b72ac1a2a5f94da790596ebd06177c8572752922165cb4";

    Value v;
    CBitcoinAddress address;
    BOOST_CHECK_NO_THROW(v = addmultisig(createArgs(1, address1Hex), false));
    address.SetString(v.get_str());
    BOOST_CHECK(address.IsValid() && address.IsScript());

    BOOST_CHECK_NO_THROW(v = addmultisig(createArgs(1, address1Hex, address2Hex), false));
    address.SetString(v.get_str());
    BOOST_CHECK(address.IsValid() && address.IsScript());

    BOOST_CHECK_NO_THROW(v = addmultisig(createArgs(2, address1Hex, address2Hex), false));
    address.SetString(v.get_str());
    BOOST_CHECK(address.IsValid() && address.IsScript());

    BOOST_CHECK_THROW(addmultisig(createArgs(0), false), runtime_error);
    BOOST_CHECK_THROW(addmultisig(createArgs(1), false), runtime_error);
    BOOST_CHECK_THROW(addmultisig(createArgs(2, address1Hex), false), runtime_error);

    BOOST_CHECK_THROW(addmultisig(createArgs(1, ""), false), runtime_error);
    BOOST_CHECK_THROW(addmultisig(createArgs(1, "NotAValidPubkey"), false), runtime_error);

    string short1(address1Hex, address1Hex+sizeof(address1Hex)-2); // last byte missing
    BOOST_CHECK_THROW(addmultisig(createArgs(2, short1.c_str()), false), runtime_error);

    string short2(address1Hex+1, address1Hex+sizeof(address1Hex)); // first byte missing
    BOOST_CHECK_THROW(addmultisig(createArgs(2, short2.c_str()), false), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_wallet)
{
    // Test RPC calls for various wallet statistics
    Value r;

    LOCK(pwalletMain->cs_wallet);

    BOOST_CHECK_NO_THROW(CallRPC("listunspent"));
    BOOST_CHECK_THROW(CallRPC("listunspent string"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("listunspent 0 string"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("listunspent 0 1 not_array"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("listunspent 0 1 [] extra"), runtime_error);
    BOOST_CHECK_NO_THROW(r=CallRPC("listunspent 0 1 []"));
    BOOST_CHECK(r.get_array().empty());

    BOOST_CHECK_NO_THROW(CallRPC("listreceivedbyaddress"));
    BOOST_CHECK_NO_THROW(CallRPC("listreceivedbyaddress 0"));
    BOOST_CHECK_THROW(CallRPC("listreceivedbyaddress not_int"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("listreceivedbyaddress 0 not_bool"), runtime_error);
    BOOST_CHECK_NO_THROW(CallRPC("listreceivedbyaddress 0 true"));
    BOOST_CHECK_THROW(CallRPC("listreceivedbyaddress 0 true extra"), runtime_error);

    BOOST_CHECK_NO_THROW(CallRPC("listreceivedbyaccount"));
    BOOST_CHECK_NO_THROW(CallRPC("listreceivedbyaccount 0"));
    BOOST_CHECK_THROW(CallRPC("listreceivedbyaccount not_int"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("listreceivedbyaccount 0 not_bool"), runtime_error);
    BOOST_CHECK_NO_THROW(CallRPC("listreceivedbyaccount 0 true"));
    BOOST_CHECK_THROW(CallRPC("listreceivedbyaccount 0 true extra"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_lotto_bet)
{
    Value r;
//  LOCK(pwalletMain->cs_wallet);
    r = CallRPC("getbalance ");
    double nbalance = r.get_real();
    if(nbalance < 5)
    {

		CallRPC("setgenerate true 12");

    }
    r = CallRPC("getbalance ");
    nbalance = r.get_real();
    LogTrace2("bet_test","\nbalance:%s", nbalance);
     r= CallRPC("getnewaddress ");

 	CReserveKey keyReward(pwalletMain);
 	CPubKey rewardPubKey;
 	bool ret;
 	ret = keyReward.GetReservedKey(rewardPubKey);
 	assert(ret); // should never fail, as we just unlocked
    string address =CBitcoinAddress(rewardPubKey.GetID()).ToString();

    LogTrace2("bet_test","\nrecvie reward address:%s", address);
    r = CallRPC("getbalance ");
    nbalance = r.get_real();
    BOOST_CHECK_NO_THROW(CallRPC("sendlottobet \"\" n36sSjfPJMnroJy3BBHkus1TbvsAykiFCr 5000 0 010203040506 "+address));
    CallRPC("setgenerate true 23");
    cout<<endl;
    r = CallRPC("getbalance ");
    nbalance = r.get_real();

    r = CallRPC("getacountbyaddress "+address);
    int64_t reviveMoney = r.get_uint64();
    int64_t chekValue = ((int64_t)(5000*3.77613*0.01 *100000000));
    BOOST_CHECK_EQUAL(reviveMoney, chekValue);

    //// height 25 13
	CBlockIndex * pindexPrev = chainActive[25];
	CBlock curTipBlcok;
  	CDiskBlockPos curTipPos(pindexPrev->nFile, pindexPrev->nDataPos);
  	if(!ReadBlockFromDisk(curTipBlcok, curTipPos))
  		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
  	int64_t nPool = curTipBlcok.lottoHeader.llPool;
    std::vector<CTransaction> vtx = curTipBlcok.vtx;

      BOOST_FOREACH(const CTransaction& tx, vtx)
      {
      	if(!tx.IsCoinBase())
      	{
			 BOOST_FOREACH(const CTxOut& out, tx.vout)
			 nPool -= out.nValue;
      	}
      }
      LogTrace2("bet_test","\nnPool 25 %ld", nPool);
      CBlockIndex * pindexPrev1 = chainActive[24];
      int64_t reminMoney = pindexPrev1->lottoHeader.llPool - reviveMoney;
      LogTrace2("bet_test","\nnpindexPrev1 25 %ld", pindexPrev1->lottoHeader.llPool);
      LogTrace2("bet_test","\nreviveMoney 25 %ld",reviveMoney);
      ///// check 24 and 25 about the reward money is correct
      BOOST_CHECK_EQUAL(reminMoney, nPool);
}
BOOST_AUTO_TEST_CASE(rpc_lotto_bet2)
{
    Value r;
//  LOCK(pwalletMain->cs_wallet);
    r = CallRPC("getbalance ");
    double nbalance = r.get_real();
    if(nbalance < 5)
    {

		CallRPC("setgenerate true 12");

    }
    r = CallRPC("getbalance ");
    nbalance = r.get_real();
    LogTrace2("bet_test","\nbalance:%s", nbalance);
     r= CallRPC("getnewaddress ");

 	CReserveKey keyReward(pwalletMain);
 	CPubKey rewardPubKey;
 	bool ret;
 	ret = keyReward.GetReservedKey(rewardPubKey);
 	assert(ret); // should never fail, as we just unlocked
    string address =CBitcoinAddress(rewardPubKey.GetID()).ToString();

    LogTrace2("bet_test","\nrecvie reward address:%s", address);
    r = CallRPC("getbalance ");
    nbalance = r.get_real();
    BOOST_CHECK_NO_THROW(CallRPC("sendlottobet \"\" n36sSjfPJMnroJy3BBHkus1TbvsAykiFCr 5000 0 010203040509 "+address));
    CallRPC("setgenerate true 23");
    cout<<endl;
    r = CallRPC("getbalance ");
    nbalance = r.get_real();

    r = CallRPC("getacountbyaddress "+address);
    int64_t reviveMoney = r.get_uint64();
    int64_t chekValue = ((int64_t)(5000*3.77613*0.01 *100000000));
    BOOST_CHECK_EQUAL(reviveMoney, chekValue);

    //// height 25 13
	CBlockIndex * pindexPrev = chainActive[25];
	CBlock curTipBlcok;
  	CDiskBlockPos curTipPos(pindexPrev->nFile, pindexPrev->nDataPos);
  	if(!ReadBlockFromDisk(curTipBlcok, curTipPos))
  		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
  	int64_t nPool = curTipBlcok.lottoHeader.llPool;
    std::vector<CTransaction> vtx = curTipBlcok.vtx;

      BOOST_FOREACH(const CTransaction& tx, vtx)
      {
      	if(!tx.IsCoinBase())
      	{
			 BOOST_FOREACH(const CTxOut& out, tx.vout)
			 nPool -= out.nValue;
      	}
      }
      LogTrace2("bet_test","\nnPool 25 %ld", nPool);
      CBlockIndex * pindexPrev1 = chainActive[24];
      int64_t reminMoney = pindexPrev1->lottoHeader.llPool - reviveMoney;
      LogTrace2("bet_test","\nnpindexPrev1 25 %ld", pindexPrev1->lottoHeader.llPool);
      LogTrace2("bet_test","\nreviveMoney 25 %ld",reviveMoney);
      ///// check 24 and 25 about the reward money is correct
      BOOST_CHECK_EQUAL(reminMoney, nPool);
}
BOOST_AUTO_TEST_SUITE_END()
