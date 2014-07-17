/*
 * CBitcoinBlock.h
 *
 *  Created on: Jun 12, 2014
 *      Author: ranger.shi
 */

#ifndef CBITCOINBLOCK_H_
#define CBITCOINBLOCK_H_

#include "script.h"
#include "serialize.h"
#include "uint256.h"
#include "core.h"

namespace Bitcoin {

class CBitcoinBlockHeader
{
public:
    // header
    static const int CURRENT_VERSION=2;
    int nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    CBitcoinBlockHeader()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    )

    void SetNull()
    {
        nVersion = CBitcoinBlockHeader::CURRENT_VERSION;
        hashPrevBlock = 0;
        hashMerkleRoot = 0;
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};

class CBitcoinBlock : public CBitcoinBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;

    // memory only
    mutable std::vector<uint256> vMerkleTree;

    CBitcoinBlock()
    {
        SetNull();
    }

    CBitcoinBlock(const CBitcoinBlockHeader &header)
    {
        SetNull();
        *((CBitcoinBlockHeader*)this) = header;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(*(CBitcoinBlockHeader*)this);
        READWRITE(vtx);
    )

    void SetNull()
    {
    	CBitcoinBlockHeader::SetNull();
        vtx.clear();
        vMerkleTree.clear();
    }

    CBitcoinBlockHeader GetBlockHeader() const
    {
    	CBitcoinBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        return block;
    }

    uint256 BuildMerkleTree() const;

    void print() const;
};

} /* namespace Bitcoin */

#endif /* CBitcoinBLOCK_H_ */
