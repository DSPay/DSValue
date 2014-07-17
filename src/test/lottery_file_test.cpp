#include "lotteryfile.h"
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/rand.h>

BOOST_AUTO_TEST_SUITE(lotto_file)


BOOST_AUTO_TEST_CASE(cipher_test) {
	std::map<int, vector<unsigned char> > cipherMap, encryptMap;
	CLotData testfile("spark");

	CLottoFileKey key;

	vector<uint256> vchkey;
	vchkey.push_back(uint256(1234));
	vchkey.push_back(uint256(3456));
	vchkey.push_back(uint256(6789));
	key.setKey(vchkey);

	for (int ii = 0; ii < 1; ii++) {
		vector<unsigned char> data;
		RandAddSeedPerfmon();
		data.resize(32);
		RAND_bytes(&data[0], 32);
		LogTrace("spark", "the data:%s\n", VectorToString(data));
		uint256 hash = Hash(data.begin(), data.end());
		LogTrace("spark", "the hash:%s\n", hash.ToString());
		data.insert(data.end(), hash.begin(), hash.end());
		cipherMap.insert(make_pair(ii, data));
		LogTrace("spark", "the data + hash:%s\n", VectorToString(data));
	}

//encrypt
	BOOST_FOREACH(PAIRTYPE(const int, vector<unsigned char>)& item, cipherMap) {
		vector<unsigned char> endata;
		BOOST_CHECK_EQUAL(testfile.EncryptData(key, item.second, endata), true);
		LogTrace("spark", "the endata:%s\n", VectorToString(endata));
		encryptMap.insert(make_pair(item.first, endata));
	}

//decrypt
	BOOST_FOREACH(PAIRTYPE(const int, vector<unsigned char> )& item, encryptMap) {
		vector<unsigned char> tmp;
		BOOST_CHECK_EQUAL(testfile.DecryptData(key, item.second, tmp), true);
		BOOST_CHECK(tmp == cipherMap[item.first]);
	}

//error key test
	CLottoFileKey nkey;
	vector<uint256> nvchkey;
	nvchkey.push_back(uint256(1233));
	nvchkey.push_back(uint256(3456));
	nvchkey.push_back(uint256(6789));
	nkey.setKey(nvchkey);
	BOOST_FOREACH(PAIRTYPE(const int, vector<unsigned char> )& item, encryptMap) {
		vector<unsigned char> tmp;
		BOOST_CHECK_EQUAL(testfile.DecryptData(nkey, item.second, tmp), false);
		BOOST_CHECK(tmp != cipherMap[item.first]);
	}
}

BOOST_AUTO_TEST_CASE(write_read_test) {
	CLotData testfile("regtest_db.dat");
	testfile.CreateClottoFile("regtest_db_key.dat",1024);
}

BOOST_AUTO_TEST_SUITE_END()

