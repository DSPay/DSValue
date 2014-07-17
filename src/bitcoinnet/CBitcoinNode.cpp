/*
 * CBitcoinNode.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#include "CBitcoinNode.h"
#include "bitcoinconfig.h"
#include "CBitcoinServer.h"
#include "CBitcoinBlock.h"
#include "CChainManager.h"
#include <stdio.h>

namespace
{
	const unsigned int NET_MESSSAGE_HEADER_LEN	= 24;
}

namespace Bitcoin
{
	void CBitcoinNode::BeginMessage(const char* pszCommand) EXCLUSIVE_LOCK_FUNCTION(cs_vSend)
	{
        ENTER_CRITICAL_SECTION(cs_vSend);
        assert(ssSend.size() == 0);
        ssSend << CBitcoinMsgHeader(pszCommand, 0);
        LogTrace("net", "sending: %s ", pszCommand);
	}

	bool CBitcoinNode::ReceiveMsgBytes(const char *pch, unsigned int nBytes)
	{
		while (nBytes > 0)
		{
			// get current incomplete message, or create a new one
			if (m_recvBitcoinMsg.empty() || m_recvBitcoinMsg.back().complete())
			{
				m_recvBitcoinMsg.push_back(CBitcoinNetMessage(SER_NETWORK, nRecvVersion));
			}
			CBitcoinNetMessage& msg = m_recvBitcoinMsg.back();
			// absorb network data
			int handled;
			if (!msg.in_data)
			{
				handled = msg.readHeader(pch, nBytes);
			}
			else
			{
				handled = msg.readData(pch, nBytes);
			}
			if (handled < 0)
			{
				return false;
			}
			pch += handled;
			nBytes -= handled;
		}
		return true;
	}

	unsigned int CBitcoinNode::GetTotalRecvSize()
	{
		unsigned int length = 0;
        BOOST_FOREACH(const CBitcoinNetMessage &msg, m_recvBitcoinMsg)
		{
        	length += msg.vRecv.size() + NET_MESSSAGE_HEADER_LEN;
		}
		return length;
	}

	bool CBitcoinNode::checkStatus()
	{
		if (!fDisconnect)
		{
			if (nLastRecv && GetTime() - nLastRecv > 15 * 60
				&& m_recvBitcoinMsg.empty()
				&& nSendSize == 0
				&& ssSend.empty())
			{
				fDisconnect = true;
			}
		}
		return fDisconnect;
	}

	void CBitcoinNode::parseMessage()
	{
	    std::deque<CBitcoinNetMessage>::iterator it = m_recvBitcoinMsg.begin();
	    CAddrMan& liteAddrMan = CBitcoinServer::instance().getAddrMan();
	    for(;it != m_recvBitcoinMsg.end(); ++it)
	    {
	        // Don't bother if send buffer is too full to respond anyway
	    	bool nodeConnected = false;
	        if (nSendSize >= SendBufferSize())
	        {
	            break;
	        }
	        // get next message
	        CBitcoinNetMessage& msg = *it;
	        if (!msg.complete())
	        {
	            break;
	        }

	        // Read header
	        CBitcoinMsgHeader& hdr = msg.hdr;
	        if (!hdr.IsValid())
	        {
	            continue;
	        }
	        string strCommand = hdr.GetCommand();
	        // Message size
	        unsigned int nMessageSize = hdr.nMessageSize;

	        // Checksum
	        CDataStream& vRecv = msg.vRecv;
	        uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
	        unsigned int nChecksum = 0;
	        memcpy(&nChecksum, &hash, sizeof(nChecksum));
	        if (nChecksum != hdr.nChecksum)
	        {
	            continue;
	        }
	        LogTrace("print","command: %s\n", strCommand);
			if (strCommand == "version")
			{
				if (nVersion != 0)
				{
					break;
				}
				int64_t nTime;
				CAddress addrMe;
				CAddress addrFrom;
				uint64_t nNonce = 1;
				vRecv >> nVersion >> nServices >> nTime >> addrMe;
				if (nVersion < BITCOIN_MIN_PEER_PROTO_VERSION)
				{
					fDisconnect = true;
					break;
				}
				if (nVersion == 10300)
				{
					nVersion = 300;
				}
				if (!vRecv.empty())
				{
				   vRecv >> addrFrom >> nNonce;
				}
				if (!vRecv.empty())
				{
				   vRecv >> strSubVer;
				   cleanSubVer = SanitizeString(strSubVer);
				}
				if (!vRecv.empty())
				{
				   vRecv >> nStartingHeight;
				}
				if (!vRecv.empty())
				{
				   vRecv >> fRelayTxes; // set to true after we get the first filter* message
				}
				else
				{
				   fRelayTxes = true;
				}
				fClient = !(nServices & NODE_NETWORK);
				PushMessage("verack");
				ssSend.SetVersion(min(nVersion, PROTOCOL_VERSION));
				// Get recent addresses
				if (fOneShot || nVersion >= CADDR_TIME_VERSION)
				{
					PushMessage("getaddr");
					fGetAddr = true;
				}
				fSuccessfullyConnected = true;
				nodeConnected = true;
	        }
	        else if (nVersion == 0)
	        {
	        	continue;
	        }
	        else if (strCommand == "verack")
	        {
	        	SetRecvVersion(min(nVersion, PROTOCOL_VERSION));
	        }
	        else if (strCommand == "headers")
	        {
	        	std::vector<CBitcoinBlock> vHeaders;
	        	vRecv>>vHeaders;
	        	CChainManager& chain = CChainManager::CreateChainManagerInstance();
	        	chain.AddBitcoinHeader(vHeaders);
	        	nodeConnected = true;
	        }
	        else if(strCommand == "addr")
	        {
	            std::vector<CAddress> vAddr;
	            vRecv >> vAddr;
	            if ((nVersion < CADDR_TIME_VERSION && liteAddrMan.size() > 1000)
	            	|| vAddr.size() > 1000)
	            {
	            	continue;
	            }
	            // Store the new addresses
	            std::vector<CAddress> vAddrOk;
	            int64_t nNow = GetAdjustedTime();
	            int64_t nSince = nNow - 10 * 60;
	            BOOST_FOREACH(CAddress& caddr, vAddr)
	            {
	                if (caddr.nTime <= 100000000 || caddr.nTime > nNow + 10 * 60)
	                	caddr.nTime = nNow - 5 * 24 * 60 * 60;
	                AddAddressKnown(caddr);
	                if (IsReachable(caddr))
	                {
	                	vAddrOk.push_back(caddr);
	                }
	            }
	            liteAddrMan.Add(vAddrOk, this->addr, 2 * 60 * 60);
	            fGetAddr = false;
	            if (fOneShot)
	            {
	            	CloseSocketDisconnect();
	                fDisconnect = true;
	            }
	            nodeConnected = true;
	        }
	        else if(strCommand == "inv")
	        {
	            std::vector<CInv> vInv;
	            vRecv >> vInv;
	            if (vInv.size() > MAX_INV_SZ)
	            {
	            	continue;
	            }
	            BOOST_FOREACH(CInv& inv, vInv)
	            {
	            	CChainManager& chain = CChainManager::CreateChainManagerInstance();
	            	if (inv.type == MSG_BLOCK && !chain.IsHaveBitcoinBlock(inv.hash))
	            	{
	            		LogTrace("print","recieve a block ivn command: %s\n", inv.hash.ToString());
	            		pushGetHeaders(inv.hash);
	            	}
	            }
	        	nodeConnected = true;
	        }
	        else if (strCommand == "ping")
	        {
	            if (nVersion > BIP0031_VERSION)
	            {
	            	uint64_t nonce = 0;
	                vRecv >> nonce;
	                PushMessage("pong", nonce);
	            }
	        }
	        else
	        {
	        	continue;
	        }
			if (fNetworkNode && nodeConnected)
			{
				liteAddrMan.Connected(this->addr);
			}
	    }
	    m_recvBitcoinMsg.erase(m_recvBitcoinMsg.begin(), it);
	}

	void CBitcoinNode::PushVersion()
	{
		int nBestHeight = -1;
	    /// when NTP implemented, change to just nTime = GetAdjustedTime()
		int64_t nTime = (fInbound ? GetAdjustedTime() : GetTime());
	    CAddress addrYou = (addr.IsRoutable() && !IsProxy(addr) ? addr : CAddress(CService("0.0.0.0",0)));
	    CAddress addrMe = GetLocalAddress(&addr);
	    RAND_bytes((unsigned char*)&nLocalHostNonce, sizeof(nLocalHostNonce));
	    PushMessage("version", PROTOCOL_VERSION, nLocalServices, nTime, addrYou, addrMe,
	                nLocalHostNonce, FormatSubVersion(CLIENT_NAME, BITCOIN_CLIENT_VERSION, std::vector<string>()),
	                nBestHeight, true);
	}

	void CBitcoinNode::pushGetHeaders(uint256 hashEnd)
	{
		CChainManager& chain = CChainManager::CreateChainManagerInstance();
		if (pindexLastGetBlocksBegin != chain.GetBitcoinChainTip()
			|| hashEnd != hashLastGetBlocksEnd)
		{
			if (hashEnd == uint256(0))
			{
				hashEnd = chain.GetOrphanRootHash();
			}
			PushMessage("getheaders", chain.GetTipBlockLocator(), hashEnd);
			pindexLastGetBlocksBegin = chain.GetBitcoinChainTip();
			hashLastGetBlocksEnd = hashEnd;
		}
	}
} /* namespace Bitcoin */
