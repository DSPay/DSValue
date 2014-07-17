/*
 * Unit test for CChainManager class
 */
#include "bitcoinnet/CChainManager.h"
#include "bitcoinnet/CBitcoinServer.h"

#include <vector>
#include <cstdio>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>


using namespace Bitcoin;
using namespace std;
using namespace boost;


BOOST_AUTO_TEST_SUITE(chain_manager)

void ReadLine(FILE *file, char *pCh)
{
	 if (file == NULL) {
	        printf("Error: file pointer is null.");
	        exit(1);
	 }

	int maximumLineLength = 128;
	char *lineBuffer = (char *)malloc(sizeof(char) * maximumLineLength);

	if (lineBuffer == NULL) {
		printf("Error allocating memory for line buffer.");
		exit(1);
	}

	char ch = getc(file);
	int count = 0;

	while ((ch != '\n') && (ch != EOF)) {
		if (count == maximumLineLength) {
			maximumLineLength += 128;
			lineBuffer = (char *)realloc(lineBuffer, maximumLineLength);
			if (lineBuffer == NULL) {
				printf("Error reallocating space for line buffer.");
				exit(1);
			}
		}
		lineBuffer[count] = ch;
		count++;

		ch = getc(file);
	}

	lineBuffer[count] = '\0';
	char line[count + 1];
	strncpy(pCh, lineBuffer, (count + 1));
	free(lineBuffer);
//	return constLine;
}

BOOST_AUTO_TEST_CASE(chain_manager_test)
{
//	boost::thread_group threadGroup;
//	litecoin::CLiteCoinServer::instance().start(threadGroup);
//	threadGroup.join_all();
	int64_t begin_time = GetTime();
	//LogPrintf("load header file begin time:%d \n",DateTimeStrFormat("%H:%M:%S", GetTime()).c_str());
	boost::filesystem::path litecoinfile= GetDataDir()/"litecoinheader.log";
	try{
		FILE * file = fopen(litecoinfile.string().c_str(), "r+");
		if(!file)
		{
			LogPrintf("open litecoinheader file error!");
		}
		vector<CBitcoinBlock> vLitecoinBlocks;
		while(!feof(file))
		{
			CBitcoinBlock block;
			char pCh[4096];
			ReadLine(file, pCh);
			string strTemp(pCh);
			size_t begin = strTemp.find('(');
			size_t end = strTemp.find(')');
			if(begin == std::string::npos || end == std::string::npos)
				continue;
			string linebuffer = strTemp.substr(begin+1, end-begin-1);
//			LogPrintf("read line substring:%s\n",linebuffer.c_str());
			vector <string> fields;
			vector <string> items;
			split(fields, linebuffer, is_any_of( ", " ), token_compress_on);
			for(int i=0; i<fields.size(); ++i)
			{
				//LogPrintf("split1:%s\n",fields[i]);
				items.clear();
				split(items, fields[i], is_any_of("="));
				//LogPrintf("split item[0]:%s, item[1]:%s\n",items[0],items[1]);
				if(2 != items.size())
					continue;
				if("ver" == items[0])
				{
					block.nVersion = atoi(items[1].c_str());
				}
				if("hashPrevBlock" == items[0])
				{
					block.hashPrevBlock = uint256(items[1].c_str());
				}
				if("hashMerkleRoot" == items[0])
				{
					block.hashMerkleRoot = uint256(items[1].c_str());
				}
				if("nTime" == items[0])
				{
					block.nTime = atoi(items[1].c_str());
				}
				if("nBits" == items[0])
				{
					vector<unsigned char> vCh = ParseHex(items[1]);
					block.nBits = vCh[0]<<24 | vCh[1]<<16 | vCh[2]<<8 | vCh[3];
				}
				if("nNonce" == items[0])
				{
					block.nNonce = atoi(items[1].c_str());
				}
			}
			//block.print();
			vLitecoinBlocks.push_back(block);

		}
		fclose(file);

		//LogPrintf("load header file end time:%d \n",DateTimeStrFormat("%H:%M:%S", GetTime()).c_str());
		LogPrintf("load header file cost time:%d seconds\n",(GetTime()-begin_time));
		//CChainManager::CreateChainManagerInstance();
		CChainManager::CreateChainManagerInstance().AddBitcoinHeader(vLitecoinBlocks);

	}
	catch(std::exception &e)
	{
		LogPrintf("open file litecoinheader.log exception:%s", e.what());
	}
}

BOOST_AUTO_TEST_CASE(chain_manager_sync)
{
	boost::thread_group threadGroup;
	Bitcoin::CBitcoinServer::instance().start(threadGroup);
	threadGroup.join_all();
//	CChainManager::CreateChainManagerInstance();
//	Base b;
//	b.f();
//	Derive d;
//	d.f();
//
//	CLiteCoinDiskBlockIndex discBlockIndex;
//	cout << discBlockIndex.GetBlockHash().ToString() << endl;
}

BOOST_AUTO_TEST_CASE(chain_manager_load)
{
	CChainManager::CreateChainManagerInstance();
}

BOOST_AUTO_TEST_SUITE_END()
