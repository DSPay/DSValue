// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "assert.h"
#include "core.h"
#include "protocol.h"
#include "util.h"
#include "bitcoinnet/CChainManager.h"
#include <boost/assign/list_of.hpp>

using namespace boost::assign;

//
// Main network
//
//const int64_t LTC_START_HIGHT = 584876;
//const uint256 hashLiteCoinStartBlock = uint256("0x0ee119a4630cd315850ca67f3f5f7a7adeb774ee87da85c6834a51fe097bf347");
//const uint256 hashLiteCoinStartPreBlock = uint256("26d696987eda4c92e8989fb936f33c1f6a984e69e1819d4b3c5a544b0b58a033");
//const uint256 hashLiteCoinStartMerkleRoot =  uint256("bb39d541cf8968683e6ca5dcc11cd4f5841921e773807b31e3a2bb6072b0eb4b");
//const uint256 hashLiteCoinGenesisBlock = uint256("0x12a765e31ffd4059bada1e25190f6e98c99d9714d334efa41a195a7e7e04bfe2");

const int64_t BTC_START_HIGHT = 308447;
const uint256 hashBitcoinStartBlock = uint256("00000000000000003ff2a53152ee98910d7383c0177459ad258c4b2d2c4d4610");
const uint256 hashBitcoinStartPreBlock = uint256("00000000000000002eed85d8b873670ff9608a32b8f58f052c301477993cf9ee");
const uint256 hashBitcoinStartMerkleRoot =  uint256("06d27706d63d7bc4cd6624a799c06510c9d7ed94d854b647568fc224df9000c3");

unsigned int pnSeed[] =
{
    0xa0fb1d73, 0xca561c73
};

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xff;
        pchMessageStart[1] = 0xfe;
        pchMessageStart[2] = 0x1d;
        pchMessageStart[3] = 0x20;
        LottoKeyMerkleRoot = "4e327a89024dea2397332e826aa0d787d3d008f3e489ada07c8b02ae78c83c5a";
        vAlertPubKey = ParseHex("02da1cf00411326ed0a29ed307666ace2a7a071bac0276d4c5d507710b7c3b6022");
        nDefaultPort = 8666;
        nRPCPort = 8667;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 8);
        nSubsidyHalvingInterval = 210000;

        // Build the genesis block. Note that the output of the genesis coinbase cannot
        // be spent as it did not originally exist in the database.
        //
        // CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
        //   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
        //     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
        //     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
        //   vMerkleTree: 4a5e1e
        const char* pszTimestamp = "The Times 03/Jul/2014 Chancellor on brink of second bailout for banks";
        CTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 536936447 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 50 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1404165600;
        genesis.nBits    = 0x2000ffff;
        genesis.nNonce   = 35;


#ifndef BETCOIN
        assert(hashGenesisBlock == uint256("0x000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"));
#endif
        assert(genesis.hashMerkleRoot == uint256("0xff28d78e02b6655055a494258347aaf79e52c76193babce79bfb283ba787b83a"));

        vSeeds.push_back(CDNSSeedData("wallet.org", "wallet.dspay.org"));
        vSeeds.push_back(CDNSSeedData("dspay.org", "seed.dspay.org"));

        base58Prefixes[PUBKEY_ADDRESS] = list_of(0x1f);
        base58Prefixes[SCRIPT_ADDRESS] = list_of(5);
        base58Prefixes[SECRET_KEY] =     list_of(128);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xAD)(0xE4);

        // Convert the pnSeeds array into usable address objects.
        for (unsigned int i = 0; i < ARRAYLEN(pnSeed); i++)
        {
            // It'll only connect to one or two seed nodes because once it connects,
            // it'll get a pile of addresses with newer timestamps.
            // Seed nodes are given a random 'last seen time' of between one and two
            // weeks ago.
            const int64_t nOneWeek = 7*24*60*60;
            struct in_addr ip;
            memcpy(&ip, &pnSeed[i], sizeof(ip));
            CAddress addr(CService(ip, GetDefaultPort()));
            addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
            vFixedSeeds.push_back(addr);
        }

        genesis.lottoHeader.nVersion = 1;
        genesis.lottoHeader.llPool = 2000000*COIN;
        genesis.lottoHeader.mBitcoinHash.insert(std::make_pair(BTC_START_HIGHT, hashBitcoinStartBlock));
//        for(int i=1; i<0xffffffff; ++i)
//        {
//        	genesis.nNonce = i;
//			CBigNum bnTarget;
//			bnTarget.SetCompact(genesis.nBits);
//        	if(genesis.GetHash()<=bnTarget.getuint256()){
//        		cout << "main nonce:"<<i<< " Target hash:"  << bnTarget.getuint256().GetHex() << endl;
//        		cout << "main nonce:"<<i<< " genesis hash:"  << genesis.GetHash().GetHex() << endl;
//        		break;
//        	}
//        }
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x008f0cf78adab363b50af2da12993a70542c9b8e073a54322f1b6b171b651dd5"));
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet (v3)
//
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xfe;
        pchMessageStart[1] = 0x2d;
        pchMessageStart[2] = 0x1c;
        pchMessageStart[3] = 0x0d;
        vAlertPubKey = ParseHex("02da1cf00411326ed0a29ed307666ace2a7a071bac0276d4c5d507710b7c3b6022");
        nDefaultPort = 18666;
        nRPCPort = 18667;
        strDataDir = "testnet3";
        LottoKeyMerkleRoot = "b2a8ce5ac4aab2a6f1d7ef694d72b3c05ace80d543feeeb7e320be7c534ab4d0";
        // Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1404252000;
        genesis.nNonce = 238;

