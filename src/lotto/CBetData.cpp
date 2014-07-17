/*
 * CBetData.cpp
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#include "CBetData.h"
#include "util.h"

namespace lotto {
void CBetData::print() {
	LogPrintf("CBetData data: %s\n", VectorToString(*this));

}

} /* namespace lotto */

