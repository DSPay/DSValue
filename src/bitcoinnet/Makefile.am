include $(top_srcdir)/src/Makefile.include
noinst_LIBRARIES= libbitcoinnet.a

BTCOIN_NET_H = \
  CChainManager.h \
  CBitcoinMsgHeader.h \
  CBitcoinNode.h \
  CBitcoinServer.h \
  CBitcoinNetMessage.h \
  CNodeManager.h \
  CBitcoinAddrDb.h \
  bitcoinconfig.h   \
  CBitcoinCheckpoints.h \
  CBitcoinBlock.h   


libbitcoinnet_a_SOURCES = \
	CChainManager.cpp \
	CBitcoinMsgHeader.cpp \
	CBitcoinNode.cpp \
	CBitcoinServer.cpp \
	CBitcoinNetMessage.cpp \
	CNodeManager.cpp \
	CBitcoinAddrDb.cpp \
	CBitcoinCheckpoints.cpp \
	CBitcoinBlock.cpp    \
	$(BTCOIN_NET_H) \
	$(BITCOIN_CORE_H)
