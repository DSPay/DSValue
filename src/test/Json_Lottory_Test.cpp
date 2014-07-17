/*
 * Json_Lottory_Test.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: spark.huang
 */
#include <boost/test/execution_monitor.hpp>
#include <boost/test/unit_test.hpp>
#include "CEncryptKey.h"

BOOST_AUTO_TEST_SUITE(JsonLottoTest)

BOOST_AUTO_TEST_CASE(xx) {
	CObjectFile test;
	test.SetTestPath("Lottory.dat");
	BOOST_CHECK(test.GenerateClottoFile(10));
	BOOST_CHECK(test.GeneratePrivatKey("123", 10));
	BOOST_CHECK(test.EncryptClottoKey("12345"));
	BOOST_CHECK(test.AppendLottoKeyObject());
	BOOST_CHECK(test.AppendLottoObject());
	BOOST_CHECK(test.AppendPkObject());
	BOOST_CHECK(test.AppendValue("test1", "fuck shiweisong"));
	BOOST_CHECK(test.AppendValue("test2", "fuck songju"));
	BOOST_CHECK(test.AppendValue("test3", "shit shiweisong"));
	BOOST_CHECK(test.AppendValue("test4", "shit songju"));
	BOOST_CHECK(test.SaveObject());

	CObjectFile test1;
	vector<unsigned char> xx, yy, zz;
	test1.SetTestPath("Lottory.dat");
	BOOST_CHECK(test1.ReadObject());

	Value tmp;
	BOOST_CHECK(test1.GetValue("test1", tmp));
	LogTrace("spark", "test1:%s\n", tmp.get_str());
	BOOST_CHECK_EQUAL(tmp.get_str(), string("fuck shiweisong"));
	BOOST_CHECK(test1.GetValue("test2", tmp));
	LogTrace("spark", "test2:%s\n", tmp.get_str());
	BOOST_CHECK_EQUAL(tmp.get_str(), string("fuck songju"));
	BOOST_CHECK(test1.GetValue("test3", tmp));
	LogTrace("spark", "test3:%s\n", tmp.get_str());
	BOOST_CHECK_EQUAL(tmp.get_str(), string("shit shiweisong"));
	BOOST_CHECK(test1.GetValue("test4", tmp));
	LogTrace("spark", "test4:%s\n", tmp.get_str());
	BOOST_CHECK_EQUAL(tmp.get_str(), string("shit songju"));

	BOOST_CHECK(test1.ReadPrivateKey(1, "123", xx));
	BOOST_CHECK(test1.ReadClottoKey(5, "12345", yy));
	BOOST_CHECK(test1.ReadClottoData(5, uint256(yy), zz));

	LogTrace("spark", "ReadPrivateKey:%s\n", HexStr(xx));
	LogTrace("spark", "ReadClottoKey:%s\n", HexStr(yy));
	LogTrace("spark", "ReadClottoData:%s\n", HexStr(zz));
}

BOOST_AUTO_TEST_SUITE_END()

