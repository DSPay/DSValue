/*
 * CDebug.h
 *
 *  Created on: Jun 9, 2014
 *      Author: ranger.shi
 */

#ifndef CDEBUG_H_
#define CDEBUG_H_
#include "util.h"
enum TRACE_TYPE
{
	TRACE_ALL,// 0 out all
	TRACE_NO,// 1 block all
	TRACE_BLOCKED_BLACK_LIST,// 2 only block black list
	TRACE_WIHTE_LIST,// 3 only out wihte list
};
using namespace std;
class CDebug {
private :
	bool IsLogAddFile;
	bool IsLogAddFunc;
	bool IsLogAddLine;
	bool IsLogAddTime;
	bool IsOutToFile;
	bool IsOutToTagFile;

	std::map<std::string,FILE*> mfileHand;


	TRACE_TYPE TraceMode;
	std::set<std::string> sAllowedTag;
	std::set<std::string> sBlockedTag;

	std::set<int> sAllowedline;
	std::set<int> sBlockedline;

	std::set<std::string> sAllowedFile;
	std::set<std::string> sBlockedFile;

	FILE* localfileout;
	boost::mutex* pmutexdebug;


	void PrepareTagFile(const char* tag);


public:


    bool Inital(map<string, vector<string> >& mapMultiArg);
	bool SetFitterMode(TRACE_TYPE mode){ TraceMode = mode; return true;};
	bool LogIsAcceptTag(const char* tag, const char* file, const char* func, int line);
	int PrintTrace(const std::string &str);
	bool LogFitter(const char* tag, const char* file, const char* func, int line);
	bool LogFitter(bool falg, const char* tag, const char* file, const char* func, int line);
	bool CfgFitter(bool IsLogAddFile = false, bool IsLogAddFunc = false, bool IsLogAddLine = false, bool IsLogAddTime =
			false);
	bool FitterWhiteLine(int flag,int lin1, int line2 = 0, int line3 = 0, int line4 = 0, int line5 = 0);
	bool FitterBlackLine(int flag,int lin1, int line2 = 0, int line3 = 0, int line4 = 0, int line5 = 0);
	bool FitterWhiteTag(int falg ,const char* tag1, const char* tag2 = NULL, const char* tag3 = NULL, const char* tag4 = NULL,
			const char* tag5 = NULL);
	bool FitterBlackTag(int falg ,const char* tag1, const char* tag2 = NULL, const char* tag3 = NULL, const char* tag4 = NULL,
			const char* tag5 = NULL);
	bool FitterWhiteFile(int falg ,const char* file1, const char* file2 = NULL, const char* file3 = NULL, const char* file4 = NULL,
			const char* file5 = NULL);
	bool FitterBlackFile( int flag,const char* file1, const char* file2 = NULL, const char* file3 = NULL, const char* file4 = NULL,
			const char* file5 = NULL);
	static CDebug& instance();
	bool print(void)  const;
	CDebug();
	virtual ~CDebug();


};

#define MAKE_TRACE_FUNC(n)                                        \
    /*   Print to debug.log if -debug=category switch is given OR category is NULL. */ \
    template<TINYFORMAT_ARGTYPES(n)>                                          \
    static inline int Logtrace(const char* tag, const char* file, const char* func, int line, const char* format, TINYFORMAT_VARARGS(n))  \
    {                                                                         \
		if(!CDebug::instance().LogFitter(tag, file, func, line)) return 0;  \
        return CDebug::instance().PrintTrace(tfm::format(format, TINYFORMAT_PASSARGS(n))); \
    }           \
	template<TINYFORMAT_ARGTYPES(n)>                                          \
	static inline int Logtrace(bool flag ,const char* tag, const char* file, const char* func, int line, const char* format, TINYFORMAT_VARARGS(n))  \
	{                                                                         \
		if(!CDebug::instance().LogFitter(flag,tag, file, func, line)) return 0;  \
		return CDebug::instance().PrintTrace(tfm::format(format, TINYFORMAT_PASSARGS(n))); \
	}

TINYFORMAT_FOREACH_ARGNUM(MAKE_TRACE_FUNC)
/* Zero-arg versions of logging and error, these are not covered by
 * TINYFORMAT_FOREACH_ARGNUM
 */
static inline int Logtrace(const char* tag, const char* file, const char *func, int line, const char* format) {
	if (!CDebug::instance().LogFitter(tag, file, func, line))
		return 0;
	return CDebug::instance().PrintTrace(format);
}
static inline int Logtrace(bool flag ,const char* tag, const char* file, const char *func, int line, const char* format) {
	if (!CDebug::instance().LogFitter(flag,tag, file, func, line))
		return 0;
	return CDebug::instance().PrintTrace(format);
}
#define LogCfgSetTraceMode(mode) CDebug::instance().SetFitterMode(mode)
#define LogCfgTagPintf(...) CDebug::instance().CfgFitter(__VA_ARGS__)
#define LogCfgTagBlackTagList(flag,...) CDebug::instance().FitterBlackTag(flag,__VA_ARGS__)
#define LogCfgTagWhiteTagList(flag,...) CDebug::instance().FitterWhiteTag(flag,__VA_ARGS__)
#define LogCfgTagWhiteLineList(flag,...) CDebug::instance().FitterWhiteLine(flag,__VA_ARGS__)
#define LogCfgTagBlackLineList(flag,...) CDebug::instance().FitterBlackLine(flag,__VA_ARGS__)
#define LogCfgTagWhiteFileList(flag,...) CDebug::instance().FitterWhiteFile(flag,__VA_ARGS__)
#define LogCfgTagBlackFileList(flag,...) CDebug::instance().FitterBlackFile(flag,__VA_ARGS__)


//#define LogTrace(tag, ...) Logtrace(tag, __FILE__, __FUNCTION__, __LINE__ , __VA_ARGS__)
//#define LogTrace2(tag, ...) Logtrace(false, tag, __FILE__, __FUNCTION__, __LINE__ , __VA_ARGS__)

#define LogTrace(tag, ...)
//Logtrace(tag, __FILE__, __FUNCTION__, __LINE__ , __VA_ARGS__)
#define LogTrace2(tag, ...)
//Logtrace(false, tag, __FILE__, __FUNCTION__, __LINE__ , __VA_ARGS__)



#endif /* CDEBUG_H_ */
