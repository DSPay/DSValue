/*
 * CNodeManager.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#include "CNodeManager.h"

namespace Bitcoin {

bool CNodeManager::insert(const boost::shared_ptr<CBitcoinNode>& node)
{
	bool ret = false;
	if (node && !node->fDisconnect)
	{
		ret = m_BitcoinNodes.insert(std::make_pair(node->addr.ToStringIP(), node)).second;
		if (ret)
		{
			node->AddRef();
		}
	}
	return ret;
}

void CNodeManager::sockectSelect()
{
	struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 50000; // frequency to poll pnode->vSend

	fd_set fdsetRecv;
	fd_set fdsetSend;
	fd_set fdsetError;
	FD_ZERO(&fdsetRecv);
	FD_ZERO(&fdsetSend);
	FD_ZERO(&fdsetError);
	SOCKET hSocketMax = 0;
	bool have_fds = false;

	typedef std::pair<const std::string, boost::shared_ptr<CBitcoinNode> > nodePair;
	BOOST_FOREACH(nodePair nodep, m_BitcoinNodes)
	{
		boost::shared_ptr<CBitcoinNode>& pnode = nodep.second;
		if (pnode->hSocket == INVALID_SOCKET)
		{
			continue;
		}

		FD_SET(pnode->hSocket, &fdsetError);
		hSocketMax = max(hSocketMax, pnode->hSocket);
		have_fds = true;
		if (!pnode->vSendMsg.empty())
		{
			FD_SET(pnode->hSocket, &fdsetSend);
			continue;
		}
		if (pnode->m_recvBitcoinMsg.empty()
			|| !pnode->m_recvBitcoinMsg.front().complete()
			|| pnode->GetTotalRecvSize() <= ReceiveFloodSize())
		{
			FD_SET(pnode->hSocket, &fdsetRecv);
		}
	}
	int nSelect = select(have_fds ? hSocketMax + 1 : 0,
						 &fdsetRecv, &fdsetSend, &fdsetError, &timeout);
	boost::this_thread::interruption_point();

	if (nSelect == SOCKET_ERROR)
	{
		FD_ZERO(&fdsetRecv);
		FD_ZERO(&fdsetSend);
		FD_ZERO(&fdsetError);
		MilliSleep(timeout.tv_usec/1000);
	}
	else
	{
		BOOST_FOREACH(nodePair nodep, m_BitcoinNodes)
		{
			boost::this_thread::interruption_point();
			boost::shared_ptr<CBitcoinNode>& pnode = nodep.second;
			if (pnode->hSocket == INVALID_SOCKET)
			{
				continue;
			}

			if (FD_ISSET(pnode->hSocket, &fdsetRecv)
				|| FD_ISSET(pnode->hSocket, &fdsetError))
			{
				// typical socket buffer is 8K-64K
				char pchBuf[0x10000];
				int nBytes = recv(pnode->hSocket, pchBuf, sizeof(pchBuf), MSG_DONTWAIT);
				if (nBytes > 0)
				{
					if (!pnode->ReceiveMsgBytes(pchBuf, nBytes))
					{
						pnode->CloseSocketDisconnect();
					}
					pnode->parseMessage();
					pnode->nLastRecv = GetTime();
					pnode->nRecvBytes += nBytes;
					pnode->RecordBytesRecv(nBytes);
				}
				else if (nBytes == 0)
				{
					// socket closed gracefully
					if (!pnode->fDisconnect)
						LogTrace("Bitcoin","net socket closed\n");
					pnode->CloseSocketDisconnect();
				}
				else if (nBytes < 0)
				{
					// error
					int nErr = WSAGetLastError();
					if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR && nErr != WSAEINPROGRESS)
					{
						if (!pnode->fDisconnect)
						{
							LogTrace("Bitcoin","socket recv error %d\n", nErr);
						}
						pnode->CloseSocketDisconnect();
					}
				}
			}
			if ((pnode->hSocket != SOCKET_ERROR) && FD_ISSET(pnode->hSocket, &fdsetSend))
			{
				SocketSendData(pnode.get());
			}
		}
	}
}

void CNodeManager::clearDisconnectNodes()
{
	typedef std::pair<const std::string, boost::shared_ptr<CBitcoinNode> > nodePair;
	std::vector<std::string> keys(m_BitcoinNodes.size());
	BOOST_FOREACH(nodePair& node, m_BitcoinNodes)
	{
		if (node.second->checkStatus())
		{
			keys.push_back(node.first);
		}
	}
	BOOST_FOREACH(std::string& key, keys)
	{
		m_BitcoinNodes.erase(key);
	}
}

void CNodeManager::startSync()
{
	typedef std::pair<const std::string, boost::shared_ptr<CBitcoinNode> > nodePair;
	boost::shared_ptr<CBitcoinNode> syncNode;
	int64_t bestScore = 0;
	BOOST_FOREACH(nodePair& node, m_BitcoinNodes)
	{
        if (!node.second->fClient && !node.second->fOneShot &&
            !node.second->fDisconnect && node.second->fSuccessfullyConnected &&
            (node.second->nVersion < NOBLKS_VERSION_START || node.second->nVersion >= NOBLKS_VERSION_END))
        {
            // every one minite sync
        	int64_t score = node.second->nLastRecv;
            if (!syncNode || (bestScore - score < 0))
            {
            	bestScore = score;
            	syncNode = node.second;
            }
        }
	}
	if (syncNode)
	{
		syncNode->pushGetHeaders(uint256(0));
	}
}

} /* namespace Bitcoin */
