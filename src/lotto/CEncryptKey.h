/*
 * CEncryptKey.h
 *
 *  Created on: Jul 7, 2014
 *      Author: spark.huang
 */

#ifndef CENCRYPTKEY_H_
#define CENCRYPTKEY_H_
#include <iostream>     // std::cout
#include <fstream>#include <algorithm>    // std::equal#include <vector>       // std::vector#include <algorithm>#include <exception>#include <map>#include <set>#include <stdint.h>#include <string>#include <utility>#include <vector>#include "key.h"#include "uint256.h"#include <boost/foreach.hpp>#include <boost/filesystem.hpp>#include "util.h"#include "lotteryfile.h"#include "json/json_spirit_reader_template.h"#include "json/json_spirit_reader.h"#include "json/json_spirit_writer.h"#include "json/json_spirit_value.h"#include "json/json_spirit_stream_reader.h"#include "json/json_spirit_utils.h"/**************************************************************************************************/
using namespace std;
using namespace json_spirit;
using namespace boost;

class CObjectFile {
private:
	json_spirit::Object Objmain;
	json_spirit::Object Objpk, Objlotto, Objkey;
#if 1
	json_spirit::Object Objtest;
	boost::filesystem::path pathtest;
#endif
	boost::filesystem::path path;

	bool AppendObject(string oName, const Object& inObj) {
		Objmain.push_back(Pair(oName, inObj));
		return true;
	}
	uint256 BuildMerkleTree(vector<uint256> vhash);

	bool GetPkObject() {
		Objpk.clear();
		Objpk = find_value(Objmain, "privatekey").get_obj();
		return (Objpk.size() > 0) ? (true) : (false);
	}

	bool GetLottoObject() {
		const string LotteryDataMerkleHash = "b2a8ce5ac4aab2a6f1d7ef694d72b3c05ace80d543feeeb7e320be7c534ab4d0";
		Objlotto.clear();
		Objlotto = find_value(Objmain, "lottorydata").get_obj();
		if (Objlotto.size() > 0) {
			Object ohash = find_value(Objlotto, "MerkleHash").get_obj();
			std::string merkleHash = find_value(ohash, "MerkleHash").get_str();
			if (merkleHash != LotteryDataMerkleHash)
			{
				return false;
			}
			uint256 mhash = uint256(ParseHex(merkleHash));
			vector<uint256> vhash;
			for (Object::const_iterator i = Objlotto.begin(); i != Objlotto.end(); ++i) {
				if (!find_value(i->value_.get_obj(), "hash").is_null()) {
					vhash.push_back(uint256(ParseHex(find_value(i->value_.get_obj(), "hash").get_str())));
				}

			}
			uint256 mhashtmp = BuildMerkleTree(vhash);
			if (mhash != mhashtmp) {
				return false;
			}
			return true;
		} else {
			return false;
		}
	}

	bool GetLottoKeyObject() {
		const std::string LotteryKeyMerkleHash = "095a6b3d4722505b274a072be262b1112c988086116babdc4c5b07ac44902d21";
		Objkey.clear();
		Objkey = find_value(Objmain, "lottoryKey").get_obj();
		if (Objkey.size() > 0) {
			Object ohash = find_value(Objkey, "MerkleHash").get_obj();
			std::string merkleHash = find_value(ohash, "MerkleHash").get_str();
			if (merkleHash != LotteryKeyMerkleHash)
			{
				return false;
			}
			uint256 mhash = uint256(ParseHex(merkleHash));
			vector<uint256> vhash;
			for (Object::const_iterator i = Objkey.begin(); i != Objkey.end(); ++i) {
				if (!find_value(i->value_.get_obj(), "hash").is_null()) {
					vhash.push_back(uint256(ParseHex(find_value(i->value_.get_obj(), "hash").get_str())));
				}
			}
			uint256 mhashtmp = BuildMerkleTree(vhash);
			if (mhash != mhashtmp) {
				return false;
			}
			return true;
		} else {
			return false;
		}
	}

public:
	CObjectFile() {
		path = GetDataDir() / "objmain.dat";
		boost::filesystem::create_directories(path.parent_path());
		Objmain.clear();
	}
	CObjectFile(string file) {
		path = GetDataDir() / file;
		boost::filesystem::create_directories(path.parent_path());
		Objmain.clear();
	}