//      for(int i=1; i<0xffffffff; ++i)
//		{
//			genesis.nNonce = i;
//			CBigNum bnTarget;
//			bnTarget.SetCompact(genesis.nBits);
//			if(genesis.GetHash()<=bnTarget.getuint256()){
//				cout << "test nonce:"<<i<< " Target hash:" << bnTarget.getuint256().GetHex() << endl;
//				cout << "test nonce:"<<i<< " genesis hash:" << genesis.GetHash().GetHex() << endl;
//				break;
//			}
//		}
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x0092e80fc7ce1f557d7f845dafff956905a584c47f656493489e847e0acd5742"));
#ifndef BETCOIN
        assert(hashGenesisBlock == uint256("0x000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943"));
#endif
        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("wallet.org", "wallet.dspay.org"));
        vSeeds.push_back(CDNSSeedData("dspay.org", "seed.dspay.org"));

        base58Prefixes[PUBKEY_ADDRESS] = list_of(0x5b);
        base58Prefixes[SCRIPT_ADDRESS] = list_of(196);
        base58Prefixes[SECRET_KEY]     = list_of(239);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94);
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;


//
// Regression test
//
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {

    	LottoKeyMerkleRoot = "b2a8ce5ac4aab2a6f1d7ef694d72b3c05ace80d543feeeb7e320be7c534ab4d0";
#if (BETCOIN == 1)
		pchMessageStart[0] = 0xfc;
		pchMessageStart[1] = 0x1d;
		pchMessageStart[2] = 0x2d;
		pchMessageStart[3] = 0x3d;
		nSubsidyHalvingInterval = 150;
		bnProofOfWorkLimit = CBigNum(~uint256(0) >> 4);//0000 1111
		genesis.nTime = 1404273600;
		genesis.nBits = 0x200fffff;//0000 1111
		genesis.nNonce = 67;
		hashGenesisBlock = genesis.GetHash();
		nDefaultPort = 18444;
		strDataDir = "regtest";

//		for(int i=1; i<0xffffffff; ++i)
//		{
//			genesis.nNonce = i;
//			CBigNum bnTarget;
//			bnTarget.SetCompact(genesis.nBits);
//			if(genesis.GetHash()<=bnTarget.getuint256()){
//				cout << "regtest nonce:"<<i<< " Target hash:" << bnTarget.getuint256().GetHex() << endl;
//				cout << "regtest nonce:"<<i<< " genesis hash:" << genesis.GetHash().GetHex() << endl;
//				break;
//			}
//		}

#if 0
//        x11 para:
//      bnProofOfWorkLimit = CBigNum(~uint256(0) >> 4);//0000 1111
//		genesis.nTime = 1296688602;
//		genesis.nBits = 0x200fffff;//0000 1111
//		genesis.nNonce = 6;
//      assert(hashGenesisBlock == uint256("036c60a37eb87f2eb7283a01fa43d612df4bf7ef3b475c0d26a99e1e15644e51"));

//        LtcoinBlockHash para:
//      bnProofOfWorkLimit = CBigNum(~uint256(0) >> 4);//0000 1111
//		genesis.nTime = 1296688602;
//		genesis.nBits = 0x200fffff;//0000 1111
//		genesis.nNonce = 34;
//      assert(hashGenesisBlock == uint256("0e89079c7d16f4b4689924fa08cf368f10b89254660ccffe5d51d51c04a7ba37"));

		while(true)
		{
			hashGenesisBlock = genesis.GetHash();

			if(hashGenesisBlock < uint256("0x0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"))
			{
				LogPrintf("hashGenesisBlock = %s genesis.nNonce = %d \n", hashGenesisBlock.ToString(), genesis.nNonce);
				break;
			}

			genesis.nNonce += 1;

		}
#endif
		assert(hashGenesisBlock == uint256("0x0bf1f174525aff948a51086ab962397ccc8b3e827b95bbd2be47eb984043e5ac"));
		vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
#elif(BETCOIN == 2)
		pchMessageStart[0] = 0xfa;
		pchMessageStart[1] = 0xbf;
		pchMessageStart[2] = 0xb5;
		pchMessageStart[3] = 0xda;
		nSubsidyHalvingInterval = 150;
		bnProofOfWorkLimit = CBigNum(~uint256(0) >> 4);//0000 1111
		genesis.nTime = 1296688602;
		genesis.nBits = 0x200fffff;//0000 1111
		genesis.nNonce = 34;
		hashGenesisBlock = genesis.GetHash();
		nDefaultPort = 18444;
		strDataDir = "regtest";

		assert(hashGenesisBlock == uint256("0e89079c7d16f4b4689924fa08cf368f10b89254660ccffe5d51d51c04a7ba37"));

		vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
#else
        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nSubsidyHalvingInterval = 150;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 1);
        genesis.nTime = 1296688602;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 2;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 18444;
        strDataDir = "regtest";
        assert(hashGenesisBlock == uint256("0x0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"));

        vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
#endif
    }

    virtual bool RequireRPCPassword() const { return false; }
    virtual Network NetworkID() const { return CChainParams::REGTEST; }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = &mainParams;
//static CChainParams *pCurrentParams = &regTestParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        case CChainParams::REGTEST:
            pCurrentParams = &regTestParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    bool fRegTest = GetBoolArg("-regtest", false);
    bool fTestNet = GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest) {
        return false;
    }

    if (fRegTest) {
        SelectParams(CChainParams::REGTEST);
    } else if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
