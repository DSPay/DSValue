/*
 * CChainManger.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#include "CBitcoinBlock.h"
#include "CChainManager.h"
#include "CBitcoinCheckpoints.h"
#include "blockhash/hashblock.h"
#include <boost/algorithm/string/replace.hpp>
#include "txdb.h"

#include "ui_interface.h"
namespace Bitcoin {

bool fTestNet = false;

CChainManager::CChainManager() :
		nBlockTreeDBCache(((unsigned int) 1 << 21)), bFork(false), Bitcoinpath(GetDataDir() / "blocks" / "btcindex"), bnProofOfWorkLimit(
				~uint256(0) >> 32), nTargetTimespan(14 * 24 * 60 * 60), nTargetSpacing(10 * 60), nInterval(
				nTargetTimespan / nTargetSpacing)
{
	pBitcoinblocktree = NULL;
	nBlockSequenceId = 1;
	fBitcoinLargeWorkForkFound = false;
    fBitcoinLargeWorkInvalidChainFound = false;
    pindexBestForkBitcoinTip = NULL;
    pindexBestForkBitcoinBase = NULL;
    pindexGenesisBlock = NULL;
	setBitcoinBlockIndexValid.clear();
	pindexBestInvalidBitcoin = NULL;
	mapBitcoinBlockIndex.clear();
	chainActiveBitcoin.SetTip(NULL);
	chainMostWorkBitcoin.SetTip(NULL);
	mapOrphanBitcoinBlocks.clear();

	pBitcoinblocktree = new CBlockTreeDB(Bitcoinpath, nBlockTreeDBCache);
	if(!LoadBitcoinBlockIndex())
	{
		LogTrace("initialize Bitcoin header:","LoadBitcoinBlockIndex error!\n");
		return;
	}
	LogTrace("Bitcoin","===mapBitcoinBlockIndex size:%d\n",mapBitcoinBlockIndex.size());
	if(!InitBlockIndex())
	{
		LogTrace("initialize Bitcoin header:","InitBlockIndex error!\n");
		return;
	}
	LogTrace("Bitcoin","CChainManager::GetTip,Hash:%s\n",chainActiveBitcoin.Tip()->GetBlockHash().ToString());
}

CChainManager::~CChainManager()
{
	if(NULL != pBitcoinblocktree)
		delete pBitcoinblocktree;

	BOOST_FOREACH(const PAIRTYPE(uint256, COrphanBlock*) &item, mapOrphanBitcoinBlocks){
		if(NULL != item.second){
			delete item.second;
		}
	}
	BOOST_FOREACH(const PAIRTYPE(uint256, CBlockIndex*) &item, mapBitcoinBlockIndex){
			if(NULL != item.second){
				delete item.second;
			}
		}
}

bool CChainManager::CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    CBigNum bnTarget;
    bnTarget.SetCompact(nBits);

    // Check range
    if (bnTarget <= 0 || bnTarget > bnProofOfWorkLimit)
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget.getuint256())
        return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}

bool CChainManager::InitBlockIndex()
{
	if (pindexGenesisBlock != NULL)
	        return true;
	LogTrace("Bitcoin","initialize genesis block info\n");
	const char* pszTimestamp = "NY Times 05/Oct/2011 Steve Jobs, Apple’s Visionary, Dies at 56";
	CTransaction txNew;
	txNew.vin.resize(1);
	txNew.vout.resize(1);
	txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
	txNew.vout[0].nValue = 50 * COIN;
	txNew.vout[0].scriptPubKey = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
	CBitcoinBlock block;
	block.vtx.push_back(txNew);
	block.hashPrevBlock = hashBitcoinStartPreBlock;
	block.hashMerkleRoot = hashBitcoinStartMerkleRoot;
	block.nVersion = 2;
	block.nTime    = 1404029522;
	block.nBits    = 0x1851aba2;
	block.nNonce   = 721885384;

	///////////////Bitcoin genesis block///////////////////////
//	block.hashPrevBlock = 0;
//	block.hashMerkleRoot = block.BuildMerkleTree();
//	block.nVersion = 1;
//	block.nTime    = 1317972665;
//	block.nBits    = 0x1e0ffff0;
//	block.nNonce   = 2084524493;

	//// debug print
	uint256 hash = block.GetHash();
	LogTrace("Bitcoin","%s\n", hash.ToString().c_str());
	LogTrace("Bitcoin","%s\n", hashBitcoinStartBlock.ToString().c_str());
	LogTrace("Bitcoin","%s\n", block.hashMerkleRoot.ToString().c_str());
	block.print();
	Assert(hash == hashBitcoinStartBlock);

	// Start new block file
	try {
		unsigned int nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
		CDiskBlockPos blockPos;
		CValidationState state;
		if (!AddToBlockIndex(block, state))
			return error("LoadBlockIndex() : genesis block not accepted");
	} catch(std::runtime_error &e) {
		return error("LoadBlockIndex() : failed to initialize block database: %s", e.what());
	}
	return true;
}

CChainManager &CChainManager::CreateChainManagerInstance()
{

	static CChainManager instance;
	return instance;
}

bool CChainManager::AddBitcoinHeader(const std::vector<CBitcoinBlock> &vBitcoinHeader)
{
	int nHeight = chainActiveBitcoin.Height();
	BOOST_FOREACH(CBitcoinBlock item, vBitcoinHeader)
	{
		CValidationState state;
		ProcessBitcoinHeader(state, item);
	}
	if(chainActiveBitcoin.Height() > nHeight)
	{
		LogPrintf("update btc hash, activate best chain\n");
		CValidationState state;
		ActivateBestChain(state);
	}
	return true;
}

bool CChainManager::ProcessBitcoinHeader(CValidationState &state, CBitcoinBlock &block)
{
    // Check for duplicate
	uint256 hash = block.GetHash();
	if(mapBitcoinBlockIndex.count(block.GetHash()))
	{
		return state.Invalid(error("ProcessBitcoinHeader() : already have block %d %s", mapBitcoinBlockIndex[hash]->nHeight, hash.ToString()), 0, "duplicate");
	}
    //Preliminary checks
	//1) Check proof of work matches claimed amount
	if(!CheckProofOfWork(hash, block.nBits))
	        return state.DoS(50, error("ProcessBitcoinHeader() : proof of work failed"), REJECT_INVALID, "high-hash");
	//2) Check timestamp
	if (block.GetBlockTime() > GetAdjustedTime() + 2 * 60 * 60)
	        return state.Invalid(error("ProcessBitcoinHeader() : block timestamp too far in the future"));

	//3) Check timestamp against prev
	CBlockIndex* pcheckpoint = BitcoinCheckpoints::GetLastCheckpoint(mapBitcoinBlockIndex);
	if (pcheckpoint && block.hashPrevBlock != (chainActiveBitcoin.Tip() ? chainActiveBitcoin.Tip()->GetBlockHash() : uint256(0)))
	{
		// Extra checks to prevent "fill up memory by spamming with bogus blocks"
		int64_t deltaTime = block.GetBlockTime() - pcheckpoint->nTime;
		if (deltaTime < 0)
		{
			return state.DoS(100, error("ProcessBitcoinHeader() : block with timestamp before last checkpoint"),
							 REJECT_CHECKPOINT, "time-too-old");
		}
		CBigNum bnNewBlock;
		bnNewBlock.SetCompact(block.nBits);
		CBigNum bnRequired;
		bnRequired.SetCompact(ComputeMinWork(pcheckpoint->nBits, deltaTime));
		if (bnNewBlock > bnRequired)
		{
			return state.DoS(100, error("ProcessBitcoinHeader() : block with too little proof-of-work"),
							 REJECT_INVALID, "bad-diffbits");
		}
	}

	// If we don't already have its previous block, shunt it off to holding area until we get it
	if (block.hashPrevBlock != 0 && !mapBitcoinBlockIndex.count(block.hashPrevBlock) && block.hashPrevBlock != hashBitcoinStartBlock)
	{
		LogTrace("Bitcoin","mapBitcoinBlockIndex size:%d\n", mapBitcoinBlockIndex.size());
		LogTrace("Bitcoin","ProcessBitcoinHeader: ORPHAN BLOCK %lu, prev=%s\n", (unsigned long)mapOrphanBitcoinBlocks.size(), block.hashPrevBlock.ToString());
//		PruneBitcoinOrphanBlocks();
//		COrphanBlock* pblock2 = new COrphanBlock();
//		{
//			CDataStream ss(SER_DISK, CLIENT_VERSION);
//			ss << block;
//			pblock2->vchBlock = std::vector<unsigned char>(ss.begin(), ss.end());
//		}
//		pblock2->hashBlock = hash;
//		pblock2->hashPrev = block.hashPrevBlock;
//		mapOrphanBitcoinBlocks.insert(make_pair(hash, pblock2));
//		mapOrphanBitcoinBlocksByPrev.insert(make_pair(pblock2->hashPrev, pblock2));

		return true;
	}

	if(!AcceptBlock(block,state))
		return false;

	// Recursively process any orphan blocks that depended on this one
//	vector<uint256> vWorkQueue;
//	vWorkQueue.push_back(hash);
//	for (unsigned int i = 0; i < vWorkQueue.size(); i++)
//	{
//		uint256 hashPrev = vWorkQueue[i];
//		for (multimap<uint256, COrphanBlock*>::iterator mi = mapOrphanBitcoinBlocksByPrev.lower_bound(hashPrev);
//			 mi != mapOrphanBitcoinBlocksByPrev.upper_bound(hashPrev);
//			 ++mi)
//		{
//			CBitcoinBlock block;
//			{
//				CDataStream ss(mi->second->vchBlock, SER_DISK, CLIENT_VERSION);
//				ss >> block;
//			}
//			// Use a dummy CValidationState so someone can't setup nodes to counter-DoS based on orphan resolution (that is, feeding people an invalid block based on LegitBlockX in order to get anyone relaying LegitBlockX banned)
//			CValidationState stateDummy;
//			if (AcceptBlock(block, stateDummy))
//				vWorkQueue.push_back(mi->second->hashBlock);
//			mapOrphanBitcoinBlocks.erase(mi->second->hashBlock);
//			delete mi->second;
//		}
//		mapOrphanBitcoinBlocksByPrev.erase(hashPrev);
//	}
	return true;
}

bool CChainManager::LoadBitcoinBlockIndex()
{
	if (!pBitcoinblocktree->LoadBlockIndexGuts(*this))
		return false;

	// Calculate nChainWork
	vector<pair<int, CBlockIndex*> > vSortedByHeight;
	vSortedByHeight.reserve(mapBitcoinBlockIndex.size());
	LogTrace("Bitcoin","mapBitcoinBlockIndex size:%d\n",mapBitcoinBlockIndex.size());
	BOOST_FOREACH(const PAIRTYPE(uint256, CBlockIndex*)& item, mapBitcoinBlockIndex)
	{
		CBlockIndex* pindex = item.second;
		vSortedByHeight.push_back(make_pair(pindex->nHeight, pindex));
	}
	sort(vSortedByHeight.begin(), vSortedByHeight.end());
	BOOST_FOREACH(const PAIRTYPE(int, CBlockIndex*)& item, vSortedByHeight)
	{
		CBlockIndex* pindex = item.second;
		pindex->nChainWork = (pindex->pprev ? pindex->pprev->nChainWork : 0) + pindex->GetBlockWork().getuint256();
		pindex->nChainTx = (pindex->pprev ? pindex->pprev->nChainTx : 0) + pindex->nTx;
		if ((pindex->nStatus & BLOCK_VALID_MASK) >= BLOCK_VALID_TRANSACTIONS && !(pindex->nStatus & BLOCK_FAILED_MASK))
			setBitcoinBlockIndexValid.insert(pindex);
		if (pindex->nStatus & BLOCK_FAILED_MASK && (!pindexBestInvalidBitcoin || pindex->nChainWork > pindexBestInvalidBitcoin->nChainWork))
			pindexBestInvalidBitcoin = pindex;
	}

	// initialize genesis block index
	if(mapBitcoinBlockIndex.count(hashBitcoinStartBlock))
	{
		pindexGenesisBlock = mapBitcoinBlockIndex[hashBitcoinStartBlock];
	}

//	// Check whether we need to continue reindexing
//	bool fReindexing = false;
//	pBitcoinblocktree->ReadReindexing(fReindexing);
//	fReindex |= fReindexing;
	uint256 bestHash(0);
	if(!pBitcoinblocktree->ReadBestBlockHash(bestHash))
	{
		return true;
	}
	// Load pointer to end of best chain
	std::map<uint256, CBlockIndex*>::iterator it = mapBitcoinBlockIndex.find(bestHash);
	if (it == mapBitcoinBlockIndex.end())
		return true;

	LOCK(cs_chainActiveBitcoin);
	chainActiveBitcoin.SetTip(it->second);
	LogTrace("Bitcoin","LoadBlockIndexDB(): hashBestChain=%s height=%d date=%s progress=%f\n",
		chainActiveBitcoin.Tip()->GetBlockHash().ToString(), chainActiveBitcoin.Height(),
		DateTimeStrFormat("%Y-%m-%d %H:%M:%S", chainActiveBitcoin.Tip()->GetBlockTime()),
		BitcoinCheckpoints::GuessVerificationProgress(chainActiveBitcoin.Tip()));

    return true;
}

uint256 CChainManager::GetBitcoinPowHash(const CBitcoinBlock &block)
{
	std::vector<unsigned char> vHashStr;
	vHashStr.insert(vHashStr.end(),BEGIN(block.nVersion),END(block.nNonce));
	return LtcoinBlockHash(&vHashStr[0], &vHashStr[vHashStr.size()-1]);
}

void CChainManager::PruneBitcoinOrphanBlocks()
{
    if (mapOrphanBitcoinBlocksByPrev.size() <= MAX_ORPHAN_BLOCKS)
        return;

    // Pick a random orphan block.
    int pos = insecure_rand() % mapOrphanBitcoinBlocksByPrev.size();
    std::multimap<uint256, COrphanBlock*>::iterator it = mapOrphanBitcoinBlocksByPrev.begin();
    while (pos--) it++;

    // As long as this block has other orphans depending on it, move to one of those successors.
    do {
        std::multimap<uint256, COrphanBlock*>::iterator it2 = mapOrphanBitcoinBlocksByPrev.find(it->second->hashBlock);
        if (it2 == mapOrphanBitcoinBlocksByPrev.end())
            break;
        it = it2;
    } while(1);

    uint256 hash = it->second->hashBlock;
    delete it->second;
    mapOrphanBitcoinBlocksByPrev.erase(it);
    mapOrphanBitcoinBlocks.erase(hash);
}

CBlockLocator CChainManager::GetTipBlockLocator()
{
	 int nStep = 1;
	 std::vector<uint256> vHave;
	 vHave.reserve(32);
	 CBlockIndex *pindex = chainActiveBitcoin.Tip();
	 if(NULL == pindex)
	 {
		 //vHave.push_back(uint256("26d696987eda4c92e8989fb936f33c1f6a984e69e1819d4b3c5a544b0b58a033"));
		 vHave.push_back(hashBitcoinStartBlock);
		 return CBlockLocator(vHave);
	 }
	 while (pindex) {
		vHave.push_back(pindex->GetBlockHash());
		// Stop when we have added the genesis block.
		if(pindex->nHeight==BTC_START_HIGHT)
		//if (pindex->nHeight == 0)
			break;
		// Exponentially larger steps back, plus the genesis block.
		int nHeight = std::max(pindex->nHeight - nStep, 0);
		// In case pindex is not in this chain, iterate pindex->pprev to find blocks.
		while (pindex->nHeight > nHeight && !IsHaveBitcoinBlock(*(pindex->phashBlock)))
			pindex = pindex->pprev;
		// If pindex is in this chain, use direct height-based access.
		if (pindex->nHeight > nHeight)
			pindex = chainActiveBitcoin[nHeight];
		if (vHave.size() > 10)
			nStep *= 2;
	 }
	 return CBlockLocator(vHave);
}

uint256 CChainManager::GetOrphanRootHash()
{
	if(mapOrphanBitcoinBlocks.empty())
		return uint256(0);
	uint256 hash = mapOrphanBitcoinBlocks.begin()->first;
	// Work back to the first block in the orphan chain
	do {
		map<uint256, COrphanBlock*>::iterator it2 = mapOrphanBitcoinBlocks.find(hash);
		if (it2 == mapOrphanBitcoinBlocks.end())
			return hash;
		hash = it2->first;
	} while(true);
}

unsigned int CChainManager::GetNextWorkRequired(const CBlockIndex* pindexLast, const CBitcoinBlock *pblock)
{
	unsigned int nProofOfWorkLimit = bnProofOfWorkLimit.GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    unsigned int nBits = 0x18415fd1;
	if (((pindexLast->nHeight + 1) % nInterval == 0) && (((pindexLast->nHeight + 1) - BTC_START_HIGHT) < nInterval))
		return nBits;
    // Only change once per interval
    if ((pindexLast->nHeight+1) % nInterval != 0)
    {
        // Special difficulty rule for testnet:
        if (fTestNet)
        {
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }

        return pindexLast->nBits;
    }

    // Bitcoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = nInterval-1;
//    if ((pindexLast->nHeight+1) != nInterval)
//        blockstogoback = nInterval;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
    {
        pindexFirst = pindexFirst->pprev;
    }

    Assert(pindexFirst);

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    LogTrace("Bitcoin","  nActualTimespan = %ld before bounds\n", nActualTimespan);
    if (nActualTimespan < nTargetTimespan/4)
        nActualTimespan = nTargetTimespan/4;
    if (nActualTimespan > nTargetTimespan*4)
        nActualTimespan = nTargetTimespan*4;

    // Retarget
    CBigNum bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnProofOfWorkLimit)
        bnNew = bnProofOfWorkLimit;

    /// debug print
    LogTrace("Bitcoin","GetNextWorkRequired RETARGET\n");
    LogTrace("Bitcoin","nTargetTimespan = %d    nActualTimespan = %d\n", nTargetTimespan, nActualTimespan);
    LogTrace("Bitcoin","Before: %08x  %s\n", pindexLast->nBits, CBigNum().SetCompact(pindexLast->nBits).getuint256().ToString().c_str());
    LogTrace("Bitcoin","After:  %08x  %s\n", bnNew.GetCompact(), bnNew.getuint256().ToString().c_str());

    return bnNew.GetCompact();
}

bool CChainManager::AcceptBlock(CBitcoinBlock &block, CValidationState &state)
{
	// Get prev block index
	uint256 hash = block.GetHash();
	CBlockIndex* pindexPrev = NULL;
	int nHeight = 0;
	if(hash != hashBitcoinStartBlock)
	{
		map<uint256, CBlockIndex*>::iterator mi = mapBitcoinBlockIndex.find(block.hashPrevBlock);
		if (mi == mapBitcoinBlockIndex.end() && block.hashPrevBlock != hashBitcoinStartBlock)
			   return state.DoS(10, error("ProcessBitcoinHeader() : prev block not found"), 0, "bad-prevblk");
		pindexPrev = (*mi).second;
		nHeight = pindexPrev->nHeight+1;
		//4) Check proof of work
		if (block.nBits != GetNextWorkRequired(pindexPrev, &block))
			return state.DoS(100, error("ProcessBitcoinHeader() : incorrect proof of work"),
							 REJECT_INVALID, "bad-diffbits");
		//5) Check timestamp against prev
		if (block.GetBlockTime() <= pindexPrev->GetMedianTimePast())
			return state.Invalid(error("ProcessBitcoinHeader() : block's timestamp is too early"),
								 REJECT_INVALID, "time-too-old");
		//6) Check that the block chain matches the known block chain up to a checkpoint
		if (!BitcoinCheckpoints::CheckBlock(nHeight, hash))
			return state.DoS(100, error("ProcessBitcoinHeader() : rejected by checkpoint lock-in at %d", nHeight));

		//7) Don't accept any forks from the main chain prior to last checkpoint
		CBlockIndex* pcheckpoint = BitcoinCheckpoints::GetLastCheckpoint(mapBitcoinBlockIndex);
		if (pcheckpoint && nHeight < pcheckpoint->nHeight)
			return state.DoS(100, error("ProcessBitcoinHeader() : forked chain older than last checkpoint (height %d)", nHeight));

		//8) Reject block.nVersion=1 blocks when 95% (75% on testnet) of the network has upgraded:
		if (block.nVersion < 2)
		{
			if ((!fTestNet && CBlockIndex::IsSuperMajority(2, pindexPrev, 950, 1000)) ||
				(fTestNet && CBlockIndex::IsSuperMajority(2, pindexPrev, 75, 100)))
			{
				return state.Invalid(error("AcceptBlock() : rejected nVersion=1 block"));
			}
		}
	}
	if(!AddToBlockIndex(block, state))
		return false;
	return true;
}

bool CChainManager::AddToBlockIndex(CBitcoinBlock &block, CValidationState &state)
{
	// Construct new block index object
	uint256 hash = block.GetHash();
	CBlockIndex* pindexNew = new CBlockIndex();
	pindexNew->nVersion       = block.nVersion;
	pindexNew->hashMerkleRoot = block.hashMerkleRoot;
	pindexNew->nTime          = block.nTime;
	pindexNew->nBits          = block.nBits;
	pindexNew->nNonce         = block.nNonce;
	{
		 LOCK(cs_nBlockSequenceId);
		 pindexNew->nSequenceId = nBlockSequenceId++;
	}
	Assert(pindexNew);

	map<uint256, CBlockIndex*>::iterator mi = mapBitcoinBlockIndex.insert(make_pair(hash, pindexNew)).first;
	pindexNew->phashBlock = &((*mi).first);
	map<uint256, CBlockIndex*>::iterator miPrev = mapBitcoinBlockIndex.find(block.hashPrevBlock);
	if (miPrev != mapBitcoinBlockIndex.end())
	{
		pindexNew->pprev = (*miPrev).second;
		pindexNew->nHeight = pindexNew->pprev->nHeight + 1;
	}
	if(hash == hashBitcoinStartBlock)
	{
		pindexNew->nHeight = BTC_START_HIGHT;
	}
	pindexNew->nTx = block.vtx.size();
	pindexNew->nChainWork = (pindexNew->pprev ? pindexNew->pprev->nChainWork : 0) + pindexNew->GetBlockWork().getuint256();
	pindexNew->nChainTx = (pindexNew->pprev ? pindexNew->pprev->nChainTx : 0) + pindexNew->nTx;

	pindexNew->nUndoPos = 0;
	pindexNew->nStatus = BLOCK_VALID_TRANSACTIONS | BLOCK_HAVE_DATA;

	setBitcoinBlockIndexValid.insert(pindexNew);

	// New best?
	if (!ConnectBestBlock(state))
		return false;

	if (pindexNew == chainActiveBitcoin.Tip())
	{
		// Clear fork warning if its no longer applicable
		CheckForkWarningConditions();
	} else
		CheckForkWarningConditionsOnNewFork(pindexNew);

	if (!pBitcoinblocktree->Flush())
		return state.Abort(_("Failed to sync block index"));
	return true;
}

bool CChainManager::ConnectBestBlock(CValidationState &state)
{
	 CBlockIndex *pindexOldTip = chainActiveBitcoin.Tip();
	 bool fComplete = false;
	 while (!fComplete) {
			FindMostWorkChain();
			fComplete = true;

			// Check whether we have something to do.
			if (chainMostWorkBitcoin.Tip() == NULL) break;

			// Disconnect active blocks which are no longer in the best chain.
			while (chainActiveBitcoin.Tip() && !chainMostWorkBitcoin.Contains(chainActiveBitcoin.Tip())) {
				if (!DisconnectTip(state))
					return false;
			}

			// Connect new blocks.
			while (!chainActiveBitcoin.Contains(chainMostWorkBitcoin.Tip())) {
				CBlockIndex *pindexConnect  = NULL;
				if(NULL == chainActiveBitcoin.Tip())
				{
					pindexConnect = chainMostWorkBitcoin[chainActiveBitcoin.Height() + 1 + BTC_START_HIGHT];
				}
				else
					pindexConnect = chainMostWorkBitcoin[chainActiveBitcoin.Height() + 1];

				if (!ConnectTip(pindexConnect, state)) {
					if (state.IsInvalid()) {
						// The block violates a consensus rule.
						if (!state.CorruptionPossible())
							InvalidChainFound(chainMostWorkBitcoin.Tip());
						fComplete = false;
						state = CValidationState();
						break;
					} else {
						// A system error occurred (disk space, database error, ...).
						return false;
					}
				}
			}
	}

	if (chainActiveBitcoin.Tip() != pindexOldTip) {
		std::string strCmd = GetArg("-blocknotify", "");
		if (!IsInitialBlockDownload() && !strCmd.empty())
		{
			boost::replace_all(strCmd, "%s", chainActiveBitcoin.Tip()->GetBlockHash().GetHex());
			boost::thread t(runCommand, strCmd); // thread runs free
		}
	}
	return true;
}

void CChainManager::FindMostWorkChain()
{
    CBlockIndex *pindexNew = NULL;

    // In case the current best is invalid, do not consider it.
    while (chainMostWorkBitcoin.Tip() && (chainMostWorkBitcoin.Tip()->nStatus & BLOCK_FAILED_MASK)) {
    	setBitcoinBlockIndexValid.erase(chainMostWorkBitcoin.Tip());
        chainMostWorkBitcoin.SetTip(chainMostWorkBitcoin.Tip()->pprev);
    }

    do {
        // Find the best candidate header.
        {
        	LogTrace("Bitcoin","setBitcoinBlockIndexValid size:%d\n",setBitcoinBlockIndexValid.size());
            std::set<CBlockIndex*, CBlockIndexWorkComparator>::reverse_iterator it = setBitcoinBlockIndexValid.rbegin();
            if (it == setBitcoinBlockIndexValid.rend())
                return;
            pindexNew = *it;
        }

        // Check whether all blocks on the path between the currently active chain and the candidate are valid.
        // Just going until the active chain is an optimization, as we know all blocks in it are valid already.
        CBlockIndex *pindexTest = pindexNew;
        bool fInvalidAncestor = false;
        while (pindexTest && !chainActiveBitcoin.Contains(pindexTest)) {
            if (pindexTest->nStatus & BLOCK_FAILED_MASK) {
                // Candidate has an invalid ancestor, remove entire chain from the set.
                if (pindexBestInvalidBitcoin == NULL || pindexNew->nChainWork > pindexBestInvalidBitcoin->nChainWork)
                	pindexBestInvalidBitcoin = pindexNew;
                CBlockIndex *pindexFailed = pindexNew;
                while (pindexTest != pindexFailed) {
                    pindexFailed->nStatus |= BLOCK_FAILED_CHILD;
                    setBitcoinBlockIndexValid.erase(pindexFailed);
                    pindexFailed = pindexFailed->pprev;
                }
                fInvalidAncestor = true;
                break;
            }
            pindexTest = pindexTest->pprev;
        }
        if (fInvalidAncestor)
            continue;
        break;
    } while(true);

    // Check whether it's actually an improvement.
    if (chainMostWorkBitcoin.Tip() && !CBlockIndexWorkComparator()(chainMostWorkBitcoin.Tip(), pindexNew))
        return;

    // We have a new best.
    chainMostWorkBitcoin.SetTip(pindexNew);
    Assert(chainMostWorkBitcoin.Tip());
}

// Disconnect chainActiveBitcoin's tip.
bool CChainManager::DisconnectTip(CValidationState &state)
{
    CBlockIndex *pindexDelete = chainActiveBitcoin.Tip();
    Assert(pindexDelete);
    if(pindexDelete->GetBlockHash()==hashBitcoinStartBlock)
    	return true;
    if(NULL == pindexDelete->pprev)
    	return true;
    bFork = true;
    LOCK(cs_chainActiveBitcoin);
    chainActiveBitcoin.SetTip(pindexDelete->pprev);
    LogTrace("Bitcoin",
			"UpdateTip: new best=%s  height=%d  log2_work=%.8g  tx=%lu  date=%s progress=%f\n",
			chainActiveBitcoin.Tip()->GetBlockHash().ToString(),
			chainActiveBitcoin.Height(),
			log(chainActiveBitcoin.Tip()->nChainWork.getdouble()) / log(2.0),
			(unsigned long )chainActiveBitcoin.Tip()->nChainTx,
			DateTimeStrFormat("%Y-%m-%d %H:%M:%S",
					chainActiveBitcoin.Tip()->GetBlockTime()),
					BitcoinCheckpoints::GuessVerificationProgress(
					chainActiveBitcoin.Tip()));
    return true;
}

// Connect a new block to chainActiveBitcoin.
bool CChainManager::ConnectTip(CBlockIndex *pindexNew, CValidationState &state)
{
	Assert(((pindexNew->GetBlockHash()) == hashBitcoinStartBlock) || pindexNew->pprev == chainActiveBitcoin.Tip());

    // Update chainActiveBitcoin & related variables.
    LOCK(cs_chainActiveBitcoin);
    chainActiveBitcoin.SetTip(pindexNew);
    LogTrace("UpdateTip",
    			"UpdateTip: new best=%s  height=%d  log2_work=%.8g  tx=%lu  date=%s progress=%f\n",
    			chainActiveBitcoin.Tip()->GetBlockHash().ToString(),
    			chainActiveBitcoin.Height(),
    			log(chainActiveBitcoin.Tip()->nChainWork.getdouble()) / log(2.0),
    			(unsigned long )chainActiveBitcoin.Tip()->nChainTx,
    			DateTimeStrFormat("%Y-%m-%d %H:%M:%S",
    					chainActiveBitcoin.Tip()->GetBlockTime()),
    			BitcoinCheckpoints::GuessVerificationProgress(
    					chainActiveBitcoin.Tip()));
    CBitcoinDiskBlockIndex blockindex(pindexNew);
    if(pindexNew->GetBlockHash() == hashBitcoinStartBlock)
    {
	   blockindex.hashPrev = hashBitcoinStartPreBlock;
    }
    if (!pBitcoinblocktree->WriteBlockIndex(blockindex))
    	return state.Abort(_("Failed to write block index"));

    if(!pBitcoinblocktree->WriteBestBlockHash(pindexNew->GetBlockHash()))
    		return state.Abort(_("Failed to write best block hash"));
    return true;
}

void CChainManager::InvalidChainFound(CBlockIndex* pindexNew)
{
    if (!pindexBestInvalidBitcoin || pindexNew->nChainWork > pindexBestInvalidBitcoin->nChainWork)
    {
    	pindexBestInvalidBitcoin = pindexNew;
        // The current code doesn't actually read the BestInvalidWork entry in
        // the block database anymore, as it is derived from the flags in block
        // index entry. We only write it for backward compatibility.
        pBitcoinblocktree->WriteBestInvalidWork(CBigNum(pindexBestInvalidBitcoin->nChainWork));
        uiInterface.NotifyBlocksChanged();
    }
	LogTrace("Bitcoin","InvalidChainFound: invalid block=%s  height=%d  log2_work=%.8g  date=%s\n",
	pindexNew->GetBlockHash().ToString(), pindexNew->nHeight,
	log(pindexNew->nChainWork.getdouble())/log(2.0), DateTimeStrFormat("%Y-%m-%d %H:%M:%S",
	pindexNew->GetBlockTime()));
	LogTrace("Bitcoin","InvalidChainFound:  current best=%s  height=%d  log2_work=%.8g  date=%s\n",
	chainActiveBitcoin.Tip()->GetBlockHash().ToString(), chainActiveBitcoin.Height(), log(chainActiveBitcoin.Tip()->nChainWork.getdouble())/log(2.0),
	DateTimeStrFormat("%Y-%m-%d %H:%M:%S", chainActiveBitcoin.Tip()->GetBlockTime()));

    CheckForkWarningConditions();
}

bool CChainManager::IsInitialBlockDownload()
{
    if (chainActiveBitcoin.Height() < BitcoinCheckpoints::GetTotalBlocksEstimate())
        return true;
    static int64_t nLastUpdate;
    static CBlockIndex* pindexLastBest;
    if (chainActiveBitcoin.Tip() != pindexLastBest)
    {
        pindexLastBest = chainActiveBitcoin.Tip();
        nLastUpdate = GetTime();
    }
    return (GetTime() - nLastUpdate < 10 &&
    		chainActiveBitcoin.Tip()->GetBlockTime() < GetTime() - 24 * 60 * 60);
}

void CChainManager::CheckForkWarningConditions()
{
    // Before we get past initial download, we cannot reliably alert about forks
    // (we assume we don't get stuck on a fork before the last checkpoint)
    if (IsInitialBlockDownload())
        return;

    // If our best fork is no longer within 72 blocks (+/- 12 hours if no one mines it)
    // of our head, drop it
    if (pindexBestForkBitcoinTip && chainActiveBitcoin.Height() - pindexBestForkBitcoinTip->nHeight >= 72)
        pindexBestForkBitcoinTip = NULL;

    if (pindexBestForkBitcoinTip || (pindexBestInvalidBitcoin && pindexBestInvalidBitcoin->nChainWork > chainActiveBitcoin.Tip()->nChainWork + (chainActiveBitcoin.Tip()->GetBlockWork() * 6).getuint256()))
    {
        if (!fBitcoinLargeWorkForkFound)
        {
            std::string strCmd = GetArg("-alertnotify", "");
            if (!strCmd.empty())
            {
                std::string warning = std::string("'Warning: Large-work fork detected, forking after block ") +
                                      pindexBestForkBitcoinBase->phashBlock->ToString() + std::string("'");
                boost::replace_all(strCmd, "%s", warning);
                boost::thread t(runCommand, strCmd); // thread runs free
            }
        }
        if (pindexBestForkBitcoinTip)
        {
            LogTrace("Bitcoin","CheckForkWarningConditions: Warning: Large valid fork found\n  forking the chain at height %d (%s)\n  lasting to height %d (%s).\nChain state database corruption likely.\n",
                   pindexBestForkBitcoinBase->nHeight, pindexBestForkBitcoinBase->phashBlock->ToString(),
                   pindexBestForkBitcoinTip->nHeight, pindexBestForkBitcoinTip->phashBlock->ToString());
            fBitcoinLargeWorkForkFound = true;
        }
        else
        {
            LogTrace("Bitcoin","CheckForkWarningConditions: Warning: Found invalid chain at least ~6 blocks longer than our best chain.\nChain state database corruption likely.\n");
            fBitcoinLargeWorkInvalidChainFound = true;
        }
    }
    else
    {
        fBitcoinLargeWorkForkFound = false;
        fBitcoinLargeWorkInvalidChainFound = false;
    }
}

void CChainManager::CheckForkWarningConditionsOnNewFork(CBlockIndex* pindexNewForkTip)
{
    // If we are on a fork that is sufficiently large, set a warning flag
    CBlockIndex* pfork = pindexNewForkTip;
    CBlockIndex* plonger = chainActiveBitcoin.Tip();
    while (pfork && pfork != plonger)
    {
        while (plonger && plonger->nHeight > pfork->nHeight)
            plonger = plonger->pprev;
        if (pfork == plonger)
            break;
        pfork = pfork->pprev;
    }

    // We define a condition which we should warn the user about as a fork of at least 7 blocks
    // who's tip is within 72 blocks (+/- 12 hours if no one mines it) of ours
    // We use 7 blocks rather arbitrarily as it represents just under 10% of sustained network
    // hash rate operating on the fork.
    // or a chain that is entirely longer than ours and invalid (note that this should be detected by both)
    // We define it this way because it allows us to only store the highest fork tip (+ base) which meets
    // the 7-block condition and from this always have the most-likely-to-cause-warning fork
    if (pfork && (!pindexBestForkBitcoinTip || (pindexBestForkBitcoinTip && pindexNewForkTip->nHeight > pindexBestForkBitcoinTip->nHeight)) &&
            pindexNewForkTip->nChainWork - pfork->nChainWork > (pfork->GetBlockWork() * 7).getuint256() &&
            chainActiveBitcoin.Height() - pindexNewForkTip->nHeight < 72)
    {
        pindexBestForkBitcoinTip = pindexNewForkTip;
        pindexBestForkBitcoinBase = pfork;
    }

    CheckForkWarningConditions();
}

CBlockIndex * CChainManager::InsertBitcoinBlockIndex(uint256 hash)
{
	 if (hash == 0)
	        return NULL;

	 if(hash == hashBitcoinStartPreBlock)
			return NULL;
	// Return existing
	map<uint256, CBlockIndex*>::iterator mi = mapBitcoinBlockIndex.find(hash);
	if (mi != mapBitcoinBlockIndex.end())
		return (*mi).second;

	// Create new
	CBlockIndex* pindexNew = new CBlockIndex();
	if (!pindexNew)
		throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");
	mi = mapBitcoinBlockIndex.insert(make_pair(hash, pindexNew)).first;
	pindexNew->phashBlock = &((*mi).first);

	return pindexNew;
}

CBitcoinDiskBlockIndex::CBitcoinDiskBlockIndex()
{
	 hashPrev = 0;
}

CBitcoinDiskBlockIndex::~CBitcoinDiskBlockIndex()
{

}

uint256 CBitcoinDiskBlockIndex::GetBlockHash() const
{
	CBitcoinBlockHeader block;
	block.nVersion        = nVersion;
	block.hashPrevBlock   = hashPrev;
	block.hashMerkleRoot  = hashMerkleRoot;
	block.nTime           = nTime;
	block.nBits           = nBits;
	block.nNonce          = nNonce;
	return block.GetHash();
}

std::string CBitcoinDiskBlockIndex::ToString() const
{

	std::string str = "CBitcoinDiskBlockIndex(";
	str += strprintf(" nHeight=%d hash=%s nVersion=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%d, nBits=%x, nNonce=%d)\n",
			nHeight, GetBlockHash().ToString(), nVersion, hashPrev.ToString(), hashMerkleRoot.ToString(), nTime, nBits, nNonce);
	return str;
}
} /* namespace Bitcoin */
