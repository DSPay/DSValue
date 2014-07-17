/*
 * CBitcoinServer.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CBitcoinSERVER_H_
#define CBitcoinSERVER_H_

#include "CNodeManager.h"
#include "addrman.h"
#include <map>
#include <string>
#include <queue>

namespace Bitcoin {

	class CBitcoinServer
	{
		public:
			static CBitcoinServer& instance();
			bool start(boost::thread_group& threadGroup);
			CAddrMan& getAddrMan()
			{
				return m_BitcoinAddrMan;
			}
		private:
			CBitcoinServer();
			~CBitcoinServer();
		private:
			void BitcoinNetMaintain();
			bool getAddrFromDb();
			void getAddrFromDnsSeed();
			bool openNetworkConnection(CAddress& addrConnect, const char *strDest, SOCKET& hSocketRet);
			boost::shared_ptr<CBitcoinNode> createNode(const SOCKET& hSocket, const CAddress& addr);
		private:
			CNodeManager				m_nodesManager;
			CAddrMan					m_BitcoinAddrMan;
			std::deque<std::string> 	m_oneShot;
	};

} /* namespace Bitcoin */

#endif /* CNODESMANGER_H_ */
