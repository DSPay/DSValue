/*
 * CDebug.cpp
 *
 *  Created on: Jun 9, 2014
 *      Author: ranger.shi
 */

#include "CDebug.h"
#include "util.h"
#include <boost/assign/list_of.hpp> // for 'map_list_of()'#include <boost/foreach.hpp>

#include <boost/foreach.hpp>using namespace std;
// fuck




CDebug::CDebug() {

	boost::filesystem::path pathDebug = GetDataDir() / "debugtrace.log";
	localfileout = fopen(pathDebug.string().c_str(), "a");
	if (localfileout)
		setbuf(localfileout, NULL); // unbuffered

	pmutexdebug = new boost::mutex();

	IsOutToTagFile = false;
	IsLogAddFile = true;
	IsLogAddFunc = true;
	IsLogAddLine = true;
	IsLogAddTime = true;
	TraceMode = TRACE_ALL;
	IsOutToFile = false;
}

CDebug& CDebug::instance() {
	static CDebug mCDebug;
	return mCDebug;
}

bool CDebug::print(void) const {

	std::string tem = "";
	tem += tfm::format("\n\CDebug::print \nIsLogAddFile:%d IsLogAddFunc:%d IsLogAddLine:%d IsLogAddTime:%d\n",
			IsLogAddFile, IsLogAddFunc, IsLogAddLine, IsLogAddTime);
	tem += tfm::format("\nTraceMode:%d IsLogAddFunc:%d IsOutToTagFile:%d \n", TraceMode, IsOutToFile, IsOutToTagFile);

	tem += "sAllowedTag: \n";
	BOOST_FOREACH(const std::string& tep1 , sAllowedTag)
	tem += tfm::format("%s\n", tep1);

	tem += "sBlockedTag: \n";
	BOOST_FOREACH(const std::string& tep1 , sBlockedTag)
	tem += tfm::format("%s\n", tep1);

	tem += "sAllowedline: \n";
	BOOST_FOREACH(const int& tep1 , sAllowedline)
	tem += tfm::format("%d\n", tep1);

	tem += "sBlockedline: \n";
	BOOST_FOREACH(const int& tep1 , sBlockedline)
	tem += tfm::format("%d\n", tep1);

	tem += "sAllowedFile: \n";
	BOOST_FOREACH(const std::string& tep1 , sAllowedFile)
	tem += tfm::format("%s\n", tep1);

	tem += "sBlockedFile: \n";
	BOOST_FOREACH(const std::string& tep1 , sBlockedFile)
	tem += tfm::format("%s\n", tep1);
	cout << tem << endl;
	return true;

}

bool CDebug::CfgFitter(bool IsAddFile, bool IsAddFunc, bool IsAddLine, bool IsAddTime) {
	IsLogAddFile = IsAddFile;
	IsLogAddFunc = IsAddFunc;
	IsLogAddLine = IsAddLine;
	IsLogAddTime = IsAddTime;
	return true;
}
struct EraseInt {
	std::set<int> &sAllowedline;
	EraseInt(std::set<int> &inial) :
			sAllowedline(inial) {
	}
	;
	void operator()(int tep) {
		sAllowedline.erase(tep);
	}
	;
};
bool CDebug::FitterWhiteLine(int falg, int lin1, int line2, int line3, int line4, int line5) {
	int line[5] = { lin1, line2, line3, line4, line5 };
	if (falg == 1) {
		sAllowedline.insert(line, line + 5);
		sAllowedline.erase(0);
	} else if (falg == -1) {
		std::for_each(line, line + 5, EraseInt(sAllowedline));
	} else
		assert(0);

	return true;

}