	void SetTestPath(string file) {
		boost::filesystem::path pathtmp;
		namespace fs = boost::filesystem;
		pathtmp = fs::system_complete("d:\\bitcoin");
		path = pathtmp / file;
#if 1
		pathtest = pathtmp / "test.dat";
#endif
	}

	bool SaveObject() {
		ofstream ofenkey(path.string().c_str());
		write_formatted(Objmain, ofenkey);
		ofenkey.close();
#if 1
		{
			ofstream oftest(pathtest.string().c_str());
			write_formatted(Objtest, oftest);
			oftest.close();
		}
#endif
		return true;
	}

	bool ReadObject() {
		ifstream ls(path.string().c_str());
		json_spirit::Value tmp;
		json_spirit::read(ls, tmp);
		ls.close();
		if (tmp.type() != obj_type) {
			return false;
		}
		Objmain.clear();
		Objmain = tmp.get_obj();
#if 0
		{
			ifstream lstest(pathtest.string().c_str());
			json_spirit::Value tmp1;
			json_spirit::read(lstest, tmp1);
			lstest.close();
			if (tmp1.type() != obj_type) {
				return false;
			}
			Objtest.clear();
			Objtest = tmp1.get_obj();
		}
#endif
		return true;
	}

	bool AppendValue(const string Name, const Value& inValue) {
		Objmain.push_back(Pair(Name, inValue));
		return true;
	}

	bool GetValue(const string Name, Value& outValue) {
		outValue = find_value(Objmain, Name);
		return (outValue.is_null()) ? (false) : (true);
	}

#if 0
	bool GetTestValue(const Object Name, Value& outValue)
	{
		Object::const_iterator ii = Name;
		Object tmp = find_value(Objtest, ii->name_).get_obj();
		outValue = find_value(tmp, ii->value_);
		return true;
	}
#endif

	Object GetObject() {
		return Objmain;
	}

	bool AppendPkObject() {
		Assert(Objpk.size() > 0);
		return AppendObject("privatekey", Objpk);
	}

	bool AppendLottoObject() {
		Assert(Objlotto.size() > 0);
		vector<uint256> vhash;
		for (Object::const_iterator i = Objlotto.begin(); i != Objlotto.end(); ++i) {
			vhash.push_back(uint256(ParseHex(find_value(i->value_.get_obj(), "hash").get_str())));
		}
		uint256 mhash = BuildMerkleTree(vhash);
		{
			Object tmp;
			tmp.push_back(Pair("MerkleHash", HexStr(mhash.begin(), mhash.end())));
			Objlotto.push_back(Pair("MerkleHash", tmp));
		}

		return AppendObject("lottorydata", Objlotto);
	}

	bool AppendLottoKeyObject() {
		Assert(Objkey.size() > 0);
		vector<uint256> vhash;
		for (Object::const_iterator i = Objkey.begin(); i != Objkey.end(); ++i) {
			vhash.push_back(uint256(ParseHex(find_value(i->value_.get_obj(), "hash").get_str())));
		}
		uint256 mhash = BuildMerkleTree(vhash);
		{
			Object tmp;
			tmp.push_back(Pair("MerkleHash", HexStr(mhash.begin(), mhash.end())));
			Objkey.push_back(Pair("MerkleHash", tmp));
		}
		return AppendObject("lottoryKey", Objkey);
	}

	bool GeneratePrivatKey(const string passwd, const int size);
	bool GenerateClottoFile(const int size);
	bool EncryptClottoKey(string passwd);

	//operate the Objmain
	bool ReadPrivateKey(const int index, const string passwd, vector<unsigned char>& vchout);
	bool ReadClottoKey(const int index, const string passwd, vector<unsigned char>& vchdataout);
	bool ReadClottoData(const int index, const uint256& passwd, vector<unsigned char>& vchdataout);
};

#endif /* CENCRYPTKEY_H_ */
