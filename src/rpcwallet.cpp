// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
#include "CAutoTest.h"
#include "CEncryptKey.h"
#include "bitcoinnet/CChainManager.h"
#include "checkpoints.h"
using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

int64_t nWalletUnlockTime;
static CCriticalSection cs_nWalletUnlockTime;
int nFlag = 0;

static inline FILE* OpenFile1(const char* filename, bool fReadOnly) {
	boost::filesystem::path path = GetDataDir() / filename;
	FILE* file = fopen(path.string().c_str(), "wb");
	if (!file && !fReadOnly)
		file = fopen(path.string().c_str(), "wb");
	if (!file) {
		LogPrintf("Unable to open file %s\n", path.string());
		return NULL;
	}
	return file;
}

Value listluckynum(const Array& params, bool fHelp)
{
	if (fHelp) {
		throw runtime_error("getluckynum \n"
				"\nArguments:\n"
				"1. \"hight\"         (int64, required) the hight of the check point \n");
	}
	boost::int32_t intTemp(-1);
	if (params.size() > 0)
		intTemp = params[0].get_int();

	if (intTemp > chainActive.Height() || intTemp < 0)
		intTemp = chainActive.Height();

	std::string strlottoret("");
	while (intTemp > 0) {
		int nID(0);
		uint256 uLottoKey(0);
		GetLotteryKey(chainActive[intTemp]->pprev->nHeight, nID, uLottoKey);

		std::vector<CTxOut> vTxOut;
		int64_t llRewards(0);
		strlottoret += DrawLottery(chainActive[intTemp]->lottoHeader.llPool, chainActive[intTemp]->pprev, vTxOut,
				llRewards, nID, uLottoKey);
		intTemp--;
	}
	return strlottoret;
}


Value getluckynum(const Array& params, bool fHelp)
{
	if(fHelp || params.size() < 1)
	{
		throw runtime_error(
				 "getluckynum \n"
				 "\nArguments:\n"
				 "1. \"hight\"         (int64, required) the hight of the check point \n" );
	}
	boost::int32_t intTemp = params[0].get_int();
	Object obj;
	if (intTemp > 0 && intTemp <= chainActive.Height())
	{
		std::vector<CTxOut> vBet;
		CBlock lotteryBlock;
		CBlockIndex* pDrawLotteryIndex = chainActive[intTemp];
		lotto::CLottoHeader header= chainActive[intTemp]->GetBlockHeader().lottoHeader;
		int id = header.nLottoID;
		uint256 nshash = header.uLottoKey;
		if (pDrawLotteryIndex->nHeight <= 2 * nIntervalLottery)
			return obj;
		if(nshash != uint256(0) && nshash != uint256(1))
		{
			uint256 bithash = GetLotteryHash(pDrawLotteryIndex);
			if(bithash == uint256(0)) return "";
			cout<<"index:"<<id<<"lottokey:%s"<<nshash.ToString()<<endl;
			lotto::CLotto plotto(bithash,id,nshash,0,vBet);
			std::map<lotto::BET_TYPE, std::vector<unsigned char> > mLuckyStar = plotto.GetLuckyNum();
			BOOST_FOREACH(const PAIRTYPE(lotto::BET_TYPE, vector<unsigned char>)& tep2 , mLuckyStar)
			{
				  string first= "";
				  first+=tfm::format("%d",tep2.first);
				  obj.push_back(Pair(first,   memdump(tep2.second) ));
			}
		}
	}
	return obj;
}

class  Info
{
public:
	uint256 blockHash;
	int nHeight;
	int64_t nValue;
	int type;
	Info(uint256 blockHas,int nHeight,int64_t nValue,int type = 0)
	{
		this->blockHash = blockHas;
		this->nHeight = nHeight;
		this->nValue = nValue;
		this->type = type;
	}
	string ToString()const
	{
		return tfm::format("Height:%d  Value:%d type:%d\n    blockHash: %s\n",nHeight,nValue,type,blockHash.GetHex());
	}
};

template<typename T>
void ConvertTo(Value& value, bool fAllowNull=false)
{
    if (fAllowNull && value.type() == null_type)
        return;
    if (value.type() == str_type)
    {
        // reinterpret string as unquoted json value
        Value value2;
        string strJSON = value.get_str();
        if (!read_string(strJSON, value2))
            throw runtime_error(string("Error parsing JSON:")+strJSON);
        ConvertTo<T>(value2, fAllowNull);
        value = value2;
    }
    else
    {
        value = value.get_value<T>();
    }
}
string GetTypelucknum(int type,int height)
{
	string first= "";
	string ret= "";
	first+=tfm::format("%d",type);
	Array param;
	string tem= "";
	tem += tfm::format("%d",height);
	param.push_back(tem);
	ConvertTo<boost::int64_t>(param[0]);
	Value result = getluckynum(param, false);
	 if(result.type() ==obj_type)
	 {
			const json_spirit::Object& obj = result.get_obj();
			size_t i = 0;
			for(; i < obj.size(); ++i)
			{
				const json_spirit::Pair& pair = obj[i];
				const std::string& str_name = pair.name_;
				if(str_name == first)
				{
					ret = pair.value_.get_str();
				}
			}
	 }
	 return ret;
}
int64_t ComputTheBetReturnValue(int height,CTxOut txout)
{
		std::vector<CTxOut> vBet;
		vBet.push_back(txout);
		CBlockIndex* blockindex = chainActive[height];
		int64_t Pool= chainActive[height-1]->GetBlockHeader().lottoHeader.llPool;
		uint256 bithash = GetLotteryHash(blockindex);
		string str = "";
		str +=tfm::format("\n----preblock pool height:%d pool money:%ld\n",height-1,Pool);
		LogTrace("pool","%s",str);
		if(blockindex->lottoHeader.uLottoKey != uint256(0))
		{
			lotto::CLotto plotto(bithash,blockindex->lottoHeader.nLottoID,blockindex->lottoHeader.uLottoKey,Pool,vBet);
			std::vector<CTxOut> ret;
			int64_t llRemains = 0;
			plotto.GetReWardOutput(ret,llRemains);
			if(ret.size() >=1)
			return ret[0].nValue;
		}
		return 0;
}

Value gettransactionbyaddr(const Array& params, bool fHelp) {
	if (fHelp || params.size() < 1) {
		throw runtime_error("gettransactionbyaddr \n"
				"\nArguments:\n"
				"1. \"address\"         (string, required) get the transaction of the address \n");
	}

	map<uint256, CTransaction> revMoneyTx;
	map<uint256, CTransaction> spendTx;
	map<uint256,Info> spendInfo;
	map<uint256, Info> revMoneyInfo;
	string address = params[0].get_str();
	string strBetData ="";
	string str= "";
	int64_t reciveCount =0;
	int64_t SpendCount =0;
	int64_t ComputeResult =0;
	int64_t betCount =0;
	CBlockIndex *pBlockIndex = chainActive.Genesis();
	while ((pBlockIndex = chainActive.Next(pBlockIndex))) {
		CBlock block;
		if (!ReadBlockFromDisk(block, pBlockIndex))
			return error("CreateNewBlock() : Read current tip block from disk failed");

		BOOST_FOREACH(CTransaction &tx, block.vtx) {
			bool bFind = false;
			BOOST_FOREACH(CTxOut &txOut, tx.vout) {
				CTxDestination desAddress;
				if (ExtractDestination(txOut.scriptPubKey, desAddress)) {
					if (address == CBitcoinAddress(desAddress).ToString()) {
						//// comput the address recive money
						reciveCount += txOut.nValue;
						revMoneyTx.insert(std::make_pair(tx.GetHash(),tx));
						revMoneyInfo.insert(std::make_pair(tx.GetHash(), Info(pBlockIndex->GetBlockHash(),pBlockIndex->nHeight,txOut.nValue)));
						break;
					}
				}
				TransactionState iTxType(NORMAL_STATE);
				if (txOut.GetLottoTxOutType(iTxType)) {
					if (iTxType == VALID_LOTTO_STATE) {
						std::vector<unsigned char> betdata;
						txOut.scriptPubKey.GetBetData(betdata);
						lotto::CBetData strData;
						strData << betdata;
						if (address == CBitcoinAddress(CKeyID(strData.GetAddr())).ToString()) {

							strBetData += tfm::format("\n----put in block height:%d,bockhash:%s\n",
									pBlockIndex->nHeight, block.GetHash().ToString());
							strBetData += tfm::format("\n----type:%d,value:%ld,slelectnum:%s,reciveaddress:%s\n",
									strData.GetType(), txOut.nValue, HexStr(strData.GetSelectV()),
									CBitcoinAddress(CKeyID(strData.GetAddr())).ToString());
							int beginlottoblock = pBlockIndex->nHeight + 2 * nIntervalLottery;
							if (beginlottoblock > chainActive.Height()) {
								strBetData += "----the block bet have't begin lotto\n";

							} else {
								string nsrt = GetTypelucknum(strData.GetType(), beginlottoblock);
								strBetData += tfm::format("\n----blockeheight:%d,luckey number:%s\n", beginlottoblock,
										nsrt);
								int64_t ret = ComputTheBetReturnValue(beginlottoblock, txOut);
								strBetData += tfm::format("\n--------comput result:%ld----------------\n\n", ret);

								str += tfm::format(
										"\n----draw lotto pool height:%d--- pool money:%ld,---recive reward:%ld\n",
										beginlottoblock, pBlockIndex->lottoHeader.llPool, ret);
								ComputeResult += ret;
								betCount += txOut.nValue;
							}

						}
					}

				}
			}

			BOOST_FOREACH(CTxIn &txIn, tx.vin) {
				CTxDestination desAddress;
				if (ExtractDestination(txIn.scriptSig, desAddress)) {
//					cout<<"recivaddress"<<CBitcoinAddress(desAddress).ToString()<<endl;
					if (address == CBitcoinAddress(desAddress).ToString()) {
						int64_t nValue= revMoneyTx[txIn.prevout.hash].vout[txIn.prevout.n].nValue;//spend money
						spendInfo.insert(std::make_pair(tx.GetHash(), Info(pBlockIndex->GetBlockHash(),pBlockIndex->nHeight,nValue)));
						spendTx.insert(std::make_pair(tx.GetHash(),tx));
						SpendCount += nValue;
						break;
					}
				}

			}
		}
//		pBlockIndex = pBlockIndex->pprev;
	}
	string tem=tfm::format("\nrevMoneyInfo:   Addr: :%s \n""****************************************\n", address);
	int conter = 0 ;
	BOOST_FOREACH(const PAIRTYPE(uint256, Info)& tep1 , revMoneyInfo)
	{
		tem += tfm::format("txId: :%s\nInfo%d:\n    %s\n",tep1.first.ToString(),conter++,tep1.second.ToString());
	}
	tem +="----------------------------------------------\n";
	tem += tfm::format("spendInfo:    Addr: :%s \n", address);
	conter = 0 ;
	BOOST_FOREACH(const PAIRTYPE(uint256, Info)& tep2 , spendInfo)
		{
			tem += tfm::format("txId: :%s \nInfo%d:\n    %s\n",tep2.first.ToString(),conter++,tep2.second.ToString());
		}
	tem +=tfm::format("betinformatiom:%s\n",strBetData);

	tem +=tfm::format("\nrevMoneyCount:   account: :%ld  spendMoneyCount:   account: :%ld computeResult:%ld\n""****************\n", reciveCount,SpendCount,ComputeResult);

	str +=tfm::format("\nrevMoneyCount:   account: :%ld  spendMoneyCount:   account: :%ld computeResult:%ld,bet money count:%ld\n""****************\n", reciveCount,SpendCount,ComputeResult,betCount);
	LogTrace("pool","%s",str);
	return tem +"****************************************end\n";
}


Value gettransactionbyheight(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 2) {
			throw runtime_error("gettransactionbyheight \n"
					"\nArguments:\n"
					"1. \"height\"         (string, required) get the transaction of the address \n"
					"2. \"transaction hash\" (string, required) the transaction hash");
	}
	int nHeight = params[0].get_int64();
	std::string txHash = params[1].get_str();
	CBlock block;
	CBlockIndex* pblockindex = chainActive[nHeight];
	if(pblockindex==NULL)
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: Height less 0 or more than current chainactive height \n");
	ReadBlockFromDisk(block, pblockindex);

	Object result;
	BOOST_FOREACH(const CTransaction &tx, block.vtx) {
		if (txHash == tx.GetHash().GetHex()) {
			result.push_back(Pair("txid", txHash));
			BOOST_FOREACH(const CTxIn &txIn, tx.vin)
				result.push_back(Pair("CTxIn", txIn.ToString()));
			BOOST_FOREACH(const CTxOut &txOut, tx.vout)
				result.push_back(Pair("CTxOut", txOut.ToString()));
		}
	}
	return result;
}

