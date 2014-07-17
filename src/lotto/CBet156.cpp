/*
 * CBet156.cpp
 *
 *  Created on: May 27, 2014
 *      Author: ranger.shi
 */

#include "CBet156.h"

#include <boost/foreach.hpp>
//#include <boost/mpl/aux_/preprocessed/gcc/and.hpp>
//#include <boost/mpl/aux_/preprocessed/gcc/or.hpp>
//#include <algorithm>
//#include <cassert>
//#include <iterator>
#include <map>
#include <vector>

namespace lotto {

int CBet_15_6::getTotalBer() const {
	Assert(CBet::type == TYPE_15_6);
	return MselectN(vch.size(), 6);
}

bool CBet_15_6::IsValid() const {
	Assert(type == TYPE_15_6);
	if (vch.size() > 15 || vch.size() < 6)
		return false;

	if( vch.end() != std::find_if (vch.begin(), vch.end(), boost::bind(std::greater<unsigned char>(),_1,READ_MAX156_NO)))
	return false;
	if( vch.end() != std::find_if (vch.begin(), vch.end(), boost::bind(std::less<unsigned char>(),_1,1)))
	return false;
	///// the select number must be sort
    std::vector<unsigned char> tep = vch;
    sort(tep.begin(),tep.end());
    if(vch!=tep)
    	return false;
	return CBet::IsValid();
}
void CBet_15_6::CreateRewardMap(){
	//rewardmap
	const std::vector<unsigned char>&luckly = vLuckyStar;
	int luckycount = 0;
	BOOST_FOREACH(const unsigned char& no,vch) {
		if (find(luckly.begin(), luckly.end(), no) != luckly.end()) {
			luckycount++;
		}
	}
	int64_t npeerBetValue = nValue/getTotalBer();
	int64_t ktop3 =0, ktop2 = 0, ktop1 = 0;
	int selectCount = vch.size();
	rewardmap.clear();
	ktop1 = MselectN(luckycount,6)*MselectN(selectCount-luckycount,0);
	ktop2 = MselectN(luckycount,5)*MselectN(selectCount-luckycount,1);
	ktop3 = MselectN(luckycount,4)*MselectN(selectCount-luckycount,2);

	if(ktop1 >0)
		rewardmap[TOP_1] = ktop1;
	if(ktop2 >0)
		rewardmap[TOP_2] = ktop2;
	if(ktop3 >0)
	{
		rewardmap[TOP_3] = ktop3;
		//cout<<"third"<<ktop3<<endl;
	}

	if(rewardmap.count(TOP_1) > 0)
	{
		rewardmap[TOP_1] *= npeerBetValue;
		LogTrace2("print","\n\n CBet_15_6::TOP_1:%ld",rewardmap[TOP_1]);
	}
	if(rewardmap.count(TOP_2) > 0)
	{
		rewardmap[TOP_2] *=npeerBetValue;
		LogTrace2("print","\n\n CBet_15_6::TOP_2:%ld",rewardmap[TOP_2]);
	}
	if(rewardmap.count(TOP_3) > 0)
	{
		rewardmap[TOP_3] *= npeerBetValue;
		LogTrace2("print","\n\n CBet_15_6::TOP_3:%ld",rewardmap[TOP_3]);
	}
	LogTrace2("print","\n\n CBet_15_6::npeerBetValue:%ld",npeerBetValue);
}
//void CBet_15_6::CreateRewardMap(){
//	//rewardmap
//	const std::vector<unsigned char>&luckly = vLuckyStar;
//	int count = 0;
//	BOOST_FOREACH(const unsigned char& no,vch) {
//		if (find(luckly.begin(), luckly.end(), no) != luckly.end()) {
//			count++;
//		}
//	}
//	int64_t npeerBetValue = nValue/getTotalBer();
//	if (count >= 2)
//	{
//		if(count == 6)
//		{
//			rewardmap[TOP_1] = 1*npeerBetValue;
//			if((vch.size()- 6) >= 1)
//			rewardmap[TOP_2] = MselectN(6,5) *MselectN(vch.size()- 6,1);
//			if((vch.size()- 6) >= 2)
//			rewardmap[TOP_3] = MselectN(6,4)*MselectN(vch.size()- 6,2);
//			if((vch.size()- 6) >= 3)
//			rewardmap[TOP_3] += MselectN(6,3)*MselectN(vch.size()- 6,3);
//			if((vch.size()- 6) >= 4)
//			rewardmap[TOP_3] += MselectN(6,2)*MselectN(vch.size()- 6,4);
//		}
//		if(count < 6 && count >4)  //// two
//		{
//			if((vch.size() - 1) >= 1)
//			rewardmap[TOP_2] = MselectN(vch.size()- 5,1);
//			if((vch.size()- 5) >= 2)
//			rewardmap[TOP_3] = MselectN(5,4)*MselectN(vch.size()- 5,2);
//			if((vch.size()- 5) >= 3)
//			rewardmap[TOP_3] += MselectN(5,3)*MselectN(vch.size()- 5,3);
//			if((vch.size()- 5) >= 4)
//			rewardmap[TOP_3] += MselectN(5,2)*MselectN(vch.size()- 5,4);
//		}
//	/// third
//	if(count < 5 && count >3)
//		{
//			if((vch.size()- 4) >= 2)
//			rewardmap[TOP_3] = MselectN(vch.size()- 4,2);
//			if((vch.size()- 4) >= 3)
//			rewardmap[TOP_3] += MselectN(4,3)*MselectN(vch.size()- 4,3);
//			if((vch.size()- 4) >= 4)
//			rewardmap[TOP_3] += MselectN(4,2)*MselectN(vch.size()- 4,4);
//		}
//		if(count < 4 && count >2)
//		{
//			rewardmap[TOP_3] = MselectN(vch.size()- 3,3);
//			if((vch.size() -3) >= 4)
//			{
//				rewardmap[TOP_3] += MselectN(3,2)*MselectN(vch.size()- 3,4) ;
//			}
//		}
//		if(count < 3 )
//		{
//			rewardmap[TOP_3] = MselectN(vch.size()- 2,4);
//		}
//	}
//
//	if(rewardmap.count(TOP_3) > 0)
//	rewardmap[TOP_3] *= npeerBetValue;
//	if(rewardmap.count(TOP_2) > 0)
//	rewardmap[TOP_2] *=npeerBetValue;
//}

int64_t CBet_15_6::GetMulit(int type) const
{
	if(type < 0 || type >2)
	{
		throw("invalid data error");
	}
	return multiplying[type];
}
bool CBet_15_6::print() const {
	std::string tem ="";
	tem += tfm::format("\n\n CBet_15_6::\nmultiplying[3] %f, %f, %f \n", multiplying[0],multiplying[1],multiplying[2]);
	tem += tfm::format("%s","bass class:\n");
	LogTrace2("print","%s",tem);
	return CBet::print();

}
}/* namespace lotto */