bool CDebug::FitterBlackLine(int falg, int lin1, int line2, int line3, int line4, int line5) {

	int line[5] = { lin1, line2, line3, line4, line5 };
	if (falg == 1) {
		sBlockedline.insert(line, line + 5);
		sBlockedline.erase(0);
	} else if (falg == -1) {
		std::for_each(line, line + 5, EraseInt(sBlockedline));
	} else
		assert(0);
	return true;

}
struct EraseStr {
	std::set<std::string> &sAllowedline;
	EraseStr(std::set<std::string> &inial) :
			sAllowedline(inial) {

	}
	;
	void operator()(std::string tep) {
		sAllowedline.erase(tep);
	}
	;
};
bool CDebug::FitterWhiteTag(int flag, const char* tag1, const char* tag2, const char* tag3, const char* tag4,
		const char* tag5) {
	std::string line[5] = { std::string(tag1 == NULL ? " " : tag1), std::string(tag2 == NULL ? " " : tag2), std::string(
			tag3 == NULL ? " " : tag3), std::string(tag4 == NULL ? " " : tag4), std::string(tag5 == NULL ? " " : tag5) };
	if (flag == 1) {
		sAllowedTag.insert(line, line + 5);
		sAllowedTag.erase(std::string(" "));
	} else if (flag == -1) {
		std::for_each(line, line + 5, EraseStr(sAllowedTag));
	} else
		assert(0);
	return true;
}

bool CDebug::FitterBlackTag(int flag, const char* tag1, const char* tag2, const char* tag3, const char* tag4,
		const char* tag5) {
	std::string line[5] = { std::string(tag1 == NULL ? " " : tag1), std::string(tag2 == NULL ? " " : tag2), std::string(
			tag3 == NULL ? " " : tag3), std::string(tag4 == NULL ? " " : tag4), std::string(tag5 == NULL ? " " : tag5) };
	if (flag == 1) {
		sBlockedTag.insert(line, line + 5);
		sBlockedTag.erase(std::string(" "));
	} else if (flag == -1) {
		std::for_each(line, line + 5, EraseStr(sBlockedTag));
	} else
		assert(0);
	return true;
}

bool CDebug::FitterWhiteFile(int flag, const char* tag1, const char* tag2, const char* tag3, const char* tag4,
		const char* tag5) {
	std::string line[5] = { std::string(tag1 == NULL ? " " : tag1), std::string(tag2 == NULL ? " " : tag2), std::string(
			tag3 == NULL ? " " : tag3), std::string(tag4 == NULL ? " " : tag4), std::string(tag5 == NULL ? " " : tag5) };
	if (flag == 1) {
		sAllowedFile.insert(line, line + 5);
		sAllowedFile.erase(std::string(" "));
	} else if (flag == -1) {
		std::for_each(line, line + 5, EraseStr(sAllowedFile));
	} else
		assert(0);
	return true;
}

bool CDebug::FitterBlackFile(int flag, const char* tag1, const char* tag2, const char* tag3, const char* tag4,
		const char* tag5) {
	std::string line[5] = { std::string(tag1 == NULL ? " " : tag1), std::string(tag2 == NULL ? " " : tag2), std::string(
			tag3 == NULL ? " " : tag3), std::string(tag4 == NULL ? " " : tag4), std::string(tag5 == NULL ? " " : tag5) };
	if (flag == 1) {
		sBlockedFile.insert(line, line + 5);
		sBlockedFile.erase(std::string(" "));
	} else if (flag == -1) {
		std::for_each(line, line + 5, EraseStr(sBlockedFile));
	} else
		assert(0);
	return true;
}
struct InsertStringSet {
	std::set<string> &sAllowedline;
	InsertStringSet(std::set<string> &inial) :
			sAllowedline(inial) {
	}
	;
	void operator()(string tep) {
		sAllowedline.insert(tep);
	}
	;
};
struct InsertIntSet {
	std::set<int> &sAllowedline;
	InsertIntSet(std::set<int> &inial) :
			sAllowedline(inial) {
	}
	;
	void operator()(string tep) {
		sAllowedline.insert(atoi(tep.c_str()));
	}
	;
};
static bool GetBoolArg(map<string, vector<string> >& mapMultiArg, const std::string& strArg, bool fDefault) {
	if (mapMultiArg.count(strArg)) {
		if (mapMultiArg[strArg].size() == 0)
			return fDefault;
		return (mapMultiArg[strArg][0] == "true");
	}
	return fDefault;
}