Value listlottokey(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 0) {
				throw runtime_error("listlottokey \n"
						"\nArguments:\n"
						"return local server lottokey");
		}
	Object result;
	int nID(0), preID(0);
	uint256 uLottoKey(0);
	int nHeight = chainActive.Height();
	for(int i=0; i<nHeight; ++i){
		GetLotteryKey(i, nID, uLottoKey);
		if (preID != nID) {
			result.push_back(Pair(tfm::format("%d",nID).c_str(), uLottoKey.GetHex()));
			preID = nID;
		}
	}
	return result;
}

Value listpoolchange(const Array& params, bool fHelp) {
	if (fHelp || params.size() != 1) {
		throw runtime_error("listpoolchange \n"
					"\nArguments: (require) the block height\n");
	}
	int height = params[0].get_int();
	string strBetData ="";
	string str= "";
	int64_t ComputeResult =0;
	int64_t SpendMoney =0;
	int64_t betCount =0;
	int64_t rewardCount =0;
	int64_t pool =0;
	CBlockIndex *pBlockIndex = chainActive.Genesis();
	while ((pBlockIndex = chainActive.Next(pBlockIndex)) && pBlockIndex->nHeight <= height) {
		int64_t Pool= chainActive[pBlockIndex->nHeight-1]->GetBlockHeader().lottoHeader.llPool;
		LogTrace("poolchange","\n----preblock pool height:%d pool money:%ld\n",pBlockIndex->nHeight-1,Pool);
		CBlock block;
		if (!ReadBlockFromDisk(block, pBlockIndex))
			return error("CreateNewBlock() : Read current tip block from disk failed");

		BOOST_FOREACH(CTransaction &tx, block.vtx) {
			bool bFind = false;
			if(tx.IsCoinBase())
			{
				for(int i=1;i <tx.vout.size();i++)
					ComputeResult +=tx.vout[i].nValue;
			}
			BOOST_FOREACH(CTxOut &txOut, tx.vout) {
				TransactionState iTxType(NORMAL_STATE);
				if (txOut.GetLottoTxOutType(iTxType)){
					if (iTxType == VALID_LOTTO_STATE){
						SpendMoney += txOut.nValue;
					}
				}
			}

		}
		betCount +=SpendMoney;
		rewardCount += ComputeResult;
		pool = block.lottoHeader.llPool;
		LogTrace("poolchange","\n----draw pool height:%d pool money:%ld,spend money:%ld,recive money:%ld\n",pBlockIndex->nHeight,block.lottoHeader.llPool,SpendMoney,ComputeResult);
		SpendMoney = 0;
		ComputeResult = 0;
	}
	LogTrace("poolchange","\n\n----current pool:%ld  bet money count:%ld,reward money count:%ld\n",pool,betCount,rewardCount);
	return "";//tem +"****************************************end\n";
}

Value setbtchashtip(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 1) {
			throw runtime_error("setbtchashtip \n"
					"\nArguments:\n"
					"1. \"hash\"  (string, required) btc block hash \n");
		}
	std::string btcHashStr = params[0].get_str();
	uint256 btcHash(btcHashStr);
	CBlockIndex* pBlockIndex = Bitcoin::CChainManager::CreateChainManagerInstance().GetBitcoinBlockIndex(btcHash);
	Assert(pBlockIndex);
	Bitcoin::CChainManager::CreateChainManagerInstance().SetChainTip(pBlockIndex);
	return "";
}
Value sendcheckpoint(const Array& params, bool fHelp)
{
	if(fHelp || params.size() != 2)
	{
		throw runtime_error(
				 "sendcheckpoint \n"
				 "\nArguments:\n"
				 "1. \"hight\"         (int64, required) the hight of the check point \n"
				 "2. \"password\"      (string, required) the password to get the private key\n" );
	}
	boost::int32_t intTemp = params[0].get_int();
	std::string password = params[1].get_str();
	std::vector<unsigned char> tep;
	lotto::DspayKeyFile.ReadPrivateKey(0, password, tep);
	if (!tep.empty() && intTemp > 0 && intTemp <= chainActive.Height())
	{
		SyncData::CSyncData data;
		SyncData::CSyncCheckPoint point;
		CDataStream sstream(SER_NETWORK, PROTOCOL_VERSION);
		point.m_height = intTemp;
		CBlock block;
		ReadObjFromFile("blockfile.data",0,block);
		LogPrintf("sendcheckpoint");
		block.print();
		point.m_hashCheckpoint = block.GetHash();//chainActive[intTemp]->GetBlockHash();
		LogTrace("bess2","send hash = %s",block.GetHash().ToString());
		sstream << point;
		if (data.Sign(tep, std::vector<unsigned char>(sstream.begin(), sstream.end()))
			&& data.CheckSignature(SyncData::strSyncDataPubKey))
		{
			SyncData::CSyncDataDb db;
			std::vector<SyncData::CSyncData> vdata;
			db.WriteCheckpoint(intTemp, data);
			Checkpoints::AddCheckpoint(point.m_height, point.m_hashCheckpoint);
			CheckActiveChain(point.m_height, point.m_hashCheckpoint);
			vdata.push_back(data);
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				if (pnode->setcheckPointKnown.count(intTemp) == 0)
				{
					pnode->setcheckPointKnown.insert(intTemp);
					pnode->PushMessage("checkpoint", vdata);
				}
			}
		}
	}
	return tfm::format("sendcheckpoint :%d\n", intTemp);
}

Value sendcheckpointchain(const Array& params, bool fHelp)
{
	if(fHelp || params.size() != 2)
	{
		throw runtime_error(
				 "sendcheckpoint \n"
				 "\nArguments:\n"
				 "1. \"hight\"         (int64, required) the hight of the check point \n"
				 "2. \"password\"      (string, required) the password to get the private key\n" );
	}
	boost::int64_t intTemp = params[0].get_int64();
	std::string password = params[1].get_str();
	std::vector<unsigned char> tep;
	lotto::DspayKeyFile.ReadPrivateKey(0, password, tep);
	if (!tep.empty() && intTemp > 0 && intTemp <= chainActive.Height())
	{
		SyncData::CSyncData data;
		SyncData::CSyncCheckPoint point;
		CDataStream sstream(SER_NETWORK, PROTOCOL_VERSION);
		point.m_height = intTemp;
		Assert(chainActive[intTemp]);
		point.m_hashCheckpoint = chainActive[intTemp]->GetBlockHash();
		LogTrace("bess2","send hash = %s",point.m_hashCheckpoint.GetHex());
		sstream << point;
		if (data.Sign(tep, std::vector<unsigned char>(sstream.begin(), sstream.end()))
			&& data.CheckSignature(SyncData::strSyncDataPubKey))
		{
			SyncData::CSyncDataDb db;
			std::vector<SyncData::CSyncData> vdata;
			db.WriteCheckpoint(intTemp, data);
			Checkpoints::AddCheckpoint(point.m_height, point.m_hashCheckpoint);
			CheckActiveChain(point.m_height, point.m_hashCheckpoint);
			vdata.push_back(data);
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				if (pnode->setcheckPointKnown.count(intTemp) == 0)
				{
					pnode->setcheckPointKnown.insert(intTemp);
					pnode->PushMessage("checkpoint", vdata);
				}
			}
		}
	}
	return tfm::format("sendcheckpoint :%d\n", intTemp);
}

Value saveblocktofile(const Array& params, bool fHelp)
{
	cout<<params.size()<<endl;
	if(fHelp || params.size() < 1)
	{
		throw runtime_error(
				 "saveblocktofile \n"
				 "\nArguments:\n"
				 "1. \"height\"         (int64, required) the hight of the lottery key \n" );
	}
	int blockHeight =  params[0].get_int();
	cout<<blockHeight<<endl;
	cout<<chainActive.Height()<<endl;
	if (blockHeight > 0 && blockHeight <= chainActive.Height())
	{
		CBlockIndex *indexblock =chainActive[blockHeight];
		Assert(indexblock);
		CBlock block;
		CDiskBlockPos curTipPos(indexblock->nFile, indexblock->nDataPos);
	 	if(!ReadBlockFromDisk(block, curTipPos))
	  		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
		boost::filesystem::path path = GetDataDir() / "blockfile.data";
		cout<<path.string().c_str()<<endl;
		block.print();
		FILE* fp=OpenFile1("blockfile.data",true);
		fclose(fp);
	 	AppendToFile("blockfile.data", block);
	}
	return tfm::format("saveblocktofile :%d\n", blockHeight);
}

Value sendlottokey(const Array& params, bool fHelp)
{
	if(fHelp || params.size() != 3)
	{
		throw runtime_error(
				 "sendlottokey \n"
				 "\nArguments:\n"
				 "1. \"index\"         (int64, required) the hight of the lottery key \n"
				 "2. \"password\"      (string, required) the password to get the private key\n"
				 "3. \"password\"      (string, required) the password to get the lottery key\n");
	}
	boost::int32_t intTemp = params[0].get_int();
	cout<<endl<<"sendlottokey:"<<intTemp<<endl;
	LogTrace("bess4","sendlottokey:%d",intTemp);
	CLottoFileKey key2;
	std::string privateKey = params[1].get_str();
	std::string lottoPrivateKey = params[2].get_str();
	std::vector<unsigned char> tep;
	std::vector<unsigned char> key;
	lotto::DspayKeyFile.ReadPrivateKey(0, privateKey, tep);
	if(!tep.empty() && (intTemp >= 0)
	   && lotto::DspayKeyFile.ReadClottoKey(intTemp, lottoPrivateKey, key))
	{
		std::cout<<"sendlottokey:"<<privateKey<<", "<<lottoPrivateKey<<", "<<HexStr(key)<<endl;
		uint256 temp(key);
		std::vector<uint256> vckey;
		vckey.push_back(temp);
		key2.setIndex(intTemp);
		key2.setKey(vckey);
		SyncData::CSyncData data;
		CDataStream sstream(SER_NETWORK, PROTOCOL_VERSION);
		sstream << key2;
		if (data.Sign(tep, std::vector<unsigned char>(sstream.begin(), sstream.end()))
			&& data.CheckSignature(SyncData::strSyncDataPubKey))
		{
			std::vector<SyncData::CSyncData> vdata;
			SyncData::CSyncDataDb db;
			db.WriteLotteryKey(intTemp, data);
			vdata.push_back(data);
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				if (pnode->setLotteKeyKnown.count(intTemp) == 0)
				{
					pnode->setLotteKeyKnown.insert(intTemp);
					pnode->PushMessage("lotterykey", vdata);
				}
			}
		}
	}
	return tfm::format("sendlottokey :%d\n", intTemp);
}

Value gettotalcoinsvalues(const json_spirit::Array& params, bool fHelp)
{
	if(fHelp || params.size() < 1)
	{
		throw runtime_error(
				 "gettotalcoinsvalues \n"
				 "\nArguments:\n"
				 "1. \"address\"         (string, required) a address of a wallet.\n" );
	}
	std::string address = params[0].get_str();
	int64_t values = 0;
	if (pcoinsTip)
	{
		values = pcoinsTip->GetTotalCoinsValues(address);
	}
	return ValueFromAmount(values);
}

std::string HelpRequiringPassphrase()
{
    return pwalletMain && pwalletMain->IsCrypted()
        ? "\nRequires wallet passphrase to be set with walletpassphrase call."
        : "";
}

void EnsureWalletIsUnlocked()
{
    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
}

