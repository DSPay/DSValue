/*
 * CNodeManager.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CNODEMANAGER_H_
#define CNODEMANAGER_H_

#include "CBitcoinNode.h"
#include <map>
#include <string>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>

namespace Bitcoin {

	class CNodeManager
	{
		public:
			CNodeManager()
			{
			}
			bool insert(const boost::shared_ptr<CBitcoinNode>& node);
			bool exist(const CService& addr) const
			{
				return m_BitcoinNodes.find(addr.ToStringIP()) != m_BitcoinNodes.end();
			}
			unsigned int getNodesSize() const
			{
				return m_BitcoinNodes.size();
			}
			void sockectSelect();
			void clearDisconnectNodes();
			void startSync();
		private:
			/* map[ip string, Bitcoin node] */
			std::map<std::string, boost::shared_ptr<CBitcoinNode> > m_BitcoinNodes;
	};

} /* namespace Bitcoin */

#endif /* CNODEMANAGER_H_ */