bool CDebug::Inital(map<string, vector<string> >& mapMultiArg) {
	vector<string> tep;

	IsLogAddFile = GetBoolArg(mapMultiArg, "-debug_tarce_file", true);
	IsLogAddFunc = GetBoolArg(mapMultiArg, "-debug_tarce_func", true);
	IsLogAddLine = GetBoolArg(mapMultiArg, "-debug_tarce_line", true);
	IsLogAddTime = GetBoolArg(mapMultiArg, "-debug_tarce_time", true);
	IsOutToFile = GetBoolArg(mapMultiArg, "-debug_tarce_to_file", false);
	IsOutToTagFile = GetBoolArg(mapMultiArg, "-debug_tarce_out_to_tag_file", false);
	if (mapMultiArg.count("-debug_trace_type")) {
		if (mapMultiArg["-debug_trace_type"].size() > 0) {
			string te = (mapMultiArg["-debug_trace_type"].front());
			if (te == "TRACE_ALL")
				TraceMode = TRACE_ALL;
			else if (te == "TRACE_NO")
				TraceMode = TRACE_NO;
			else if (te == "TRACE_BLOCKED_BLACK_LIST")
				TraceMode = TRACE_BLOCKED_BLACK_LIST;
			else if (te == "TRACE_WIHTE_LIST")
				TraceMode = TRACE_WIHTE_LIST;
			else
				assert(0);
		}

	}
	if (mapMultiArg.count("-debug_trace_tag_white_list")) {
		tep = mapMultiArg["-debug_trace_tag_white_list"];
		std::for_each(tep.begin(), tep.end(), InsertStringSet(sAllowedTag));
	}

	if (mapMultiArg.count("-debug_trace_tag_black_list")) {
		tep = mapMultiArg["-debug_trace_tag_black_list"];
		std::for_each(tep.begin(), tep.end(), InsertStringSet(sBlockedTag));
	}

	if (mapMultiArg.count("-debug_trace_file_black_list")) {
		tep = mapMultiArg["-debug_trace_file_black_list"];
		std::for_each(tep.begin(), tep.end(), InsertStringSet(sBlockedFile));
	}

	if (mapMultiArg.count("-debug_trace_file_white_list")) {
		tep = mapMultiArg["-debug_trace_file_white_list"];
		std::for_each(tep.begin(), tep.end(), InsertStringSet(sAllowedFile));
	}

	if (mapMultiArg.count("-debug_trace_line_black_list")) {
		tep = mapMultiArg["-debug_trace_line_black_list"];
		std::for_each(tep.begin(), tep.end(), InsertIntSet(sBlockedline));
	}

	if (mapMultiArg.count("-debug_trace_line_white_list")) {
		tep = mapMultiArg["-debug_trace_line_white_list"];
		std::for_each(tep.begin(), tep.end(), InsertIntSet(sAllowedline));
	}

	return true;
}

void CDebug::PrepareTagFile(const char* tag) {
	if (IsOutToTagFile == true) {
		if (mfileHand.count(string(tag)) == 0) {
			boost::filesystem::path pathDebug = GetDataDir() / (string("tag_") + string(tag) + "_trace.log");
			FILE* fileout = fopen(pathDebug.string().c_str(), "a");
			if (fileout)
			{
				setbuf(fileout, NULL); // unbuffered
			mfileHand.insert(std::make_pair(string(tag), fileout));
			localfileout = fileout;
			}
		} else {
			localfileout = mfileHand[string(tag)];
		}
	} else {
		Assert(0);
	}
}

