//
// Unit tests for block.CheckBlock()
//



#include "main.h"
#include "rpcserver.h"
#include "rpcclient.h"

#include "base58.h"
#include "wallet.h"
#include "CDebug.h"
#include <cstdio>
#include "lotto/lotteryfile.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include "lotteryfile.h"

using namespace json_spirit;
extern Value CallRPC(string args);
extern CWallet* pwalletMain;
BOOST_AUTO_TEST_SUITE(CheckBlock_tests)

bool
read_block(const std::string& filename, CBlock& block)
{
    namespace fs = boost::filesystem;
    fs::path testFile = fs::current_path() / "data" / filename;
#ifdef TEST_DATA_DIR
    if (!fs::exists(testFile))
    {
        testFile = fs::path(BOOST_PP_STRINGIZE(TEST_DATA_DIR)) / filename;
    }
#endif
    FILE* fp = fopen(testFile.string().c_str(), "rb");
    if (!fp) return false;
    static int pos=0;
    fseek(fp, pos+8, SEEK_SET); // skip msgheader/size

    CAutoFile filein = CAutoFile(fp, SER_DISK, CLIENT_VERSION);
    if (!filein) return false;

    filein >> block;
    //filein >> block;
    pos += ftell(fp);
    return true;
}
BOOST_AUTO_TEST_CASE(test_read)
{
	CBlockIndex *pBlockIndex = chainActive.Genesis();
	boost::filesystem::path testFile = GetDataDir() / "blk00000.dat";

//	fs::path testFile = fs::current_path() / "blk00000.dat";
	FILE * file = fopen(testFile.string().c_str(), "ab+");
	if(!file) return;
	fseek(file, 0, SEEK_END);
	int len = ftell(file);
	fseek(file, 0, SEEK_SET);
	while(!feof(file) && len-->0){
		fputc(0,file);
	}
//	fseek(file, 0, SEEK_SET);
//	unsigned char buf[4];
//	memcpy(buf, Params().MessageStart(), 4);
//	unsigned int length = 0;
//	CDataStream stream(SER_DISK, CLIENT_VERSION);
//	while ((pBlockIndex = chainActive.Next(pBlockIndex)))
//	{
//		if(pBlockIndex->nHeight == 186)
//			break;
//		CBlock block;
//		if (!ReadBlockFromDisk(block, pBlockIndex))
//			throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
//
//		block.print();
//		length = block.GetSerializeSize(SER_DISK, CLIENT_VERSION);
//		stream << buf[0]<< buf[1]<< buf[2] << buf[3] << length << block;
//	}
//	fwrite(&stream[0], 1, stream.size(), file);
	fclose(file);
}

