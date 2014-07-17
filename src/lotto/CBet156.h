/*
 * CBet156.h
 *
 *  Created on: May 27, 2014
 *      Author: ranger.shi
 */

#ifndef CBET156_H_
#define CBET156_H_
#include "CBet.h"
namespace lotto {
static const unsigned char READ_MAX156_NO = 15;
class CBet_15_6 : public CBet {

public:
	CBet_15_6() {
		multiplying[0]= 3000;
		multiplying[1]= 17;
		multiplying[2]= 2;
	}
	~CBet_15_6() {

	}
	virtual int getTotalBer() const;
	virtual bool IsValid() const;
	virtual void CreateRewardMap() ;
	virtual int64_t GetMulit(int type) const;
	 virtual bool print()const;
private:
	int64_t multiplying[3];  /*{5005,3003,1365,455,105}15 select 6*/
};

} /* namespace lotto */

#endif /* CBET156_H_ */