CDebug::~CDebug() {
	delete pmutexdebug;
BOOST_FOREACH(const PAIRTYPE(std::string,FILE*)&item, mfileHand) {
	fclose(item.second);

}

}

// We use boost::call_once() to make sure these are initialized in
// in a thread-safe manner the first time it is called:

int CDebug::PrintTrace(const std::string &str) {
int ret = 0; // Returns total number of characters written
if (IsOutToFile == false) {
	// print to console
	ret = fwrite(str.data(), 1, str.size(), stdout);
} else {

	if (localfileout == NULL)
		return ret;
	boost::mutex::scoped_lock scoped_lock(*pmutexdebug);
	ret = fwrite(str.data(), 1, str.size(), localfileout);
}

return ret;
}

bool CDebug::LogIsAcceptTag(const char* tag, const char* file, const char* func, int line) {

if (TraceMode == TRACE_ALL) // out all
	return true;
if (TraceMode == TRACE_NO) //block all
	return false;
if (TraceMode == TRACE_BLOCKED_BLACK_LIST) {  //2 only block black list
	if (sBlockedTag.find(std::string(tag)) != sBlockedTag.end() || sBlockedline.find(line) != sBlockedline.end()
			|| sBlockedFile.find(std::string(file)) != sBlockedFile.end()) {
		//line  black list can not block print log
		if (sBlockedline.find(line) != sBlockedline.end()) {
			return false;
		}
	}
	return true;
} else if (TraceMode == TRACE_WIHTE_LIST) { //3 only wihte list allowed
	if (sAllowedTag.find(std::string(tag)) != sAllowedTag.end() || sAllowedline.find(line) != sAllowedline.end()
			|| sAllowedFile.find(std::string(file)) != sAllowedFile.end())
		return true;
	return false;
}
Assert(0);
return false;
}

bool CDebug::LogFitter(const char* tag, const char* file, const char* func, int line) {

if (LogIsAcceptTag(tag, file, func, line)) {
	std::string tep;
	tep += tfm::format("Tag:%s ", tag);
	tep += (IsLogAddFile) ? (tfm::format("File:%s ", file)) : ("");
	tep += (IsLogAddFunc) ? (tfm::format("Func:%s ", func)) : ("");
	tep += (IsLogAddTime) ?
			(tfm::format("Time:%s", DateTimeStrFormat("%H:%M:%S", GetTime()).c_str())
					+ (tfm::format(":%d ", GetTimeMillis() % 1000))) :
			("");
	tep += (IsLogAddLine) ? (tfm::format("Line:%d ", line)) : ("");
	tep += "\n";
	if (IsOutToTagFile == true) {
		PrepareTagFile(tag);
	}
	PrintTrace(tep);
	return true;
} else {
	return false;
}

}
bool CDebug::LogFitter(bool IsOutTag, const char* tag, const char* file, const char* func, int line) {

if (IsOutTag == false) {
	if (IsOutToTagFile == true)
		PrepareTagFile(tag);
	return true;
}

if (LogIsAcceptTag(tag, file, func, line)) {
	std::string tep;
	tep += (IsLogAddTime) ?
			(tfm::format("Time:%s", DateTimeStrFormat("%H:%M:%S", GetTime()).c_str())
					+ (tfm::format(":%d ", GetTimeMillis() % 1000))) :
			("");
	tep += tfm::format("Tag:%s ", tag);
	tep += (IsLogAddFile) ? (tfm::format("File:%s ", file)) : ("");
	tep += (IsLogAddFunc) ? (tfm::format("Func:%s ", func)) : ("");

	tep += (IsLogAddLine) ? (tfm::format("Line:%d ", line)) : ("");
	tep += "\n";
	if (IsOutToTagFile == true) {
		PrepareTagFile(tag);
	}
	PrintTrace(tep);
	return true;
} else {
	return false;
}
}