void WalletTxToJSON(const CWalletTx& wtx, Object& entry)
{
    int confirms = wtx.GetDepthInMainChain();
    entry.push_back(Pair("confirmations", confirms));
    if (wtx.IsCoinBase())
        entry.push_back(Pair("generated", true));
    if (confirms > 0)
    {
        entry.push_back(Pair("blockhash", wtx.hashBlock.GetHex()));
        entry.push_back(Pair("blockindex", wtx.nIndex));
        entry.push_back(Pair("blocktime", (boost::int64_t)(mapBlockIndex[wtx.hashBlock]->nTime)));
    }
    uint256 hash = wtx.GetHash();
    entry.push_back(Pair("txid", hash.GetHex()));
    Array conflicts;
    BOOST_FOREACH(const uint256& conflict, wtx.GetConflicts())
        conflicts.push_back(conflict.GetHex());
    entry.push_back(Pair("walletconflicts", conflicts));
    entry.push_back(Pair("time", (boost::int64_t)wtx.GetTxTime()));
    entry.push_back(Pair("timereceived", (boost::int64_t)wtx.nTimeReceived));
    BOOST_FOREACH(const PAIRTYPE(string,string)& item, wtx.mapValue)
        entry.push_back(Pair(item.first, item.second));
}

string AccountFromValue(const Value& value)
{
    string strAccount = value.get_str();
    if (strAccount == "*")
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account name");
    return strAccount;
}

Value getnewaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getnewaddress ( \"account\" )\n"
            "\nReturns a new Bitcoin address for receiving payments.\n"
            "If 'account' is specified (recommended), it is added to the address book \n"
            "so payments received with the address will be credited to 'account'.\n"
            "\nArguments:\n"
            "1. \"account\"        (string, optional) The account name for the address to be linked to. if not provided, the default account \"\" is used. It can also be set to the empty string \"\" to represent the default account. The account does not need to exist, it will be created if there is no account by the given name.\n"
            "\nResult:\n"
            "\"bitcoinaddress\"    (string) The new bitcoin address\n"
            "\nExamples:\n"
            + HelpExampleCli("getnewaddress", "")
            + HelpExampleCli("getnewaddress", "\"\"")
            + HelpExampleCli("getnewaddress", "\"myaccount\"")
            + HelpExampleRpc("getnewaddress", "\"myaccount\"")
        );

    // Parse the account first so we don't generate a key if there's an error
    string strAccount;
    if (params.size() > 0)
        strAccount = AccountFromValue(params[0]);

    if (!pwalletMain->IsLocked())
        pwalletMain->TopUpKeyPool();

    // Generate a new key that is added to wallet
    CPubKey newKey;
    if (!pwalletMain->GetKeyFromPool(newKey))
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    CKeyID keyID = newKey.GetID();

    pwalletMain->SetAddressBook(keyID, strAccount, "receive");

    return CBitcoinAddress(keyID).ToString();
}


CBitcoinAddress GetAccountAddress(string strAccount, bool bForceNew=false)
{
    CWalletDB walletdb(pwalletMain->strWalletFile);

    CAccount account;
    walletdb.ReadAccount(strAccount, account);

    bool bKeyUsed = false;

    // Check if the current key has been used
    if (account.vchPubKey.IsValid())
    {
        CScript scriptPubKey;
        scriptPubKey.SetDestination(account.vchPubKey.GetID());
        for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin();
             it != pwalletMain->mapWallet.end() && account.vchPubKey.IsValid();
             ++it)
        {
            const CWalletTx& wtx = (*it).second;
            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
                if (txout.scriptPubKey == scriptPubKey)
                    bKeyUsed = true;
        }
    }

    // Generate a new key
    if (!account.vchPubKey.IsValid() || bForceNew || bKeyUsed)
    {
        if (!pwalletMain->GetKeyFromPool(account.vchPubKey))
            throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");

        pwalletMain->SetAddressBook(account.vchPubKey.GetID(), strAccount, "receive");
        walletdb.WriteAccount(strAccount, account);
    }

    return CBitcoinAddress(account.vchPubKey.GetID());
}

Value getaccountaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getaccountaddress \"account\"\n"
            "\nReturns the current Bitcoin address for receiving payments to this account.\n"
            "\nArguments:\n"
            "1. \"account\"       (string, required) The account name for the address. It can also be set to the empty string \"\" to represent the default account. The account does not need to exist, it will be created and a new address created  if there is no account by the given name.\n"
            "\nResult:\n"
            "\"bitcoinaddress\"   (string) The account bitcoin address\n"
            "\nExamples:\n"
            + HelpExampleCli("getaccountaddress", "")
            + HelpExampleCli("getaccountaddress", "\"\"")
            + HelpExampleCli("getaccountaddress", "\"myaccount\"")
            + HelpExampleRpc("getaccountaddress", "\"myaccount\"")
        );

    // Parse the account first so we don't generate a key if there's an error
    string strAccount = AccountFromValue(params[0]);

    Value ret;

    ret = GetAccountAddress(strAccount).ToString();

    return ret;
}


Value getrawchangeaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getrawchangeaddress\n"
            "\nReturns a new Bitcoin address, for receiving change.\n"
            "This is for use with raw transactions, NOT normal use.\n"
            "\nResult:\n"
            "\"address\"    (string) The address\n"
            "\nExamples:\n"
            + HelpExampleCli("getrawchangeaddress", "")
            + HelpExampleRpc("getrawchangeaddress", "")
       );

    if (!pwalletMain->IsLocked())
        pwalletMain->TopUpKeyPool();

    CReserveKey reservekey(pwalletMain);
    CPubKey vchPubKey;
    if (!reservekey.GetReservedKey(vchPubKey))
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Unable to obtain key for change");

    reservekey.KeepKey();

    CKeyID keyID = vchPubKey.GetID();

    return CBitcoinAddress(keyID).ToString();
}


Value setaccount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "setaccount \"bitcoinaddress\" \"account\"\n"
            "\nSets the account associated with the given address.\n"
            "\nArguments:\n"
            "1. \"bitcoinaddress\"  (string, required) The bitcoin address to be associated with an account.\n"
            "2. \"account\"         (string, required) The account to assign the address to.\n"
            "\nExamples:\n"
            + HelpExampleCli("setaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"tabby\"")
            + HelpExampleRpc("setaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\", \"tabby\"")
        );

    CBitcoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");


    string strAccount;
    if (params.size() > 1)
        strAccount = AccountFromValue(params[1]);

    // Detect when changing the account of an address that is the 'unused current key' of another account:
    if (pwalletMain->mapAddressBook.count(address.Get()))
    {
        string strOldAccount = pwalletMain->mapAddressBook[address.Get()].name;
        if (address == GetAccountAddress(strOldAccount))
            GetAccountAddress(strOldAccount, true);
    }

    pwalletMain->SetAddressBook(address.Get(), strAccount, "receive");

    return Value::null;
}


Value getaccount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getaccount \"bitcoinaddress\"\n"
            "\nReturns the account associated with the given address.\n"
            "\nArguments:\n"
            "1. \"bitcoinaddress\"  (string, required) The bitcoin address for account lookup.\n"
            "\nResult:\n"
            "\"accountname\"        (string) the account address\n"
            "\nExamples:\n"
            + HelpExampleCli("getaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\"")
            + HelpExampleRpc("getaccount", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\"")
        );

    CBitcoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

    string strAccount;
    map<CTxDestination, CAddressBookData>::iterator mi = pwalletMain->mapAddressBook.find(address.Get());
    if (mi != pwalletMain->mapAddressBook.end() && !(*mi).second.name.empty())
        strAccount = (*mi).second.name;
    return strAccount;
}


Value getaddressesbyaccount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getaddressesbyaccount \"account\"\n"
            "\nReturns the list of addresses for the given account.\n"
            "\nArguments:\n"
            "1. \"account\"  (string, required) The account name.\n"
            "\nResult:\n"
            "[                     (json array of string)\n"
            "  \"bitcoinaddress\"  (string) a bitcoin address associated with the given account\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("getaddressesbyaccount", "\"tabby\"")
            + HelpExampleRpc("getaddressesbyaccount", "\"tabby\"")
        );

    string strAccount = AccountFromValue(params[0]);

    // Find all addresses that have the given account
    Array ret;
    BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, CAddressBookData)& item, pwalletMain->mapAddressBook)
    {
        const CBitcoinAddress& address = item.first;
        const string& strName = item.second.name;
        if (strName == strAccount)
            ret.push_back(address.ToString());
    }
    return ret;
}

Value sendtoaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "sendtoaddress \"bitcoinaddress\" amount ( \"comment\" \"comment-to\" )\n"
            "\nSent an amount to a given address. The amount is a real and is rounded to the nearest 0.00000001\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. \"bitcoinaddress\"  (string, required) The bitcoin address to send to.\n"
            "2. \"amount\"      (numeric, required) The amount in btc to send. eg 0.1\n"
            "3. \"comment\"     (string, optional) A comment used to store what the transaction is for. \n"
            "                             This is not part of the transaction, just kept in your wallet.\n"
            "4. \"comment-to\"  (string, optional) A comment to store the name of the person or organization \n"
            "                             to which you're sending the transaction. This is not part of the \n"
            "                             transaction, just kept in your wallet.\n"
            "\nResult:\n"
            "\"transactionid\"  (string) The transaction id. (view at https://blockchain.info/tx/[transactionid])\n"
            "\nExamples:\n"
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1")
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.1 \"donation\" \"seans outpost\"")
            + HelpExampleRpc("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.1, \"donation\", \"seans outpost\"")
        );

    CBitcoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

    // Amount
    int64_t nAmount = AmountFromValue(params[1]);

    // Wallet comments
    CWalletTx wtx;
    if (params.size() > 2 && params[2].type() != null_type && !params[2].get_str().empty())
        wtx.mapValue["comment"] = params[2].get_str();
    if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
        wtx.mapValue["to"]      = params[3].get_str();

    EnsureWalletIsUnlocked();

    string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx);
    if (strError != "")
        throw JSONRPCError(RPC_WALLET_ERROR, strError);

    return wtx.GetHash().GetHex();
}

Value listaddressgroupings(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error(
            "listaddressgroupings\n"
            "\nLists groups of addresses which have had their common ownership\n"
            "made public by common use as inputs or as the resulting change\n"
            "in past transactions\n"
            "\nResult:\n"
            "[\n"
            "  [\n"
            "    [\n"
            "      \"bitcoinaddress\",     (string) The bitcoin address\n"
            "      amount,                 (numeric) The amount in btc\n"
            "      \"account\"             (string, optional) The account\n"
            "    ]\n"
            "    ,...\n"
            "  ]\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("listaddressgroupings", "")
            + HelpExampleRpc("listaddressgroupings", "")
        );

    Array jsonGroupings;
    map<CTxDestination, int64_t> balances = pwalletMain->GetAddressBalances();
    BOOST_FOREACH(set<CTxDestination> grouping, pwalletMain->GetAddressGroupings())
    {
        Array jsonGrouping;
        BOOST_FOREACH(CTxDestination address, grouping)
        {
            Array addressInfo;
            addressInfo.push_back(CBitcoinAddress(address).ToString());
            addressInfo.push_back(ValueFromAmount(balances[address]));
            {
                LOCK(pwalletMain->cs_wallet);
                if (pwalletMain->mapAddressBook.find(CBitcoinAddress(address).Get()) != pwalletMain->mapAddressBook.end())
                    addressInfo.push_back(pwalletMain->mapAddressBook.find(CBitcoinAddress(address).Get())->second.name);
            }
            jsonGrouping.push_back(addressInfo);
        }
        jsonGroupings.push_back(jsonGrouping);
    }
    return jsonGroupings;
}

Value signmessage(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error(
            "signmessage \"bitcoinaddress\" \"message\"\n"
            "\nSign a message with the private key of an address"
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"bitcoinaddress\"  (string, required) The bitcoin address to use for the private key.\n"
            "2. \"message\"         (string, required) The message to create a signature of.\n"
            "\nResult:\n"
            "\"signature\"          (string) The signature of the message encoded in base 64\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("signmessage", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\", \"my message\"")
        );

    EnsureWalletIsUnlocked();

    string strAddress = params[0].get_str();
    string strMessage = params[1].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

