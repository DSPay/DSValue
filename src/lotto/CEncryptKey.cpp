/*
 * CEncryptKey.cpp
 *
 *  Created on: Jul 7, 2014
 *      Author: spark.huang
 */
#include "CEncryptKey.h"

#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/evp.h>

static bool EncryptData(const vector<unsigned char> &key, const vector<unsigned char> &vInput,
		vector<unsigned char>& vEncrypted) {
	CKeyingMaterial tmpinput(vInput.begin(), vInput.end());
	CKeyingMaterial tmpkey(key.begin(), key.end());
	if (!sEncryptData(tmpkey, tmpinput, uint256(100), vEncrypted)) {
		Assert(0);
		return false;
	}
	return true;
}

static bool DecryptData(const vector<unsigned char> &key, const vector<unsigned char>& vInput,
		vector<unsigned char>& vDecrypted) {
	CKeyingMaterial tmpkey(key.begin(), key.end());
	CKeyingMaterial tmpout;
	if (!sDecryptData(tmpkey, vInput, uint256(100), tmpout)) {
		Assert(0);
		return false;
	}
	vDecrypted.clear();
	vDecrypted.assign(tmpout.begin(), tmpout.end());

	return true;
}

/**************************************************************************************************/

bool CObjectFile::GeneratePrivatKey(const string passwd, const int size) {
	Assert(size > 0);
	uint256 ukey = Hash(passwd.begin(), passwd.end());
	vector<unsigned char> vkey(ukey.begin(), ukey.end());
	for (int ii = 0; ii < size; ii++) {
		RandAddSeedPerfmon();
		CKey secret;
		secret.MakeNewKey(true);
		CPubKey pubkey = secret.GetPubKey();
		vector<unsigned char> vEnData;
		vector<unsigned char> vClear(secret.begin(), secret.end());
		if (!EncryptData(vkey, vClear, vEnData)) {
			Assert(0);
			return false;
		}

		vector<unsigned char> vReClearData;
		if (!DecryptData(vkey, vEnData, vReClearData)) {
			Assert(0);
			return false;
		}

		if (vReClearData != vClear) {
			Assert(0);
			return false;
		}

		{
			CKey key;
			key.Set(vReClearData.begin(), vReClearData.end(), pubkey.IsCompressed());
			if (key.GetPubKey() != pubkey) {
				Assert(0);
				return false;
			}

		}
		{
			Object otmp;
			otmp.push_back(Pair("index", itostr(ii)));
			otmp.push_back(Pair("enprivatekey", HexStr(vEnData)));
			otmp.push_back(Pair("publickey", HexStr(pubkey.begin(), pubkey.end())));
			Objpk.push_back(Pair(itostr(ii), otmp));
		}
#if 0
		{
			Object tmp;
			tmp.push_back(Pair("index", itostr(ii)));
			tmp.push_back(Pair("privatekey", HexStr(vClear)));
			Objtest.push_back(Pair("private", tmp));
		}
#endif
	}

	return true;
}

bool CObjectFile::GenerateClottoFile(const int size) {
	Assert(size > 0);
	for (int ii = 0; ii < size; ii++) {

		//Creat key data
		vector<unsigned char> vkey = GetRandomVec(32);

		//Creat Clear Text
		vector<unsigned char> vClearData = GetRandomVec(32);

		//encrypt data
		vector<unsigned char> vEnData;
		if (!EncryptData(vkey, vClearData, vEnData)) {
			Assert(0);
			return false;
		}

		//decrypt data
		vector<unsigned char> vReClearData;
		if (!DecryptData(vkey, vEnData, vReClearData)) {
			Assert(0);
			return false;
		}

		//verify data
		if (vReClearData != vClearData) {
			Assert(0);
		} else {
			{
				Object okey;
				okey.push_back(Pair("index", itostr(ii)));
				okey.push_back(Pair("keydata", HexStr(vkey)));
				Objkey.push_back(Pair(itostr(ii), okey));
			}
#if 0
			{
				Object tmp;
				tmp.push_back(Pair("index", itostr(ii)));
				tmp.push_back(Pair("keydata", HexStr(vkey)));
				tmp.push_back(Pair("cleardata", HexStr(vClearData)));
				Objtest.push_back(Pair("lotto", tmp));
			}
#endif
			{
				Object oendata;
				oendata.push_back(Pair("index", itostr(ii)));
				oendata.push_back(Pair("endata", HexStr(vEnData)));
				oendata.push_back(Pair("hash", HexStr(Hash(vClearData.begin(), vClearData.end()))));
				Objlotto.push_back(Pair(itostr(ii), oendata));
			}
		}
	}

	return true;
}

