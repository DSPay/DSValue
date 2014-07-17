#define BOOST_TEST_MODULE Bitcoin Test Suite



#include "main.h"
#include "txdb.h"
#include "ui_interface.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "db.h"
#include "wallet.h"
#endif
#include "bitcoinnet/CChainManager.h"
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>


CWallet* pwalletMain;

extern bool fPrintToConsole;
extern void noui_connect();

struct TestingSetup {
    CCoinsViewDB *pcoinsdbview;
    boost::filesystem::path pathTemp;
    boost::thread_group threadGroup;

    TestingSetup() {
        fPrintToDebugLog = TRUE; // don't want to write to debug.log file
        noui_connect();
#ifdef ENABLE_WALLET
        bitdb.MakeMock();
#endif
        mapArgs["-datadir"] = "d:\\bitcoin";
        ReadConfigFile(mapArgs, mapMultiArgs);
        CDebug::instance().Inital(mapMultiArgs);
        if (!SelectParamsFromCommandLine()) {
       		   fprintf(stderr, "Error: Invalid combination of -regtest and -testnet.\n");
       		   return ;
        }
        LogPrintf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		boost::filesystem::path blocksDir = GetDataDir() / "blocks";
		if (!boost::filesystem::exists(blocksDir)) {
			boost::filesystem::create_directory(blocksDir);

		}
        pblocktree = new CBlockTreeDB(1 << 20, false, false);
        pcoinsdbview = new CCoinsViewDB(1 << 23, false, false);
        pcoinsTip = new CCoinsViewCache(*pcoinsdbview);

        Bitcoin::CChainManager::CreateChainManagerInstance();
        LoadBlockIndex();
        InitBlockIndex();

#ifdef ENABLE_WALLET
        bool fFirstRun;
        pwalletMain = new CWallet("wallet.dat");
        pwalletMain->LoadWallet(fFirstRun);
        RegisterWallet(pwalletMain);
#endif
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        RegisterNodeSignals(GetNodeSignals());
    }
    ~TestingSetup()
    {
        threadGroup.interrupt_all();
        threadGroup.join_all();
        UnregisterNodeSignals(GetNodeSignals());
#ifdef ENABLE_WALLET
        delete pwalletMain;
        pwalletMain = NULL;
#endif
        delete pcoinsTip;
        delete pcoinsdbview;
        delete pblocktree;
#ifdef ENABLE_WALLET
        bitdb.Flush(true);
#endif
        boost::filesystem::remove_all(pathTemp);
    }
};

BOOST_GLOBAL_FIXTURE(TestingSetup);

void Shutdown(void* parg)
{
  exit(0);
}

void StartShutdown()
{
  exit(0);
}