Value getreceivedbyaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getreceivedbyaddress \"bitcoinaddress\" ( minconf )\n"
            "\nReturns the total amount received by the given bitcoinaddress in transactions with at least minconf confirmations.\n"
            "\nArguments:\n"
            "1. \"bitcoinaddress\"  (string, required) The bitcoin address for transactions.\n"
            "2. minconf             (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "\nResult:\n"
            "amount   (numeric) The total amount in btc received at this address.\n"
            "\nExamples:\n"
            "\nThe amount from transactions with at least 1 confirmation\n"
            + HelpExampleCli("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\"") +
            "\nThe amount including unconfirmed transactions, zero confirmations\n"
            + HelpExampleCli("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" 0") +
            "\nThe amount with at least 6 confirmation, very safe\n"
            + HelpExampleCli("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getreceivedbyaddress", "\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\", 6")
       );

    // Bitcoin address
    CBitcoinAddress address = CBitcoinAddress(params[0].get_str());
    CScript scriptPubKey;
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");
    scriptPubKey.SetDestination(address.Get());
    if (!IsMine(*pwalletMain,scriptPubKey))
        return (double)0.0;

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Tally
    int64_t nAmount = 0;
    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (wtx.IsCoinBase() || !IsFinalTx(wtx))
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            if (txout.scriptPubKey == scriptPubKey)
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
    }

    return  ValueFromAmount(nAmount);
}


Value getreceivedbyaccount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getreceivedbyaccount \"account\" ( minconf )\n"
            "\nReturns the total amount received by addresses with <account> in transactions with at least [minconf] confirmations.\n"
            "\nArguments:\n"
            "1. \"account\"      (string, required) The selected account, may be the default account using \"\".\n"
            "2. minconf          (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "\nResult:\n"
            "amount              (numeric) The total amount in btc received for this account.\n"
            "\nExamples:\n"
            "\nAmount received by the default account with at least 1 confirmation\n"
            + HelpExampleCli("getreceivedbyaccount", "\"\"") +
            "\nAmount received at the tabby account including unconfirmed amounts with zero confirmations\n"
            + HelpExampleCli("getreceivedbyaccount", "\"tabby\" 0") +
            "\nThe amount with at least 6 confirmation, very safe\n"
            + HelpExampleCli("getreceivedbyaccount", "\"tabby\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getreceivedbyaccount", "\"tabby\", 6")
        );

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Get the set of pub keys assigned to account
    string strAccount = AccountFromValue(params[0]);
    set<CTxDestination> setAddress = pwalletMain->GetAccountAddresses(strAccount);

    // Tally
    int64_t nAmount = 0;
    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (wtx.IsCoinBase() || !IsFinalTx(wtx))
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            CTxDestination address;
            if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*pwalletMain, address) && setAddress.count(address))
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
        }
    }

    return (double)nAmount / (double)COIN;
}


int64_t GetAccountBalance(CWalletDB& walletdb, const string& strAccount, int nMinDepth)
{
    int64_t nBalance = 0;

    // Tally wallet transactions
    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (!IsFinalTx(wtx) || wtx.GetBlocksToMaturity() > 0 || wtx.GetDepthInMainChain() < 0)
            continue;

        int64_t nReceived, nSent, nFee;
        wtx.GetAccountAmounts(strAccount, nReceived, nSent, nFee);
        if (nReceived != 0 && wtx.GetDepthInMainChain() >= nMinDepth)
            nBalance += nReceived;
        nBalance -= nSent + nFee;
    }

    // Tally internal accounting entries
    nBalance += walletdb.GetAccountCreditDebit(strAccount);

    return nBalance;
}

int64_t GetAccountBalance(const string& strAccount, int nMinDepth)
{
    CWalletDB walletdb(pwalletMain->strWalletFile);
    return GetAccountBalance(walletdb, strAccount, nMinDepth);
}


Value getbalance(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getbalance ( \"account\" minconf )\n"
            "\nIf account is not specified, returns the server's total available balance.\n"
            "If account is specified, returns the balance in the account.\n"
            "Note that the account \"\" is not the same as leaving the parameter out.\n"
            "The server total may be different to the balance in the default \"\" account.\n"
            "\nArguments:\n"
            "1. \"account\"      (string, optional) The selected account, or \"*\" for entire wallet. It may be the default account using \"\".\n"
            "2. minconf          (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "\nResult:\n"
            "amount              (numeric) The total amount in btc received for this account.\n"
            "\nExamples:\n"
            "\nThe total amount in the server across all accounts\n"
            + HelpExampleCli("getbalance", "") +
            "\nThe total amount in the server across all accounts, with at least 5 confirmations\n"
            + HelpExampleCli("getbalance", "\"*\" 6") +
            "\nThe total amount in the default account with at least 1 confirmation\n"
            + HelpExampleCli("getbalance", "\"\"") +
            "\nThe total amount in the account named tabby with at least 6 confirmations\n"
            + HelpExampleCli("getbalance", "\"tabby\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getbalance", "\"tabby\", 6")
        );

    if (params.size() == 0)
        return  ValueFromAmount(pwalletMain->GetBalance());

    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    if (params[0].get_str() == "*") {
        // Calculate total balance a different way from GetBalance()
        // (GetBalance() sums up all unspent TxOuts)
        // getbalance and getbalance '*' 0 should return the same number
        int64_t nBalance = 0;
        for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (!wtx.IsTrusted() || wtx.GetBlocksToMaturity() > 0)
                continue;

            int64_t allFee;
            string strSentAccount;
            list<pair<CTxDestination, int64_t> > listReceived;
            list<pair<CTxDestination, int64_t> > listSent;
            wtx.GetAmounts(listReceived, listSent, allFee, strSentAccount);
            if (wtx.GetDepthInMainChain() >= nMinDepth)
            {
                BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64_t)& r, listReceived)
                    nBalance += r.second;
            }
            BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64_t)& r, listSent)
                nBalance -= r.second;
            nBalance -= allFee;
        }
        return  ValueFromAmount(nBalance);
    }

    string strAccount = AccountFromValue(params[0]);

    int64_t nBalance = GetAccountBalance(strAccount, nMinDepth);

    return ValueFromAmount(nBalance);
}

Value getunconfirmedbalance(const Array &params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw runtime_error(
                "getunconfirmedbalance\n"
                "Returns the server's total unconfirmed balance\n");
    return ValueFromAmount(pwalletMain->GetUnconfirmedBalance());
}


Value movecmd(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 5)
        throw runtime_error(
            "move \"fromaccount\" \"toaccount\" amount ( minconf \"comment\" )\n"
            "\nMove a specified amount from one account in your wallet to another.\n"
            "\nArguments:\n"
            "1. \"fromaccount\"   (string, required) The name of the account to move funds from. May be the default account using \"\".\n"
            "2. \"toaccount\"     (string, required) The name of the account to move funds to. May be the default account using \"\".\n"
            "3. minconf           (numeric, optional, default=1) Only use funds with at least this many confirmations.\n"
            "4. \"comment\"       (string, optional) An optional comment, stored in the wallet only.\n"
            "\nResult:\n"
            "true|false           (boolean) true if successfull.\n"
            "\nExamples:\n"
            "\nMove 0.01 btc from the default account to the account named tabby\n"
            + HelpExampleCli("move", "\"\" \"tabby\" 0.01") +
            "\nMove 0.01 btc timotei to akiko with a comment and funds have 6 confirmations\n"
            + HelpExampleCli("move", "\"timotei\" \"akiko\" 0.01 6 \"happy birthday!\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("move", "\"timotei\", \"akiko\", 0.01, 6, \"happy birthday!\"")
        );

    string strFrom = AccountFromValue(params[0]);
    string strTo = AccountFromValue(params[1]);
    int64_t nAmount = AmountFromValue(params[2]);
    if (params.size() > 3)
        // unused parameter, used to be nMinDepth, keep type-checking it though
        (void)params[3].get_int();
    string strComment;
    if (params.size() > 4)
        strComment = params[4].get_str();

    CWalletDB walletdb(pwalletMain->strWalletFile);
    if (!walletdb.TxnBegin())
        throw JSONRPCError(RPC_DATABASE_ERROR, "database error");

    int64_t nNow = GetAdjustedTime();

    // Debit
    CAccountingEntry debit;
    debit.nOrderPos = pwalletMain->IncOrderPosNext(&walletdb);
    debit.strAccount = strFrom;
    debit.nCreditDebit = -nAmount;
    debit.nTime = nNow;
    debit.strOtherAccount = strTo;
    debit.strComment = strComment;
    walletdb.WriteAccountingEntry(debit);

    // Credit
    CAccountingEntry credit;
    credit.nOrderPos = pwalletMain->IncOrderPosNext(&walletdb);
    credit.strAccount = strTo;
    credit.nCreditDebit = nAmount;
    credit.nTime = nNow;
    credit.strOtherAccount = strFrom;
    credit.strComment = strComment;
    walletdb.WriteAccountingEntry(credit);

    if (!walletdb.TxnCommit())
        throw JSONRPCError(RPC_DATABASE_ERROR, "database error");

    return true;
}


Value sendfrom(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 6)
        throw runtime_error(
            "sendfrom \"fromaccount\" \"tobitcoinaddress\" amount ( minconf \"comment\" \"comment-to\" )\n"
            "\nSent an amount from an account to a bitcoin address.\n"
            "The amount is a real and is rounded to the nearest 0.00000001."
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"fromaccount\"       (string, required) The name of the account to send funds from. May be the default account using \"\".\n"
            "2. \"tobitcoinaddress\"  (string, required) The bitcoin address to send funds to.\n"
            "3. amount                (numeric, required) The amount in btc. (transaction fee is added on top).\n"
            "4. minconf               (numeric, optional, default=1) Only use funds with at least this many confirmations.\n"
            "5. \"comment\"           (string, optional) A comment used to store what the transaction is for. \n"
            "                                     This is not part of the transaction, just kept in your wallet.\n"
            "6. \"comment-to\"        (string, optional) An optional comment to store the name of the person or organization \n"
            "                                     to which you're sending the transaction. This is not part of the transaction, \n"
            "                                     it is just kept in your wallet.\n"
            "\nResult:\n"
            "\"transactionid\"        (string) The transaction id. (view at https://blockchain.info/tx/[transactionid])\n"
            "\nExamples:\n"
            "\nSend 0.01 btc from the default account to the address, must have at least 1 confirmation\n"
            + HelpExampleCli("sendfrom", "\"\" \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.01") +
            "\nSend 0.01 from the tabby account to the given address, funds must have at least 6 confirmations\n"
            + HelpExampleCli("sendfrom", "\"tabby\" \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 0.01 6 \"donation\" \"seans outpost\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("sendfrom", "\"tabby\", \"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\", 0.01, 6, \"donation\", \"seans outpost\"")
        );

    string strAccount = AccountFromValue(params[0]);
    CBitcoinAddress address(params[1].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");
    int64_t nAmount = AmountFromValue(params[2]);
    int nMinDepth = 1;
    if (params.size() > 3)
        nMinDepth = params[3].get_int();

    CWalletTx wtx;
    wtx.strFromAccount = strAccount;
    if (params.size() > 4 && params[4].type() != null_type && !params[4].get_str().empty())
        wtx.mapValue["comment"] = params[4].get_str();
    if (params.size() > 5 && params[5].type() != null_type && !params[5].get_str().empty())
        wtx.mapValue["to"]      = params[5].get_str();

    EnsureWalletIsUnlocked();

    // Check funds
    int64_t nBalance = GetAccountBalance(strAccount, nMinDepth);
    if (nAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
    string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx);
    if (strError != "")
        throw JSONRPCError(RPC_WALLET_ERROR, strError);

    return wtx.GetHash().GetHex();
}



Value getacountbyaddress(const Array& params, bool fHelp)
{
   if(fHelp || params.size() < 1)
	   throw runtime_error(
	             "sendlottobet \"fromaccount\" {\"address\":amount,...} ( minconf \"comment\" )\n"
	             "\nSend multiple times. Amounts are double-precision floating point numbers."
	             + HelpRequiringPassphrase() + "\n"
	             "\nArguments:\n"
	             "1. \"address\"         (string, required) the wallet's address\n"
	             "2. \"comment\"             (string, optional) A comment\n");

   {
         LOCK(pwalletMain->cs_wallet);
         for (map<uint256, CWalletTx>::const_iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
         {
             const uint256& wtxid = it->first;
             const CWalletTx* pcoin = &(*it).second;

             if (!IsFinalTx(*pcoin))
                 continue;

             if (!pcoin->IsTrusted())
                 continue;

             if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0)
                 continue;

             int nDepth = pcoin->GetDepthInMainChain();
             if (nDepth < 0)
                 continue;


             for (unsigned int i = 0; i < pcoin->vout.size(); i++)
             {
            	 std::string strTemp = params[0].get_str();
            	 CBitcoinAddress rewardAddress(strTemp);
            	 CKeyID rewardKey;
            	 rewardAddress.GetKeyID(rewardKey);
            	 std::vector<unsigned char> address;
            	 address.insert(address.end(), BEGIN(rewardKey), END(rewardKey));
            	 CScript scriptPubKey;

				txnouttype type;
				vector<CTxDestination> addresses;
				int nRequired;
				if(ExtractDestinations(pcoin->vout[i].scriptPubKey, type, addresses, nRequired))
				{
					std::string nwalleadres = CBitcoinAddress(addresses[0]).ToString();
					LogTrace2("getacountbyaddress","my wallet addreww:%s\n",nwalleadres);
					if( nwalleadres == strTemp)
					 {
						 //LogPrintf("the money count = %f",pcoin->vout[i].nValue);
						 return  pcoin->vout[i].nValue;//ValueFromAmount(pcoin->vout[i].nValue);
					 }
				}


             }
         }
     }
     return 0;//ValueFromAmount(0);

}