bool CObjectFile::EncryptClottoKey(string passwd) {
	uint256 ukey = Hash(passwd.begin(), passwd.end());
	Object oenkey;
	vector<unsigned char> vkey(ukey.begin(), ukey.end());
	for (Object::const_iterator i = Objkey.begin(); i != Objkey.end(); ++i) {
		vector<unsigned char> vEnData;
		vector<unsigned char> vClear = ParseHex(find_value(i->value_.get_obj(), "keydata").get_str());

		if (!EncryptData(vkey, vClear, vEnData)) {
			Assert(0);
			return false;
		}
		uint256 hash = Hash(vClear.begin(), vClear.end());
		{
			Object otmp;
			string sindex = find_value(i->value_.get_obj(), "index").get_str();
			otmp.push_back(Pair("index", sindex));
			otmp.push_back(Pair("endata", HexStr(vEnData)));
			otmp.push_back(Pair("hash", HexStr(hash.begin(), hash.end())));
			oenkey.push_back(Pair(sindex, otmp));
		}
	}
	Objkey.clear();
	Objkey = oenkey;
	return true;
}

bool CObjectFile::ReadPrivateKey(const int index, const string passwd, vector<unsigned char>& vchout) {

	Value mm = find_value(Objpk, itostr(index));
	if (mm.type() != obj_type) {
		Assert(0);
		return false;
	}

	uint256 ukey = Hash(passwd.begin(), passwd.end());
	vector<unsigned char> vEnData = ParseHex(find_value(mm.get_obj(), "enprivatekey").get_str());
	vector<unsigned char> vPubKey = ParseHex(find_value(mm.get_obj(), "publickey").get_str());
	CPubKey pubkey(vPubKey.begin(), vPubKey.end());
	vector<unsigned char> vReClearData;
	vector<unsigned char> vkey(ukey.begin(), ukey.end());

	if (!DecryptData(vkey, vEnData, vReClearData)) {
		Assert(0);
		return false;
	}

	{
		CKey key;
		key.Set(vReClearData.begin(), vReClearData.end(), pubkey.IsCompressed());
		if (key.GetPubKey() != pubkey) {
			Assert(0);
			return false;
		}
	}
	vchout = vReClearData;

	return true;
}

bool CObjectFile::ReadClottoKey(const int index, const string passwd, vector<unsigned char>& vchdataout) {

	Value mm = find_value(Objkey, itostr(index));
	if (mm.type() != obj_type) {
		Assert(0);
		return false;
	}
	//decrypt data
	uint256 ukey = Hash(passwd.begin(), passwd.end());
	vector<unsigned char> vEnData = ParseHex(find_value(mm.get_obj(), "endata").get_str());
	uint256 hash(ParseHex(find_value(mm.get_obj(), "hash").get_str()));
	vector<unsigned char> vReClearData;
	vector<unsigned char> vkey(ukey.begin(), ukey.end());

	if (!DecryptData(vkey, vEnData, vReClearData)) {
		Assert(0);
		return false;
	}

	if (hash != Hash(vReClearData.begin(), vReClearData.end())) {
		Assert(0);
		return false;
	}

	vchdataout = vReClearData;
	return true;
}

bool CObjectFile::ReadClottoData(const int index, const uint256& passwd, vector<unsigned char>& vchdataout) {

	Value mm = find_value(Objlotto, itostr(index));
	if (mm.type() != obj_type) {
		Assert(0);
		return false;
	}

	vector<unsigned char> vEnData = ParseHex(find_value(mm.get_obj(), "endata").get_str());
	uint256 hash(ParseHex(find_value(mm.get_obj(), "hash").get_str()));
	vector<unsigned char> vReClearData;
	vector<unsigned char> vkey(passwd.begin(), passwd.end());
	if (!DecryptData(vkey, vEnData, vReClearData)) {
		Assert(0);
		return false;
	}

	if (hash != Hash(vReClearData.begin(), vReClearData.end())) {
		Assert(0);
		return false;
	}
	vchdataout = vReClearData;

	return true;
}

uint256 CObjectFile::BuildMerkleTree(vector<uint256> vhash) {
	vector<uint256> vMerkleTree = vhash;
	int j = 0;
	for (int nSize = vhash.size(); nSize > 1; nSize = (nSize + 1) / 2) {
		for (int i = 0; i < nSize; i += 2) {
			int i2 = std::min(i + 1, nSize - 1);
			vMerkleTree.push_back(
					Hash(BEGIN(vMerkleTree[j + i]), END(vMerkleTree[j + i]), BEGIN(vMerkleTree[j + i2]),
							END(vMerkleTree[j + i2])));
		}
		j += nSize;
	}
	return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
}
bool CObjectFile::Initialize(const std::string& file, const std::string&merkleRoot)
{
	LotteryDataMerkleHash = merkleRoot;

	bool ret = false;
	do
	{
		path = GetDataDir() / file;
		if (!boost::filesystem::is_regular_file(path))
		{
			Assert(0);
			break;
		}
		if (!ReadObject()) {
			Assert(0);
			break;
		}
		if (!GetLottoObject()) {
			Assert(0);
			break;
		}
		GetLottoKeyObject();
		GetPkObject();
		ret = true;
	}while(false);
	return ret;
}
