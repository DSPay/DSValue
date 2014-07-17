/*
 * CBitcoinNode.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CBitcoinNODE_H_
#define CBitcoinNODE_H_

#include "net.h"
#include "core.h"
#include "CBitcoinNetMessage.h"

namespace Bitcoin {

	class CBitcoinNode: public CNode
	{
		public:
			CBitcoinNode(SOCKET hSocketIn, CAddress addrIn, std::string addrNameIn = "", bool fInboundIn=false)
				:CNode(hSocketIn, addrIn, addrNameIn, fInboundIn)
			{
			}
			virtual ~CBitcoinNode()
			{
				m_recvBitcoinMsg.clear();
			}
		public:
			bool ReceiveMsgBytes(const char *pch, unsigned int nBytes);
			unsigned int GetTotalRecvSize();
			bool checkStatus();
			void parseMessage();
			void pushGetHeaders(uint256 hashEnd);
			void PushVersion();
		public:
			virtual void BeginMessage(const char* pszCommand) EXCLUSIVE_LOCK_FUNCTION(cs_vSend);
		public:
			std::deque<CBitcoinNetMessage> m_recvBitcoinMsg;
	};

} /* namespace Bitcoin */

#endif /* CBitcoinNODE_H_ */