Value sendlottobet(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 5 || params.size() > 7)
        throw runtime_error(
            "sendlottobet \"fromaccount\" {\"address\":amount,...} ( minconf \"comment\" )\n"
            "\nSend multiple times. Amounts are double-precision floating point numbers."
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"fromaccount\"         (string, required) The account to send the funds from, can be \"\" for the default account\n"
            "2. amount					(numeric, required) Bet amount "
            "3. type                    (numeric, required) Bet type,0: 6 from 15(1~15), 1: 5 from 20(1~20) and 2 from 10(1~10)"
            "4. \"selectnum\"           (string, required) Selected bet number "
            "5. \"recrewardaddr\"       (string, required) receive rewards address, can be  \"\" for the default account"
            "6. minconf                 (numeric, optional, default=1) Only use the balance confirmed at least this many times.\n"
            "7. \"comment\"             (string, optional) A comment\n"

            "\nResult:\n"
            "\"transactionid\"          (string) The transaction id for the send. Only 1 transaction is created regardless of \n"
            "                                    the number of addresses. See https://blockchain.info/tx/[transactionid]\n"
            "\nExamples:\n"
            "\nSend from amounts \"tabby\" 50 cent to lotto pool, select num 010203040506, the rewardaddress from the wallet\n"
            + HelpExampleCli("sendlottobet", "\"tabby\", 50, 0, 010203040506, \"\"") +
            "\nSend from default amounts 50 cent to lotto pool, select num 0102030405FF0203, the rewardaddress is mzPyPsY3FBvGnrxdH7Zw1rcL1ZtgQHXm3n\n"
            + HelpExampleCli("sendlottobet", "\"\", 50, 1, 1617181920FF0809, mzPyPsY3FBvGnrxdH7Zw1rcL1ZtgQHXm3n")
        );

    string strAccount = AccountFromValue(params[0]);

    int nMinDepth = 1;
    if (params.size() > 5)
        nMinDepth = params[5].get_int();

    LogPrintf("Param size:%d\n", params.size());
    CWalletTx wtx;
    wtx.strFromAccount = strAccount;
    if (params.size() > 6 && params[6].type() != null_type && !params[6].get_str().empty())
        wtx.mapValue["comment"] = params[6].get_str();

	CBitcoinAddress address(GetLottoPoolAddress());
	if (!address.IsValid())
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Bitcoin address: ")+GetLottoPoolAddress());

	CScript scriptPubKey;
	CKeyID sendToPubkey;
	int ret = address.GetKeyID(sendToPubkey);
	assert(ret);

	LogPrintf("lotto bet transation!\n");
	std::vector<unsigned char> vLottoBet;
	//step1:push the lotto_type to vector
	vLottoBet.push_back((unsigned char)(params[2].get_int()));
 	//step2:push the lotto_reward  address to vector
	LogPrintf("rewardkey:%s\n", params[4].get_str());
 	if("" == params[4].get_str())
	{
		CReserveKey keyReward(pwalletMain);
		CPubKey rewardPubKey;
		bool ret;
		ret = keyReward.GetReservedKey(rewardPubKey);
		assert(ret); // should never fail, as we just unlocked
		CKeyID rewardKey = rewardPubKey.GetID();
		vLottoBet.insert(vLottoBet.end(),BEGIN(rewardKey), END(rewardKey));
		LogPrintf("rewardKey:%s\n",rewardKey.GetHex());
	}
	else
	{
		std::string strTemp = params[4].get_str();
		CBitcoinAddress rewardAddress(strTemp);
		CKeyID rewardKey;
		int ret = rewardAddress.GetKeyID(rewardKey);
		assert(ret);
		vLottoBet.insert(vLottoBet.end(), BEGIN(rewardKey), END(rewardKey));
		LogPrintf("rewardKey:%s\n",rewardKey.GetHex());
     }
 	 //step3:push the select number to vector
 	 std::string strTemp = params[3].get_str();
 	 std::vector<unsigned char> vTemp;
 	 vTemp = ParseHex(strTemp);
 	 if(params[2].get_int() == lotto::TYPE_15_6)
 	 {
 		 sort(vTemp.begin(),vTemp.end());

 	 }
 	 if(params[2].get_int() == lotto::TYPE_MULILT)
 	 {
 		std::vector<unsigned char>::iterator it = std::find(vTemp.begin(), vTemp.end(), lotto::gSplitFlag);
 		std::vector<unsigned char> ftep(vTemp.begin(),it);
 		std::vector<unsigned char> step(it+1,vTemp.end());
 		sort(ftep.begin(),ftep.end());
 		sort(step.begin(),step.end());
 		vTemp.clear();
 		vTemp.assign(ftep.begin(),ftep.end());
 		vTemp.push_back(lotto::gSplitFlag);
 		vTemp.insert(vTemp.end(),step.begin(),step.end());

 	 }
 	 vLottoBet.insert(vLottoBet.end(), vTemp.begin(), vTemp.end());

 	 scriptPubKey << OP_LOTTOUNSPEND << OP_DUP << OP_HASH160 << sendToPubkey
        			<< OP_EQUALVERIFY << OP_CHECKSIG << OP_LOTTO << vLottoBet;

 	vector<pair<CScript, int64_t> > vecSend;
	int64_t nAmount = params[1].get_int();
	nAmount *= CENT;// CTransaction::nMinTxFee;//CENT;
	LogPrintf("bet nAmount:%d \n", nAmount);
    vecSend.push_back(make_pair(scriptPubKey, nAmount));


    EnsureWalletIsUnlocked();

    // Check funds
    int64_t nBalance = GetAccountBalance(strAccount, nMinDepth);
    if (nAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
    CReserveKey keyChange(pwalletMain);
    int64_t nFeeRequired = 0;
    string strFailReason;
    bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, strFailReason);
    if (!fCreated)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, strFailReason);
    if (!pwalletMain->CommitTransaction(wtx, keyChange))
        throw JSONRPCError(RPC_WALLET_ERROR, "Transaction commit failed");

    return wtx.GetHash().GetHex();
}

Value sendmany(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "sendmany \"fromaccount\" {\"address\":amount,...} ( minconf \"comment\" )\n"
            "\nSend multiple times. Amounts are double-precision floating point numbers."
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"fromaccount\"         (string, required) The account to send the funds from, can be \"\" for the default account\n"
            "2. \"amounts\"             (string, required) A json object with addresses and amounts\n"
            "    {\n"
            "      \"address\":amount   (numeric) The bitcoin address is the key, the numeric amount in btc is the value\n"
            "      ,...\n"
            "    }\n"
            "3. minconf                 (numeric, optional, default=1) Only use the balance confirmed at least this many times.\n"
            "4. \"comment\"             (string, optional) A comment\n"
            "\nResult:\n"
            "\"transactionid\"          (string) The transaction id for the send. Only 1 transaction is created regardless of \n"
            "                                    the number of addresses. See https://blockchain.info/tx/[transactionid]\n"
            "\nExamples:\n"
            "\nSend two amounts to two different addresses:\n"
            + HelpExampleCli("sendmany", "\"tabby\" \"{\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\\\":0.01,\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\":0.02}\"") +
            "\nSend two amounts to two different addresses setting the confirmation and comment:\n"
            + HelpExampleCli("sendmany", "\"tabby\" \"{\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\\\":0.01,\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\":0.02}\" 6 \"testing\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("sendmany", "\"tabby\", \"{\\\"1D1ZrZNe3JUo7ZycKEYQQiQAWd9y54F4XZ\\\":0.01,\\\"1353tsE8YMTA4EuV7dgUXGjNFf9KpVvKHz\\\":0.02}\", 6, \"testing\"")
        );

    string strAccount = AccountFromValue(params[0]);
    Object sendTo = params[1].get_obj();
    int nMinDepth = 1;
    if (params.size() > 2)
        nMinDepth = params[2].get_int();

    CWalletTx wtx;
    wtx.strFromAccount = strAccount;
    if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
        wtx.mapValue["comment"] = params[3].get_str();

    set<CBitcoinAddress> setAddress;
    vector<pair<CScript, int64_t> > vecSend;

    int64_t totalAmount = 0;
    BOOST_FOREACH(const Pair& s, sendTo)
    {
        CBitcoinAddress address(s.name_);
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Bitcoin address: ")+s.name_);

        if (setAddress.count(address))
            throw JSONRPCError(RPC_INVALID_PARAMETER, string("Invalid parameter, duplicated address: ")+s.name_);
        setAddress.insert(address);

        CScript scriptPubKey;
        scriptPubKey.SetDestination(address.Get());
        int64_t nAmount = AmountFromValue(s.value_);
        totalAmount += nAmount;

        vecSend.push_back(make_pair(scriptPubKey, nAmount));
    }

    EnsureWalletIsUnlocked();

    // Check funds
    int64_t nBalance = GetAccountBalance(strAccount, nMinDepth);
    if (totalAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
    CReserveKey keyChange(pwalletMain);
    int64_t nFeeRequired = 0;
    string strFailReason;
    bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, strFailReason);
    if (!fCreated)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, strFailReason);
    if (!pwalletMain->CommitTransaction(wtx, keyChange))
        throw JSONRPCError(RPC_WALLET_ERROR, "Transaction commit failed");

    return wtx.GetHash().GetHex();
}

// Defined in rpcmisc.cpp
extern CScript _createmultisig(const Array& params);

Value addmultisigaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        string msg = "addmultisigaddress nrequired [\"key\",...] ( \"account\" )\n"
            "\nAdd a nrequired-to-sign multisignature address to the wallet.\n"
            "Each key is a Bitcoin address or hex-encoded public key.\n"
            "If 'account' is specified, assign address to that account.\n"

            "\nArguments:\n"
            "1. nrequired        (numeric, required) The number of required signatures out of the n keys or addresses.\n"
            "2. \"keysobject\"   (string, required) A json array of bitcoin addresses or hex-encoded public keys\n"
            "     [\n"
            "       \"address\"  (string) bitcoin address or hex-encoded public key\n"
            "       ...,\n"
            "     ]\n"
            "3. \"account\"      (string, optional) An account to assign the addresses to.\n"

            "\nResult:\n"
            "\"bitcoinaddress\"  (string) A bitcoin address associated with the keys.\n"

            "\nExamples:\n"
            "\nAdd a multisig address from 2 addresses\n"
            + HelpExampleCli("addmultisigaddress", "2 \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("addmultisigaddress", "2, \"[\\\"16sSauSf5pF2UkUwvKGq4qjNRzBZYqgEL5\\\",\\\"171sgjn4YtPu27adkKGrdDwzRTxnRkBfKV\\\"]\"")
        ;
        throw runtime_error(msg);
    }

    string strAccount;
    if (params.size() > 2)
        strAccount = AccountFromValue(params[2]);

    // Construct using pay-to-script-hash:
    CScript inner = _createmultisig(params);
    CScriptID innerID = inner.GetID();
    pwalletMain->AddCScript(inner);

    pwalletMain->SetAddressBook(innerID, strAccount, "send");
    return CBitcoinAddress(innerID).ToString();
}


struct tallyitem
{
    int64_t nAmount;
    int nConf;
    vector<uint256> txids;
    tallyitem()
    {
        nAmount = 0;
        nConf = std::numeric_limits<int>::max();
    }
};

