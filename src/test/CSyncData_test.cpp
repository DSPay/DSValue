/*
 * CSyncData_test.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: ranger.shi
 */

#include "CSyncData.h"

#include <cstdio>
#include <string>
#include <vector>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

class SyncData_test
{
	public:
		SyncData_test()
		{
			m_key.MakeNewKey(true);
			m_privKey = HexStr(m_key.GetPrivKey());
			CPubKey pKey = m_key.GetPubKey();
			m_pubKey = HexStr(pKey.begin(), pKey.end());
		}
		void CheckSignature_test()
		{
			BOOST_CHECK(!m_privKey.empty());
			BOOST_CHECK(!m_pubKey.empty());
			CPrivKey prKey = m_key.GetPrivKey();
			std::vector<unsigned char> sig;
			std::vector<unsigned char> data(prKey.begin(), prKey.end());
			BOOST_CHECK(m_key.Sign(Hash(data.begin(), data.end()), sig));
			m_syncData.m_vchMsg = data;
			m_syncData.m_vchSig = sig;
			BOOST_CHECK(m_syncData.CheckSignature(m_pubKey));
		}
		void Sign_test()
		{
			BOOST_CHECK(!m_privKey.empty());
			BOOST_CHECK(!m_pubKey.empty());
			CPrivKey prKey = m_key.GetPrivKey();
			std::vector<unsigned char> data;
			data.assign(prKey.begin(), prKey.end());
			BOOST_CHECK(m_syncData.Sign(std::vector<unsigned char>(m_key.begin(), m_key.end()), data));
			BOOST_CHECK(data == m_syncData.m_vchMsg);
			CPubKey pKey = m_key.GetPubKey();
			BOOST_CHECK(pKey.Verify(Hash(data.begin(), data.end()), m_syncData.m_vchSig));
		}
	private:
		CKey				m_key;
		std::string			m_privKey;
		std::string			m_pubKey;
		SyncData::CSyncData	m_syncData;
};

BOOST_AUTO_TEST_SUITE(CSyncData_test)

BOOST_AUTO_TEST_CASE(Sign_test)
{
	SyncData_test test;
	test.Sign_test();
}

BOOST_AUTO_TEST_CASE(CheckSignature_test)
{
	SyncData_test test;
	test.CheckSignature_test();
}

BOOST_AUTO_TEST_SUITE_END()
