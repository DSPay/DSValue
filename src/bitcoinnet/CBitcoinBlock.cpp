/*
 * CBitcoinBlock.cpp
 *
 *  Created on: Jun 12, 2014
 *      Author: ranger.shi
 */

#include "CBitcoinBlock.h"

namespace Bitcoin {

uint256 CBitcoinBlockHeader::GetHash() const
{
    return Hash(BEGIN(nVersion), END(nNonce));
}

void CBitcoinBlock::print() const
{
    LogTrace("print","CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%"PRIszu")\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
    	 LogTrace("print","  ");
    	 LogTrace("print","%s", vtx[i].ToString());

    }
    LogTrace("print","  vMerkleTree: ");
    for (unsigned int i = 0; i < vMerkleTree.size(); i++)
    	LogTrace("print","%s ", vMerkleTree[i].ToString());
    LogTrace("print","\n");
}

uint256 CBitcoinBlock::BuildMerkleTree() const
{
    vMerkleTree.clear();
    BOOST_FOREACH(const CTransaction& tx, vtx)
        vMerkleTree.push_back(tx.GetHash());
    int j = 0;
    for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
    {
        for (int i = 0; i < nSize; i += 2)
        {
            int i2 = std::min(i+1, nSize-1);
            vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
                                       BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
        }
        j += nSize;
    }
    return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
}

} /* namespace Bitcoin */