Value ListReceived(const Array& params, bool fByAccounts)
{
    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    // Whether to include empty accounts
    bool fIncludeEmpty = false;
    if (params.size() > 1)
        fIncludeEmpty = params[1].get_bool();

    // Tally
    map<CBitcoinAddress, tallyitem> mapTally;
    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;

        if (wtx.IsCoinBase() || !IsFinalTx(wtx))
            continue;

        int nDepth = wtx.GetDepthInMainChain();
        if (nDepth < nMinDepth)
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            CTxDestination address;
            if (!ExtractDestination(txout.scriptPubKey, address) || !IsMine(*pwalletMain, address))
                continue;

            tallyitem& item = mapTally[address];
            item.nAmount += txout.nValue;
            item.nConf = min(item.nConf, nDepth);
            item.txids.push_back(wtx.GetHash());
        }
    }

    // Reply
    Array ret;
    map<string, tallyitem> mapAccountTally;
    BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, CAddressBookData)& item, pwalletMain->mapAddressBook)
    {
        const CBitcoinAddress& address = item.first;
        const string& strAccount = item.second.name;
        map<CBitcoinAddress, tallyitem>::iterator it = mapTally.find(address);
        if (it == mapTally.end() && !fIncludeEmpty)
            continue;

        int64_t nAmount = 0;
        int nConf = std::numeric_limits<int>::max();
        if (it != mapTally.end())
        {
            nAmount = (*it).second.nAmount;
            nConf = (*it).second.nConf;
        }

        if (fByAccounts)
        {
            tallyitem& item = mapAccountTally[strAccount];
            item.nAmount += nAmount;
            item.nConf = min(item.nConf, nConf);
        }
        else
        {
            Object obj;
            obj.push_back(Pair("address",       address.ToString()));
            obj.push_back(Pair("account",       strAccount));
            obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            Array transactions;
            if (it != mapTally.end())
            {
                BOOST_FOREACH(const uint256& item, (*it).second.txids)
                {
                    transactions.push_back(item.GetHex());
                }
            }
            obj.push_back(Pair("txids", transactions));
            ret.push_back(obj);
        }
    }

    if (fByAccounts)
    {
        for (map<string, tallyitem>::iterator it = mapAccountTally.begin(); it != mapAccountTally.end(); ++it)
        {
            int64_t nAmount = (*it).second.nAmount;
            int nConf = (*it).second.nConf;
            Object obj;
            obj.push_back(Pair("account",       (*it).first));
            obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            ret.push_back(obj);
        }
    }

    return ret;
}

Value listreceivedbyaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "listreceivedbyaddress ( minconf includeempty )\n"
            "\nList balances by receiving address.\n"
            "\nArguments:\n"
            "1. minconf       (numeric, optional, default=1) The minimum number of confirmations before payments are included.\n"
            "2. includeempty  (numeric, optional, dafault=false) Whether to include addresses that haven't received any payments.\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"address\" : \"receivingaddress\",  (string) The receiving address\n"
            "    \"account\" : \"accountname\",       (string) The account of the receiving address. The default account is \"\".\n"
            "    \"amount\" : x.xxx,                  (numeric) The total amount in btc received by the address\n"
            "    \"confirmations\" : n                (numeric) The number of confirmations of the most recent transaction included\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n"
            + HelpExampleCli("listreceivedbyaddress", "")
            + HelpExampleCli("listreceivedbyaddress", "6 true")
            + HelpExampleRpc("listreceivedbyaddress", "6, true")
        );

    return ListReceived(params, false);
}

Value listreceivedbyaccount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "listreceivedbyaccount ( minconf includeempty )\n"
            "\nList balances by account.\n"
            "\nArguments:\n"
            "1. minconf      (numeric, optional, default=1) The minimum number of confirmations before payments are included.\n"
            "2. includeempty (boolean, optional, default=false) Whether to include accounts that haven't received any payments.\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"account\" : \"accountname\",  (string) The account name of the receiving account\n"
            "    \"amount\" : x.xxx,             (numeric) The total amount received by addresses with this account\n"
            "    \"confirmations\" : n           (numeric) The number of confirmations of the most recent transaction included\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n"
            + HelpExampleCli("listreceivedbyaccount", "")
            + HelpExampleCli("listreceivedbyaccount", "6 true")
            + HelpExampleRpc("listreceivedbyaccount", "6, true")
        );

    return ListReceived(params, true);
}

static void MaybePushAddress(Object & entry, const CTxDestination &dest)
{
    CBitcoinAddress addr;
    if (addr.Set(dest))
        entry.push_back(Pair("address", addr.ToString()));
}

void ListTransactions(const CWalletTx& wtx, const string& strAccount, int nMinDepth, bool fLong, Array& ret)
{
    int64_t nFee;
    string strSentAccount;
    list<pair<CTxDestination, int64_t> > listReceived;
    list<pair<CTxDestination, int64_t> > listSent;

    wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount);

    bool fAllAccounts = (strAccount == string("*"));

    // Sent
    if ((!listSent.empty() || nFee != 0) && (fAllAccounts || strAccount == strSentAccount))
    {
        BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& s, listSent)
        {
            Object entry;
            entry.push_back(Pair("account", strSentAccount));
            MaybePushAddress(entry, s.first);
            entry.push_back(Pair("category", "send"));
            entry.push_back(Pair("amount", ValueFromAmount(-s.second)));
            entry.push_back(Pair("fee", ValueFromAmount(-nFee)));
            if (fLong)
                WalletTxToJSON(wtx, entry);
            ret.push_back(entry);
        }
    }

    // Received
    if (listReceived.size() > 0 && wtx.GetDepthInMainChain() >= nMinDepth)
    {
        BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& r, listReceived)
        {
            string account;
            if (pwalletMain->mapAddressBook.count(r.first))
                account = pwalletMain->mapAddressBook[r.first].name;
            if (fAllAccounts || (account == strAccount))
            {
                Object entry;
                entry.push_back(Pair("account", account));
                MaybePushAddress(entry, r.first);
                if (wtx.IsCoinBase())
                {
                    if (wtx.GetDepthInMainChain() < 1)
                        entry.push_back(Pair("category", "orphan"));
                    else if (wtx.GetBlocksToMaturity() > 0)
                        entry.push_back(Pair("category", "immature"));
                    else
                        entry.push_back(Pair("category", "generate"));
                }
                else
                {
                    entry.push_back(Pair("category", "receive"));
                }
                entry.push_back(Pair("amount", ValueFromAmount(r.second)));
                if (fLong)
                    WalletTxToJSON(wtx, entry);
                ret.push_back(entry);
            }
        }
    }
}

void AcentryToJSON(const CAccountingEntry& acentry, const string& strAccount, Array& ret)
{
    bool fAllAccounts = (strAccount == string("*"));

    if (fAllAccounts || acentry.strAccount == strAccount)
    {
        Object entry;
        entry.push_back(Pair("account", acentry.strAccount));
        entry.push_back(Pair("category", "move"));
        entry.push_back(Pair("time", (boost::int64_t)acentry.nTime));
        entry.push_back(Pair("amount", ValueFromAmount(acentry.nCreditDebit)));
        entry.push_back(Pair("otheraccount", acentry.strOtherAccount));
        entry.push_back(Pair("comment", acentry.strComment));
        ret.push_back(entry);
    }
}

Value listtransactions(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
            "listtransactions ( \"account\" count from )\n"
            "\nReturns up to 'count' most recent transactions skipping the first 'from' transactions for account 'account'.\n"
            "\nArguments:\n"
            "1. \"account\"    (string, optional) The account name. If not included, it will list all transactions for all accounts.\n"
            "                                     If \"\" is set, it will list transactions for the default account.\n"
            "2. count          (numeric, optional, default=10) The number of transactions to return\n"
            "3. from           (numeric, optional, default=0) The number of transactions to skip\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"account\":\"accountname\",       (string) The account name associated with the transaction. \n"
            "                                                It will be \"\" for the default account.\n"
            "    \"address\":\"bitcoinaddress\",    (string) The bitcoin address of the transaction. Not present for \n"
            "                                                move transactions (category = move).\n"
            "    \"category\":\"send|receive|move\", (string) The transaction category. 'move' is a local (off blockchain)\n"
            "                                                transaction between accounts, and not associated with an address,\n"
            "                                                transaction id or block. 'send' and 'receive' transactions are \n"
            "                                                associated with an address, transaction id and block details\n"
            "    \"amount\": x.xxx,          (numeric) The amount in btc. This is negative for the 'send' category, and for the\n"
            "                                         'move' category for moves outbound. It is positive for the 'receive' category,\n"
            "                                         and for the 'move' category for inbound funds.\n"
            "    \"fee\": x.xxx,             (numeric) The amount of the fee in btc. This is negative and only available for the \n"
            "                                         'send' category of transactions.\n"
            "    \"confirmations\": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and \n"
            "                                         'receive' category of transactions.\n"
            "    \"blockhash\": \"hashvalue\", (string) The block hash containing the transaction. Available for 'send' and 'receive'\n"
            "                                          category of transactions.\n"
            "    \"blockindex\": n,          (numeric) The block index containing the transaction. Available for 'send' and 'receive'\n"
            "                                          category of transactions.\n"
            "    \"txid\": \"transactionid\", (string) The transaction id (see https://blockchain.info/tx/[transactionid]. Available \n"
            "                                          for 'send' and 'receive' category of transactions.\n"
            "    \"time\": xxx,              (numeric) The transaction time in seconds since epoch (midnight Jan 1 1970 GMT).\n"
            "    \"timereceived\": xxx,      (numeric) The time received in seconds since epoch (midnight Jan 1 1970 GMT). Available \n"
            "                                          for 'send' and 'receive' category of transactions.\n"
            "    \"comment\": \"...\",       (string) If a comment is associated with the transaction.\n"
            "    \"otheraccount\": \"accountname\",  (string) For the 'move' category of transactions, the account the funds came \n"
            "                                          from (for receiving funds, positive amounts), or went to (for sending funds,\n"
            "                                          negative amounts).\n"
            "  }\n"
            "]\n"

            "\nExamples:\n"
            "\nList the most recent 10 transactions in the systems\n"
            + HelpExampleCli("listtransactions", "") +
            "\nList the most recent 10 transactions for the tabby account\n"
            + HelpExampleCli("listtransactions", "\"tabby\"") +
            "\nList transactions 100 to 120 from the tabby account\n"
            + HelpExampleCli("listtransactions", "\"tabby\" 20 100") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("listtransactions", "\"tabby\", 20, 100")
        );

    string strAccount = "*";
    if (params.size() > 0)
        strAccount = params[0].get_str();
    int nCount = 10;
    if (params.size() > 1)
        nCount = params[1].get_int();
    int nFrom = 0;
    if (params.size() > 2)
        nFrom = params[2].get_int();

    if (nCount < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
    if (nFrom < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");

    Array ret;

    std::list<CAccountingEntry> acentries;
    CWallet::TxItems txOrdered = pwalletMain->OrderedTxItems(acentries, strAccount);

    // iterate backwards until we have nCount items to return:
    for (CWallet::TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
    {
        CWalletTx *const pwtx = (*it).second.first;
        if (pwtx != 0)
            ListTransactions(*pwtx, strAccount, 0, true, ret);
        CAccountingEntry *const pacentry = (*it).second.second;
        if (pacentry != 0)
            AcentryToJSON(*pacentry, strAccount, ret);

        if ((int)ret.size() >= (nCount+nFrom)) break;
    }
    // ret is newest to oldest

    if (nFrom > (int)ret.size())
        nFrom = ret.size();
    if ((nFrom + nCount) > (int)ret.size())
        nCount = ret.size() - nFrom;
    Array::iterator first = ret.begin();
    std::advance(first, nFrom);
    Array::iterator last = ret.begin();
    std::advance(last, nFrom+nCount);

    if (last != ret.end()) ret.erase(last, ret.end());
    if (first != ret.begin()) ret.erase(ret.begin(), first);

    std::reverse(ret.begin(), ret.end()); // Return oldest to newest

    return ret;
}

Value listaccounts(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "listaccounts ( minconf )\n"
            "\nReturns Object that has account names as keys, account balances as values.\n"
            "\nArguments:\n"
            "1. minconf     (numeric, optional, default=1) Only onclude transactions with at least this many confirmations\n"
            "\nResult:\n"
            "{                      (json object where keys are account names, and values are numeric balances\n"
            "  \"account\": x.xxx,  (numeric) The property name is the account name, and the value is the total balance for the account.\n"
            "  ...\n"
            "}\n"
            "\nExamples:\n"
            "\nList account balances where there at least 1 confirmation\n"
            + HelpExampleCli("listaccounts", "") +
            "\nList account balances including zero confirmation transactions\n"
            + HelpExampleCli("listaccounts", "0") +
            "\nList account balances for 6 or more confirmations\n"
            + HelpExampleCli("listaccounts", "6") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("listaccounts", "6")
        );

    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    map<string, int64_t> mapAccountBalances;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, CAddressBookData)& entry, pwalletMain->mapAddressBook) {
        if (IsMine(*pwalletMain, entry.first)) // This address belongs to me
            mapAccountBalances[entry.second.name] = 0;
    }

    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        int64_t nFee;
        string strSentAccount;
        list<pair<CTxDestination, int64_t> > listReceived;
        list<pair<CTxDestination, int64_t> > listSent;
        int nDepth = wtx.GetDepthInMainChain();
        if (wtx.GetBlocksToMaturity() > 0 || nDepth < 0)
            continue;
        wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount);
        mapAccountBalances[strSentAccount] -= nFee;
        BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& s, listSent)
            mapAccountBalances[strSentAccount] -= s.second;
        if (nDepth >= nMinDepth)
        {
            BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& r, listReceived)
                if (pwalletMain->mapAddressBook.count(r.first))
                    mapAccountBalances[pwalletMain->mapAddressBook[r.first].name] += r.second;
                else
                    mapAccountBalances[""] += r.second;
        }
    }

    list<CAccountingEntry> acentries;
    CWalletDB(pwalletMain->strWalletFile).ListAccountCreditDebit("*", acentries);
    BOOST_FOREACH(const CAccountingEntry& entry, acentries)
        mapAccountBalances[entry.strAccount] += entry.nCreditDebit;

    Object ret;
    BOOST_FOREACH(const PAIRTYPE(string, int64_t)& accountBalance, mapAccountBalances) {
        ret.push_back(Pair(accountBalance.first, ValueFromAmount(accountBalance.second)));
    }
    return ret;
}

