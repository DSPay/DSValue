/*
 * BitcoinConfig.h
 *
 *  Created on: Jun 4, 2014
 *      Author: ranger.shi
 */

#ifndef BITCOIN_CONFIG_H_
#define BITCOIN_CONFIG_H_

namespace Bitcoin
{

	const int  BITCOIN_CLIENT_VERSION_MAJOR 	= 0;
	const int  BITCOIN_CLIENT_VERSION_MINOR 	= 9;
	const int  BITCOIN_CLIENT_VERSION_REVISION = 1;
	const int  BITCOIN_CLIENT_VERSION_BUILD 	= 0;

	// disconnect from peers older than this proto version
	const int BITCOIN_MIN_PEER_PROTO_VERSION = 70002;
	const int BITCOIN_CLIENT_VERSION =
	                           1000000 * BITCOIN_CLIENT_VERSION_MAJOR
	                         +   10000 * BITCOIN_CLIENT_VERSION_MINOR
	                         +     100 * BITCOIN_CLIENT_VERSION_REVISION
	                         +       1 * BITCOIN_CLIENT_VERSION_BUILD;

	extern unsigned char BitcoinMessageStart[4] ;
}

#endif /* BitcoinCONFIG_H_ */
