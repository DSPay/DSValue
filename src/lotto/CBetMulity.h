/*
 * CBetMulity.h
 *
 *  Created on: Jun 7, 2014
 *      Author: ranger.shi
 */

#ifndef CBETMULITY_H_
#define CBETMULITY_H_

#include "CBet.h"

namespace lotto {
extern const unsigned char gSplitFlag;
class CBetMulity: public CBet {

private:
	int64_t multiplying[5];  /*{5005,3003,1365,455,105}15 select 6*/
public:
	std::vector<unsigned char> vRed;
	std::vector<unsigned char> vBlue;
public:
	virtual bool IsValid() const;
    virtual void CreateRewardMap()  ;
    virtual int64_t GetMulit(int type) const;
	int virtual getTotalBer() const ;
	virtual bool print()const;
	void SplitSelectNum();
	void CalcRewardBet(int ,int );


	CBetMulity();
	virtual ~CBetMulity();
};

} /* namespace lotto */

#endif /* CBETMULITY_H_ */