Value listsinceblock(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error(
            "listsinceblock ( \"blockhash\" target-confirmations )\n"
            "\nGet all transactions in blocks since block [blockhash], or all transactions if omitted\n"
            "\nArguments:\n"
            "1. \"blockhash\"   (string, optional) The block hash to list transactions since\n"
            "2. target-confirmations:    (numeric, optional) The confirmations required, must be 1 or more\n"
            "\nResult:\n"
            "{\n"
            "  \"transactions\": [\n"
            "    \"account\":\"accountname\",       (string) The account name associated with the transaction. Will be \"\" for the default account.\n"
            "    \"address\":\"bitcoinaddress\",    (string) The bitcoin address of the transaction. Not present for move transactions (category = move).\n"
            "    \"category\":\"send|receive\",     (string) The transaction category. 'send' has negative amounts, 'receive' has positive amounts.\n"
            "    \"amount\": x.xxx,          (numeric) The amount in btc. This is negative for the 'send' category, and for the 'move' category for moves \n"
            "                                          outbound. It is positive for the 'receive' category, and for the 'move' category for inbound funds.\n"
            "    \"fee\": x.xxx,             (numeric) The amount of the fee in btc. This is negative and only available for the 'send' category of transactions.\n"
            "    \"confirmations\": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blockhash\": \"hashvalue\",     (string) The block hash containing the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blockindex\": n,          (numeric) The block index containing the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blocktime\": xxx,         (numeric) The block time in seconds since epoch (1 Jan 1970 GMT).\n"
            "    \"txid\": \"transactionid\",  (string) The transaction id (see https://blockchain.info/tx/[transactionid]. Available for 'send' and 'receive' category of transactions.\n"
            "    \"time\": xxx,              (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT).\n"
            "    \"timereceived\": xxx,      (numeric) The time received in seconds since epoch (Jan 1 1970 GMT). Available for 'send' and 'receive' category of transactions.\n"
            "    \"comment\": \"...\",       (string) If a comment is associated with the transaction.\n"
            "    \"to\": \"...\",            (string) If a comment to is associated with the transaction.\n"
             "  ],\n"
            "  \"lastblock\": \"lastblockhash\"     (string) The hash of the last block\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("listsinceblock", "")
            + HelpExampleCli("listsinceblock", "\"000000000000000bacf66f7497b7dc45ef753ee9a7d38571037cdb1a57f663ad\" 6")
            + HelpExampleRpc("listsinceblock", "\"000000000000000bacf66f7497b7dc45ef753ee9a7d38571037cdb1a57f663ad\", 6")
        );

    CBlockIndex *pindex = NULL;
    int target_confirms = 1;

    if (params.size() > 0)
    {
        uint256 blockId = 0;

        blockId.SetHex(params[0].get_str());
        std::map<uint256, CBlockIndex*>::iterator it = mapBlockIndex.find(blockId);
        if (it != mapBlockIndex.end())
            pindex = it->second;
    }

    if (params.size() > 1)
    {
        target_confirms = params[1].get_int();

        if (target_confirms < 1)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
    }

    int depth = pindex ? (1 + chainActive.Height() - pindex->nHeight) : -1;

    Array transactions;

    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); it++)
    {
        CWalletTx tx = (*it).second;

        if (depth == -1 || tx.GetDepthInMainChain() < depth)
            ListTransactions(tx, "*", 0, true, transactions);
    }

    CBlockIndex *pblockLast = chainActive[chainActive.Height() + 1 - target_confirms];
    uint256 lastblock = pblockLast ? pblockLast->GetBlockHash() : 0;

    Object ret;
    ret.push_back(Pair("transactions", transactions));
    ret.push_back(Pair("lastblock", lastblock.GetHex()));

    return ret;
}

Value gettransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "gettransaction \"txid\"\n"
            "\nGet detailed information about in-wallet transaction <txid>\n"
            "\nArguments:\n"
            "1. \"txid\"    (string, required) The transaction id\n"
            "\nResult:\n"
            "{\n"
            "  \"amount\" : x.xxx,        (numeric) The transaction amount in btc\n"
            "  \"confirmations\" : n,     (numeric) The number of confirmations\n"
            "  \"blockhash\" : \"hash\",  (string) The block hash\n"
            "  \"blockindex\" : xx,       (numeric) The block index\n"
            "  \"blocktime\" : ttt,       (numeric) The time in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"txid\" : \"transactionid\",   (string) The transaction id, see also https://blockchain.info/tx/[transactionid]\n"
            "  \"time\" : ttt,            (numeric) The transaction time in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"timereceived\" : ttt,    (numeric) The time received in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"details\" : [\n"
            "    {\n"
            "      \"account\" : \"accountname\",  (string) The account name involved in the transaction, can be \"\" for the default account.\n"
            "      \"address\" : \"bitcoinaddress\",   (string) The bitcoin address involved in the transaction\n"
            "      \"category\" : \"send|receive\",    (string) The category, either 'send' or 'receive'\n"
            "      \"amount\" : x.xxx                  (numeric) The amount in btc\n"
            "    }\n"
            "    ,...\n"
            "  ],\n"
            "  \"hex\" : \"data\"         (string) Raw data for transaction\n"
            "}\n"

            "\nbExamples\n"
            + HelpExampleCli("gettransaction", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\"")
            + HelpExampleRpc("gettransaction", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\"")
        );

    uint256 hash;
    hash.SetHex(params[0].get_str());

    Object entry;
    if (!pwalletMain->mapWallet.count(hash))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid or non-wallet transaction id");
    const CWalletTx& wtx = pwalletMain->mapWallet[hash];

    int64_t nCredit = wtx.GetCredit();
    int64_t nDebit = wtx.GetDebit();
    int64_t nNet = nCredit - nDebit;
    int64_t nFee = (wtx.IsFromMe() ? wtx.GetValueOut() - nDebit : 0);

    entry.push_back(Pair("amount", ValueFromAmount(nNet - nFee)));
    if (wtx.IsFromMe())
        entry.push_back(Pair("fee", ValueFromAmount(nFee)));

    WalletTxToJSON(wtx, entry);

    Array details;
    ListTransactions(wtx, "*", 0, false, details);
    entry.push_back(Pair("details", details));

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << static_cast<CTransaction>(wtx);
    string strHex = HexStr(ssTx.begin(), ssTx.end());
    entry.push_back(Pair("hex", strHex));

    return entry;
}


Value backupwallet(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "backupwallet \"destination\"\n"
            "\nSafely copies wallet.dat to destination, which can be a directory or a path with filename.\n"
            "\nArguments:\n"
            "1. \"destination\"   (string) The destination directory or file\n"
            "\nExamples:\n"
            + HelpExampleCli("backupwallet", "\"backup.dat\"")
            + HelpExampleRpc("backupwallet", "\"backup.dat\"")
        );

    string strDest = params[0].get_str();
    if (!BackupWallet(*pwalletMain, strDest))
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");

    return Value::null;
}


Value keypoolrefill(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "keypoolrefill ( newsize )\n"
            "\nFills the keypool."
            + HelpRequiringPassphrase() + "\n"
            "\nArguments\n"
            "1. newsize     (numeric, optional, default=100) The new keypool size\n"
            "\nExamples:\n"
            + HelpExampleCli("keypoolrefill", "")
            + HelpExampleRpc("keypoolrefill", "")
        );

    // 0 is interpreted by TopUpKeyPool() as the default keypool size given by -keypool
    unsigned int kpSize = 0;
    if (params.size() > 0) {
        if (params[0].get_int() < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected valid size.");
        kpSize = (unsigned int)params[0].get_int();
    }

    EnsureWalletIsUnlocked();
    pwalletMain->TopUpKeyPool(kpSize);

    if (pwalletMain->GetKeyPoolSize() < kpSize)
        throw JSONRPCError(RPC_WALLET_ERROR, "Error refreshing keypool.");

    return Value::null;
}


static void LockWallet(CWallet* pWallet)
{
    LOCK(cs_nWalletUnlockTime);
    nWalletUnlockTime = 0;
    pWallet->Lock();
}

Value walletpassphrase(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
        throw runtime_error(
            "walletpassphrase \"passphrase\" timeout\n"
            "\nStores the wallet decryption key in memory for 'timeout' seconds.\n"
            "This is needed prior to performing transactions related to private keys such as sending bitcoins\n"
            "\nArguments:\n"
            "1. \"passphrase\"     (string, required) The wallet passphrase\n"
            "2. timeout            (numeric, required) The time to keep the decryption key in seconds.\n"
            "\nNote:\n"
            "Issuing the walletpassphrase command while the wallet is already unlocked will set a new unlock\n"
            "time that overrides the old one.\n"
            "\nExamples:\n"
            "\nunlock the wallet for 60 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 60") +
            "\nLock the wallet again (before 60 seconds)\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletpassphrase", "\"my pass phrase\", 60")
        );

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrase was called.");

    // Note that the walletpassphrase is stored in params[0] which is not mlock()ed
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    strWalletPass = params[0].get_str().c_str();

    if (strWalletPass.length() > 0)
    {
        if (!pwalletMain->Unlock(strWalletPass))
            throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    }
    else
        throw runtime_error(
            "walletpassphrase <passphrase> <timeout>\n"
            "Stores the wallet decryption key in memory for <timeout> seconds.");

    pwalletMain->TopUpKeyPool();

    int64_t nSleepTime = params[1].get_int64();
    LOCK(cs_nWalletUnlockTime);
    nWalletUnlockTime = GetTime() + nSleepTime;
    RPCRunLater("lockwallet", boost::bind(LockWallet, pwalletMain), nSleepTime);

    return Value::null;
}


Value walletpassphrasechange(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
        throw runtime_error(
            "walletpassphrasechange \"oldpassphrase\" \"newpassphrase\"\n"
            "\nChanges the wallet passphrase from 'oldpassphrase' to 'newpassphrase'.\n"
            "\nArguments:\n"
            "1. \"oldpassphrase\"      (string) The current passphrase\n"
            "2. \"newpassphrase\"      (string) The new passphrase\n"
            "\nExamples:\n"
            + HelpExampleCli("walletpassphrasechange", "\"old one\" \"new one\"")
            + HelpExampleRpc("walletpassphrasechange", "\"old one\", \"new one\"")
        );

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrasechange was called.");

    // TODO: get rid of these .c_str() calls by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strOldWalletPass;
    strOldWalletPass.reserve(100);
    strOldWalletPass = params[0].get_str().c_str();

    SecureString strNewWalletPass;
    strNewWalletPass.reserve(100);
    strNewWalletPass = params[1].get_str().c_str();

    if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1)
        throw runtime_error(
            "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
            "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");

    if (!pwalletMain->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass))
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");

    return Value::null;
}


