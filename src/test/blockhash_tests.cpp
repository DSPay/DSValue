
#include "uint256.h"
#include "hashblock.h"
#include "util.h"
#include "core.h"


#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(blockhash_tests)

BOOST_AUTO_TEST_CASE(x11hash)
{
	CBlockHeader testheader;
	testheader.hashPrevBlock = 0;
	testheader.hashMerkleRoot = 0;
	testheader.nVersion = 0;
	testheader.nTime    = 0;
	testheader.nBits    = 0;
	testheader.nNonce   = 0;
	BOOST_CHECK_EQUAL(Hash9(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("a33e7ae4978d4c851d725e5ed811a662f52ab65d011ef656ce5b968061842883"));
	testheader.nVersion = 1;
	testheader.nTime    = 123456789;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 10;
	BOOST_CHECK_EQUAL(Hash9(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("f290936b6b81193a0ca1411b0617fd3f3cc5e5fe07c581e536f3491feca8e066"));

	testheader.nVersion = 2;
	testheader.nTime    = 987654321;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 100;
	BOOST_CHECK_EQUAL(Hash9(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("576bcdd406b9825963098b3e072df5c734c9f3e124b609778d79993ac8265a23"));

	testheader.nVersion = 3;
	testheader.nTime    = 147258369;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 1000;
	BOOST_CHECK_EQUAL(Hash9(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("43af9bb1a1a7d1a1f9218978478df3f304488c0ebf0f3f68e2c820336f914b45"));

	testheader.nVersion = 4;
	testheader.nTime    = 963852741;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 10000;
	BOOST_CHECK_EQUAL(Hash9(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("a72be5e57f3b1a778a34cca346b028478bf20e6bb049ae6b2304ffeba75feb01"));
}
BOOST_AUTO_TEST_CASE(ltcoinhash)
{
	CBlockHeader testheader;
	testheader.hashPrevBlock = 0;
	testheader.hashMerkleRoot = 0;
	testheader.nVersion = 0;
	testheader.nTime    = 0;
	testheader.nBits    = 0;
	testheader.nNonce   = 0;
	BOOST_CHECK_EQUAL(LtcoinBlockHash(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("694b3a55a61339b43c01421b13f710e22e33a7eabda1cd48103bb9f376081d16"));

	testheader.nVersion = 1;
	testheader.nTime    = 123456789;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 10;
	BOOST_CHECK_EQUAL(LtcoinBlockHash(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("5ad26ef4afa2d4094642d0955e9c46710ba72809f2c4382b4309a896e5b24c49"));

	testheader.nVersion = 2;
	testheader.nTime    = 987654321;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 100;
	BOOST_CHECK_EQUAL(LtcoinBlockHash(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("e79a4e120df53bd4fb0929ca2b0f4e745b8237006184bc92e6467988d487a0e1"));

	testheader.nVersion = 3;
	testheader.nTime    = 147258369;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 1000;
	BOOST_CHECK_EQUAL(LtcoinBlockHash(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("eadb0f3e6039d320d3e74feb2c17931b522a159c7fe2ad3eba6274666959f7b5"));

	testheader.nVersion = 4;
	testheader.nTime    = 963852741;
	testheader.nBits    = 0x1d00ffff;
	testheader.nNonce   = 10000;
	BOOST_CHECK_EQUAL(LtcoinBlockHash(BEGIN(testheader.nVersion), END(testheader.nNonce)).ToString(), std::string("82283757841a98b13b0fc1aa1caae69a5a7d20721516ce88312f1f76ab6c972c"));
}

BOOST_AUTO_TEST_SUITE_END()