BOOST_AUTO_TEST_CASE(May15)
{
    // Putting a 1MB binary file in the git repository is not a great
    // idea, so this test is only run if you manually download
    // test/data/Mar12Fork.dat from
    // http://sourceforge.net/projects/bitcoin/files/Bitcoin/blockchain/Mar12Fork.dat/download
    unsigned int tMay15 = 1368576000;
    SetMockTime(tMay15); // Test as if it was right at May 15

    CBlock forkingBlock;
    for(int i=0; i< 10; ++i)
    {
    if (read_block("Mar12Fork.dat", forkingBlock))
    {
        CValidationState state;
        forkingBlock.print();
        // After May 15'th, big blocks are OK:
        forkingBlock.nTime = tMay15; // Invalidates PoW
        BOOST_CHECK(CheckBlock(forkingBlock, state, false, false));
    }
    }

    SetMockTime(0);
}
BOOST_AUTO_TEST_CASE(test_newblock)
{

	CBlock forkingBlock;
	//CallRPC("setgenerate true 12");
	CBlockIndex * pindexPrev = chainActive[1];
	Assert(pindexPrev);
  	CDiskBlockPos curTipPos(pindexPrev->nFile, pindexPrev->nDataPos);
  	if(!ReadBlockFromDisk(forkingBlock, curTipPos))
  		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
  	forkingBlock.print();

	CValidationState state;

	BOOST_CHECK(CheckBlock(forkingBlock, state, false, false));

	///// modify the hash
	std::map<int, uint256>::iterator it;
	it = forkingBlock.lottoHeader.mBitcoinHash.begin();
	uint256 npre = (*it).second ;
	(*it).second =  uint256(0);
	npre = (*it).second ;
	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));

	///// modify the height
	pindexPrev = chainActive[2];
	Assert(pindexPrev);
	CDiskBlockPos curTipPos1(pindexPrev->nFile, pindexPrev->nDataPos);
	if(!ReadBlockFromDisk(forkingBlock, curTipPos1))
		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
	forkingBlock.print();
	it = forkingBlock.lottoHeader.mBitcoinHash.begin();
	int index= (*it).first;
	 npre = (*it).second ;
	forkingBlock.lottoHeader.mBitcoinHash.erase(it);
	forkingBlock.lottoHeader.mBitcoinHash[index + 6] = npre;
	forkingBlock.print();
	bool ret=CheckBlock(forkingBlock, state, false, false);
	LogTrace("bess", "%d\n", ret);
	BOOST_CHECK(!ret);
	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));


	//// erase a block
	pindexPrev = chainActive[3];
	Assert(pindexPrev);
	CDiskBlockPos curTipPos2(pindexPrev->nFile, pindexPrev->nDataPos);
	if(!ReadBlockFromDisk(forkingBlock, curTipPos2))
		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
	it = forkingBlock.lottoHeader.mBitcoinHash.begin();
	if(forkingBlock.lottoHeader.mBitcoinHash.size() >1)
	it++;
	forkingBlock.lottoHeader.mBitcoinHash.erase(it);
	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));


	/// the height and hash is modifyed
	pindexPrev = chainActive[4];
	Assert(pindexPrev);
	CDiskBlockPos curTipPos3(pindexPrev->nFile, pindexPrev->nDataPos);
	if(!ReadBlockFromDisk(forkingBlock, curTipPos3))
		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
	it = forkingBlock.lottoHeader.mBitcoinHash.begin();
	if(forkingBlock.lottoHeader.mBitcoinHash.size() >1)
	it++;
	npre = (*it).second ;
	forkingBlock.lottoHeader.mBitcoinHash.erase(it);
	forkingBlock.lottoHeader.mBitcoinHash[6] = uint256(0);
	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));


    pindexPrev = chainActive[7];
	Assert(pindexPrev);
	CDiskBlockPos curTipPos4(pindexPrev->nFile, pindexPrev->nDataPos);
	if(!ReadBlockFromDisk(forkingBlock, curTipPos4))
		  throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");
	forkingBlock.print();
//	cout<<"id"<<forkingBlock.lottoHeader.nLottoID<<"scrhash"<<forkingBlock.lottoHeader.uSecretHash.ToString()<<endl;
	BOOST_CHECK(CheckBlock(forkingBlock, state, false, false));
	int64_t npreValue = forkingBlock.lottoHeader.llPool;
	forkingBlock.lottoHeader.llPool = 500000000000;
	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));

//	forkingBlock.lottoHeader.llPool = npreValue;
//	CTransaction coinBaseTx = forkingBlock.vtx[0];
//	std::vector<CTxOut> lottoTxOut(coinBaseTx.vout.begin(), coinBaseTx.vout.end());
//	npreValue = lottoTxOut[0].nValue;
//	if(lottoTxOut.size() >0)
//		forkingBlock.vtx[0].vout[0].nValue = 51*COIN;
//	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));

	forkingBlock.vtx[0].vout[0].nValue = npreValue;

	int lottid = forkingBlock.lottoHeader.nLottoID;
	forkingBlock.lottoHeader.nLottoID = lottid + 5;
	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));

	forkingBlock.lottoHeader.nLottoID = lottid ;

	uint256 prehash = forkingBlock.lottoHeader.uLottoKey;
	forkingBlock.lottoHeader.uLottoKey = uint256("4565");
	BOOST_CHECK(!CheckBlock(forkingBlock, state, false, false));

}
BOOST_AUTO_TEST_CASE(test_read_block)
{
	CBlockIndex *pBlockIndex = chainActive.Genesis();
//	FILE * file = OpenFile("blk00000.dat", false);
//	if(!file) return;
//	int len = ftell(file);
//	fseek(file, 0, SEEK_SET);
//	fclose(file);
	unsigned char buf[4];
	memcmp(buf, Params().MessageStart(), 4);
	unsigned int length = 0;
	while ((pBlockIndex = chainActive.Next(pBlockIndex)))
	{
		if(pBlockIndex->nHeight == 186)
			break;
		CBlock block;
		if (!ReadBlockFromDisk(block, pBlockIndex))
			throw std::runtime_error("CreateNewBlock() : Read current tip block from disk failed");

		block.print();
		length = block.GetSerializeSize(SER_DISK, CLIENT_VERSION);
		CDataStream stream(SER_DISK, CLIENT_VERSION);
		stream << buf[0]<< buf[1]<< buf[2] << buf[3] << length;
		AppendToFile("blk00000.dat", stream);
	 	AppendToFile("blk00000.dat", block);
	}
}

BOOST_AUTO_TEST_SUITE_END()