Value walletlock(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 0))
        throw runtime_error(
            "walletlock\n"
            "\nRemoves the wallet encryption key from memory, locking the wallet.\n"
            "After calling this method, you will need to call walletpassphrase again\n"
            "before being able to call any methods which require the wallet to be unlocked.\n"
            "\nExamples:\n"
            "\nSet the passphrase for 2 minutes to perform a transaction\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 120") +
            "\nPerform a send (requires passphrase set)\n"
            + HelpExampleCli("sendtoaddress", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\" 1.0") +
            "\nClear the passphrase since we are done before 2 minutes is up\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletlock", "")
        );

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletlock was called.");

    {
        LOCK(cs_nWalletUnlockTime);
        pwalletMain->Lock();
        nWalletUnlockTime = 0;
    }

    return Value::null;
}


Value encryptwallet(const Array& params, bool fHelp)
{
    if (!pwalletMain->IsCrypted() && (fHelp || params.size() != 1))
        throw runtime_error(
            "encryptwallet \"passphrase\"\n"
            "\nEncrypts the wallet with 'passphrase'. This is for first time encryption.\n"
            "After this, any calls that interact with private keys such as sending or signing \n"
            "will require the passphrase to be set prior the making these calls.\n"
            "Use the walletpassphrase call for this, and then walletlock call.\n"
            "If the wallet is already encrypted, use the walletpassphrasechange call.\n"
            "Note that this will shutdown the server.\n"
            "\nArguments:\n"
            "1. \"passphrase\"    (string) The pass phrase to encrypt the wallet with. It must be at least 1 character, but should be long.\n"
            "\nExamples:\n"
            "\nEncrypt you wallet\n"
            + HelpExampleCli("encryptwallet", "\"my pass phrase\"") +
            "\nNow set the passphrase to use the wallet, such as for signing or sending bitcoin\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\"") +
            "\nNow we can so something like sign\n"
            + HelpExampleCli("signmessage", "\"bitcoinaddress\" \"test message\"") +
            "\nNow lock the wallet again by removing the passphrase\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("encryptwallet", "\"my pass phrase\"")
        );

    if (fHelp)
        return true;
    if (pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an encrypted wallet, but encryptwallet was called.");

    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[0].get_str().c_str();

    if (strWalletPass.length() < 1)
        throw runtime_error(
            "encryptwallet <passphrase>\n"
            "Encrypts the wallet with <passphrase>.");

    if (!pwalletMain->EncryptWallet(strWalletPass))
        throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");

    // BDB seems to have a bad habit of writing old data into
    // slack space in .dat files; that is bad if the old data is
    // unencrypted private keys. So:
    StartShutdown();
    return "wallet encrypted; Bitcoin server stopping, restart to run with encrypted wallet. The keypool has been flushed, you need to make a new backup.";
}

Value lockunspent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "lockunspent unlock [{\"txid\":\"txid\",\"vout\":n},...]\n"
            "\nUpdates list of temporarily unspendable outputs.\n"
            "Temporarily lock (lock=true) or unlock (lock=false) specified transaction outputs.\n"
            "A locked transaction output will not be chosen by automatic coin selection, when spending bitcoins.\n"
            "Locks are stored in memory only. Nodes start with zero locked outputs, and the locked output list\n"
            "is always cleared (by virtue of process exit) when a node stops or fails.\n"
            "Also see the listunspent call\n"
            "\nArguments:\n"
            "1. unlock            (boolean, required) Whether to unlock (true) or lock (false) the specified transactions\n"
            "2. \"transactions\"  (string, required) A json array of objects. Each object the txid (string) vout (numeric)\n"
            "     [           (json array of json objects)\n"
            "       {\n"
            "         \"txid\":\"id\",    (string) The transaction id\n"
            "         \"vout\": n         (numeric) The output number\n"
            "       }\n"
            "       ,...\n"
            "     ]\n"

            "\nResult:\n"
            "true|false    (boolean) Whether the command was successful or not\n"

            "\nExamples:\n"
            "\nList the unspent transactions\n"
            + HelpExampleCli("listunspent", "") +
            "\nLock an unspent transaction\n"
            + HelpExampleCli("lockunspent", "false \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nList the locked transactions\n"
            + HelpExampleCli("listlockunspent", "") +
            "\nUnlock the transaction again\n"
            + HelpExampleCli("lockunspent", "true \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("lockunspent", "false, \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"")
        );

    if (params.size() == 1)
        RPCTypeCheck(params, list_of(bool_type));
    else
        RPCTypeCheck(params, list_of(bool_type)(array_type));

    bool fUnlock = params[0].get_bool();

    if (params.size() == 1) {
        if (fUnlock)
            pwalletMain->UnlockAllCoins();
        return true;
    }

    Array outputs = params[1].get_array();
    BOOST_FOREACH(Value& output, outputs)
    {
        if (output.type() != obj_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected object");
        const Object& o = output.get_obj();

        RPCTypeCheck(o, map_list_of("txid", str_type)("vout", int_type));

        string txid = find_value(o, "txid").get_str();
        if (!IsHex(txid))
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected hex txid");

        int nOutput = find_value(o, "vout").get_int();
        if (nOutput < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");

        COutPoint outpt(uint256(txid), nOutput);

        if (fUnlock)
            pwalletMain->UnlockCoin(outpt);
        else
            pwalletMain->LockCoin(outpt);
    }

    return true;
}

Value listlockunspent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw runtime_error(
            "listlockunspent\n"
            "\nReturns list of temporarily unspendable outputs.\n"
            "See the lockunspent call to lock and unlock transactions for spending.\n"
            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"txid\" : \"transactionid\",     (string) The transaction id locked\n"
            "    \"vout\" : n                      (numeric) The vout value\n"
            "  }\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            "\nList the unspent transactions\n"
            + HelpExampleCli("listunspent", "") +
            "\nLock an unspent transaction\n"
            + HelpExampleCli("lockunspent", "false \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nList the locked transactions\n"
            + HelpExampleCli("listlockunspent", "") +
            "\nUnlock the transaction again\n"
            + HelpExampleCli("lockunspent", "true \"[{\\\"txid\\\":\\\"a08e6907dbbd3d809776dbfc5d82e371b764ed838b5655e72f463568df1aadf0\\\",\\\"vout\\\":1}]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("listlockunspent", "")
        );

    vector<COutPoint> vOutpts;
    pwalletMain->ListLockedCoins(vOutpts);

    Array ret;

    BOOST_FOREACH(COutPoint &outpt, vOutpts) {
        Object o;

        o.push_back(Pair("txid", outpt.hash.GetHex()));
        o.push_back(Pair("vout", (int)outpt.n));
        ret.push_back(o);
    }

    return ret;
}

Value settxfee(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
        throw runtime_error(
            "settxfee amount\n"
            "\nSet the transaction fee per kB.\n"
            "\nArguments:\n"
            "1. amount         (numeric, required) The transaction fee in BTC/kB rounded to the nearest 0.00000001\n"
            "\nResult\n"
            "true|false        (boolean) Returns true if successful\n"
            "\nExamples:\n"
            + HelpExampleCli("settxfee", "0.00001")
            + HelpExampleRpc("settxfee", "0.00001")
        );

    // Amount
    int64_t nAmount = 0;
    if (params[0].get_real() != 0.0)
        nAmount = AmountFromValue(params[0]);        // rejects 0.0 amounts

    nTransactionFee = nAmount;
    return true;
}

test::CAutoTest tests1;
Value tests(const Array& params, bool fHelp)
{
	if (fHelp || !(params.size() < 6 || params.size() == 1)) {
		throw runtime_error("getluckynum \n"
				"\nArguments:\n"
				"1. \"the thead stop or start\"         (int, required) the thead statr or not \n"
				"2. \"recivebetaddress\"         (str, required) the bet recive of address \n"
				"3. \"type\"         (int, opntional) \n"
				"4. \"cycles\"         (int, required)  ssycles times \n"
				"5. \"gaptimems\"         (int, opntional) \n"
				"6. \"privateKey\"     (string, opntinal) \n"
				"7. \"lottoPrivateKey\"(string, opntional) ");
	}
	if (params[0].get_bool() == true) {
		std::string reciveaddress = params[1].get_str();
		int cycles = 10;
		int gaptimems = 1001;
		std::string privateKey("123456");
		std::string lottoPrivateKey("123456789");
		if (params.size() == 5) {
			cycles = params[3].get_int();
			gaptimems = params[4].get_int();
		}
		if (params.size() == 4) {
			cycles = params[3].get_int();
		}
		if( params.size() > 5){
			privateKey = params[5].get_str();
		}
		if( params.size() > 6){
			lottoPrivateKey = params[6].get_str();
		}
		tests1.Inital(reciveaddress, cycles, gaptimems, privateKey, lottoPrivateKey);
		int type = params[2].get_int();
		if (type > 0 && type < 6) {
			tests1.startThread(type);
		} else {
			Assert(0);
		}

	} else {
		tests1.stopThread();
	}
	return string("");
}
void calcsameCount(vector<unsigned char>luckey,std::map<int, int > &save,int total)
{
	string tem = "";
	for(int i =0;i<luckey.size();i++)
	{
		save[luckey[i]]++;
	}

}
Value computepeernumcount(const Array& params, bool fHelp)
{
	if (fHelp) {
			throw runtime_error("getluckynum \n"
					"\nArguments:\n"
					"1. \"hight\"         (int64, required) the hight of the check point \n");
		}

	boost::int32_t intTemp(-1);
	if (params.size() > 0)
		intTemp = params[0].get_int();

	if (intTemp > chainActive.Height() || intTemp < 0)
		intTemp = chainActive.Height();

	std::map<int, int > cbet156Count;
	std::map<int, int > cbetMuiltredCount;
	std::map<int, int > cbetMuiltblueCount;
	string tem = "";
	string tem1 = "";
	while (intTemp > 0) {

		Array strParams;
		strParams.push_back(tfm::format("%d", intTemp));
		ConvertTo<int64_t>(strParams[0]);
		Value value =getluckynum(strParams,false);
		if (value.type() == obj_type) {
				const json_spirit::Object& obj = value.get_obj();
				size_t i = 0;
				for (; i < obj.size(); ++i) {
					const json_spirit::Pair& pair = obj[i];
					const std::string& str_name = pair.name_;
					if (str_name == "0") {
						const json_spirit::Value& val_val = pair.value_;
						vector<unsigned char>luckey;
						//cout<<"156:"<<val_val.get_str()<<endl;
						//tem +=tfm::format("156:%s\n",val_val.get_str());
						luckey = ParseHex(val_val.get_str());
						calcsameCount(luckey,cbet156Count,15);
					}
					if (str_name == "1") {
						const json_spirit::Value& val_val = pair.value_;
						vector<unsigned char>luckey;
						//cout<<"muli:"<<val_val.get_str()<<endl;
						//tem +=tfm::format("mulite:%s\n",val_val.get_str());
						luckey = ParseHex(val_val.get_str());
						std::vector<unsigned char>::iterator it = std::find(luckey.begin(), luckey.end(), lotto::gSplitFlag);
						std::vector<unsigned char> red(luckey.begin(),it);
						std::vector<unsigned char> blue(it +1,luckey.end());
						calcsameCount(red,cbetMuiltredCount,20);
						calcsameCount(blue,cbetMuiltblueCount,10);
					}
				}

		}
		intTemp--;
	}
	string str= "\ncbet156:\n";
	BOOST_FOREACH(const PAIRTYPE(int, int )& tep2 , cbet156Count)
	{
		str +=tfm::format("%d:%d,",tep2.first,tep2.second);
	}
	str +=tfm::format("\n%s\n","cbetmulit:\nred:\n");
	BOOST_FOREACH(const PAIRTYPE(int, int )& tep3 , cbetMuiltredCount)
	{
		str +=tfm::format("%d:%d,",tep3.first,tep3.second);
	}
	str +=tfm::format("\n%s\n","blue:\n");
	BOOST_FOREACH(const PAIRTYPE(int, int )& tep1 , cbetMuiltblueCount)
	{
		str +=tfm::format("%d:%d,",tep1.first,tep1.second);
	}
	return str;

}
