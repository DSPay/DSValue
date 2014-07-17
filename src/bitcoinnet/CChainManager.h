/*
 * CChainManger.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CCHAINMANGER_H_
#define CCHAINMANGER_H_

#include "main.h"
#include "bitcoinconfig.h"
#include "CBitcoinBlock.h"

namespace Bitcoin {

extern bool fTestNet;

struct CBlockIndexWorkComparator
{
    bool operator()(CBlockIndex *pa, CBlockIndex *pb) {
        // First sort by most total work, ...
        if (pa->nChainWork > pb->nChainWork) return false;
        if (pa->nChainWork < pb->nChainWork) return true;

        // ... then by earliest time received, ...
        if (pa->nSequenceId < pb->nSequenceId) return false;
        if (pa->nSequenceId > pb->nSequenceId) return true;

        // Use pointer address as tie breaker (should only happen with blocks
        // loaded from disk, as those all have id 0).
        if (pa < pb) return false;
        if (pa > pb) return true;

        // Identical blocks.
        return false;
    }
};

struct COrphanBlock {
    uint256 hashBlock;
    uint256 hashPrev;
    vector<unsigned char> vchBlock;
};

class CChainManager
{
	private:
		CChain chainMostWorkBitcoin;
		map<uint256, COrphanBlock*> mapOrphanBitcoinBlocks;
		multimap<uint256, COrphanBlock*> mapOrphanBitcoinBlocksByPrev;
		map<uint256, CBlockIndex*> mapBitcoinBlockIndex;
		CChainManager();

		CBlockTreeDB *pBitcoinblocktree;

		CCriticalSection cs_nBlockSequenceId;
		uint32_t nBlockSequenceId;

		CBlockIndex *pindexBestInvalidBitcoin;
		set<CBlockIndex*, CBlockIndexWorkComparator> setBitcoinBlockIndexValid;

		const CBigNum bnProofOfWorkLimit;

		const int64_t nTargetTimespan; // Bitcoin: 14 days
		const int64_t nTargetSpacing; 			// Bitcoin: 10 minutes
		const int64_t nInterval;

		bool fBitcoinLargeWorkForkFound;
		bool fBitcoinLargeWorkInvalidChainFound;
		CBlockIndex *pindexBestForkBitcoinTip, *pindexBestForkBitcoinBase;

		const size_t nBlockTreeDBCache;

		CBlockIndex *pindexGenesisBlock;

		const boost::filesystem::path Bitcoinpath;

		CCriticalSection cs_chainActiveBitcoin;

		CChain chainActiveBitcoin;

		bool bFork;

	public:
		~CChainManager();

		static CChainManager &CreateChainManagerInstance();

		bool AddBitcoinHeader(const std::vector<CBitcoinBlock> &vBitcoinHeader);

		CBlockLocator GetTipBlockLocator();

		uint256 GetOrphanRootHash();

		CBlockIndex *InsertBitcoinBlockIndex(uint256 hash);

		inline bool IsHaveBitcoinBlock(uint256 hash)
		{
			if(mapBitcoinBlockIndex.count(hash))
			{
				return true;
			}
			return false;
		}

		inline CBlockIndex *GetBitcoinChainTip()
		{
			LOCK(cs_chainActiveBitcoin);
			return chainActiveBitcoin.Tip();
		}

		inline CBlockIndex *GetBitcoinBlockIndex(int64_t nHeight)
		{
			LOCK(cs_chainActiveBitcoin);
			return chainActiveBitcoin[nHeight];
		}

		inline CBlockIndex *GetBitcoinBlockIndex(uint256 uHash)
		{
			return mapBitcoinBlockIndex[uHash];
		}

		inline void SetChainTip(CBlockIndex *pIndex) {
			LOCK(cs_chainActiveBitcoin);
			chainActiveBitcoin.SetTip(pIndex);
		}

		bool IsForkState() {
			return bFork;
		}

	    void setForkState(bool state) {
	    	bFork = state;
	    }
	private:

		bool InitBlockIndex();

		bool LoadBitcoinBlockIndex();

		bool ProcessBitcoinHeader(CValidationState &state, CBitcoinBlock &item);

		uint256 GetBitcoinPowHash(const CBitcoinBlock &block);

		void PruneBitcoinOrphanBlocks();

		unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBitcoinBlock *pblock);

		bool AcceptBlock(CBitcoinBlock &block, CValidationState &state);

		bool AddToBlockIndex(CBitcoinBlock &block, CValidationState &state);

		bool ConnectBestBlock(CValidationState &state);

		void FindMostWorkChain();

		bool DisconnectTip(CValidationState &state);

		bool ConnectTip(CBlockIndex *pindexNew, CValidationState &state);

		void InvalidChainFound(CBlockIndex* pindexNew);

		bool IsInitialBlockDownload();

		void CheckForkWarningConditionsOnNewFork(CBlockIndex* pindexNewForkTip);

		void CheckForkWarningConditions();

		bool CheckProofOfWork(uint256 hash, unsigned int nBits);


};


class CBitcoinDiskBlockIndex : public CBlockIndex
{

public:
	CBitcoinDiskBlockIndex();

	virtual ~CBitcoinDiskBlockIndex();
public:
    uint256 hashPrev;

    explicit CBitcoinDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex) {
        hashPrev = (pprev ? pprev->GetBlockHash() : 0);
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(VARINT(nHeight));

        // block header
        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    )

    uint256 GetBlockHash() const;

    std::string ToString() const;

    void print() const
    {
        LogPrintf("%s\n", ToString().c_str());
    }
};

} /* namespace Bitcoin */

#endif /* CCHAINMANGER_H_ */
