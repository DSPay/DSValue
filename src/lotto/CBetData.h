/*
 * CBetData.h
 *
 *  Created on: May 28, 2014
 *      Author: ranger.shi
 */

#ifndef CBETDATA_H_
#define CBETDATA_H_
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "core.h"

#define ADRESSLEN 20
namespace lotto {


using namespace std;

enum BET_TYPE {
	TYPE_15_6,  //15 select 6
	TYPE_MULILT,   //20 select 5  and 10 select 2
	typenull,
};
enum RWD_TYPE {  //Reward
	TOP_1,  //15 select 6
	TOP_2,
	TOP_3,
	TOP_4,
	TOP_5,
	TOP_null,
};


class CBetData : public std::vector<unsigned char>  {
public:
	CBetData() {

	}
	~CBetData() {
	}

// to add data strut script code c1 data type address len var
public:
	BET_TYPE GetType() {
		const_iterator pc = begin();
	      if (pc >= end())
	            return (BET_TYPE)0;
		unsigned int type = *pc;
		Assert(!empty() && type < (int)typenull);
		return (BET_TYPE)type;
	}
	uint160 GetAddr() const {
		if(size() < ADRESSLEN)
			return uint160();

		std::vector<unsigned char> vaddr(begin()+1,begin()+1 + ADRESSLEN);
		Assert(!empty() && *(begin()) < (int)typenull);
		return uint160(vaddr);
	}
	std::vector<unsigned char> GetSelectV() const {
		std::vector<unsigned char> vaddr;
		if(size() < ADRESSLEN)
			return vaddr;
		const_iterator pc = begin();
		unsigned char type = *pc;
		Assert(!empty() && *pc < (int)typenull);
		vaddr.assign(begin()+1 + ADRESSLEN,end());
		return vaddr;
//		return std::vector<unsigned char>(pc+1 + 20 + 1 + 1,
//				pc+1 + 20 + 1 + 1 + len); // ToDo
	}

	explicit CBetData(const std::vector<unsigned char>& b) {
		operator<<(b);
	}
	CBetData& operator<<(const std::vector<unsigned char>& b) {
		//std::vector<unsigned char>(b.begin(), b.end());
		insert(end(), b.begin(), b.end());
		return *this;
	}
	void print();
};

} /* namespace lotto */

#endif /* CBETDATA_H_ */
