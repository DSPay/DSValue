#ifndef LOTTERY_FILE_H
#define LOTTERY_FILE_H

#include "bignum.h"
#include "chainparams.h"
#include "coins.h"
#include "core.h"
#include "net.h"
#include "script.h"
#include "sync.h"
#include "txmempool.h"
#include "uint256.h"
#include "lotto/CLottoHeader.h"

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;

template<typename T>
static bool ReadObjFromFile(const char* filename, int index, T& obj, int nType = SER_GETHASH, int nVersion =
		PROTOCOL_VERSION) {

	int dataSize = obj.GetSerializeSize(nType, nVersion);
	int pos = dataSize * index;
	boost::filesystem::path path = GetDataDir() / filename;
	FILE* file = fopen(path.string().c_str(), "rb");
	if (!file) {
		LogPrintf("Unable to open file %s\n", path.string());
		return false;
	}
	if (pos > 0) {
		if (fseek(file, pos, SEEK_SET)) {
			LogPrintf("Unable to seek to position %u of %s\n", pos, path.string());
			fclose(file);
			return false;
		}
	}

	CAutoFile filein = CAutoFile(file, nType, nVersion);
	if (!filein)
		return error("CLotDB::Read() : open failed");

	try {
		filein >> obj;
	} catch (std::exception &e) {
		return error("CLotDB::Read(): I/O error or stream data corrupted");
	}

	filein.fclose();
	return true;
}
static inline FILE* OpenFile(const char* filename, bool fReadOnly) {
	boost::filesystem::path path = GetDataDir() / filename;
	FILE* file = fopen(path.string().c_str(), "ab");
	if (!file && !fReadOnly)
		file = fopen(path.string().c_str(), "ab");
	if (!file) {
		LogPrintf("Unable to open file %s\n", path.string());
		return NULL;
	}
	return file;
}
template<typename T>
int AppendToFile(const char* filename, const T& obj, int nType = SER_GETHASH, int nVersion = PROTOCOL_VERSION) {

	CAutoFile fileout = CAutoFile(OpenFile(filename, false), SER_DISK, CLIENT_VERSION);
	if (!fileout)
		return error("CLotDB::Write() : open failed");
	long fileOutPos = ftell(fileout);
	if (fileOutPos < 0)
		return error("WriteBlockToDisk : ftell failed");
	// Write and commit header, data
	try {
		fileout << obj;
	} catch (std::exception &e) {
		return error("CLotDB::Write() : I/O error");
	}
	fflush(fileout);
	FileCommit(fileout);
	fileout.fclose();
	return true;
}

class CLottoFileKey {

public:
	int Index;
private:
	vector<uint256> Key;
public:

	IMPLEMENT_SERIALIZE
	(
			READWRITE(Index);
			READWRITE(Key);
	)
	CLottoFileKey(int index) {
		Index = index;

		Key.push_back(uint256(0));
	}
	CLottoFileKey(int index, uint256 hash) {
		Index = index;
		Key.clear();
		Key.push_back(hash);
	}
	CLottoFileKey() {
		Index = 0;
		Key.push_back(uint256(0));
	}
	const vector<uint256>& getKey() {
		return Key;

	}

	const int& getIndex() {
		return Index;
	}

	void setKey(const vector<uint256>& nKey) {
		Key = nKey;
	}

	void setIndex(const int& nIndex) {
		Index = nIndex;
	}
	std::string ToString() const {
		int i = 0;
		std::string tem = "";
		tem += tfm::format("\n\n class : %s\n", "CLottoFileKey");
		tem += tfm::format("Index: %d\n", Index);
		BOOST_FOREACH(const uint256 & item, Key) {
			tem += tfm::format("vector<uint256> Key%d :%s\n", i++, item.ToString());
		}
		return tem;
	}
	bool print() const {
		LogTrace2("print", "%s", ToString());
		return true;
	}

};

class CLottoDecryptData {
private:

	vector<unsigned char> Start;
public:
	static const int DATA_SIZE = 48;
	int nVersion;
	int Index;
	vector<unsigned char> vClearData;
	CLottoDecryptData() {
		nVersion = 0;
		Index = 0;
		unsigned char * tep = (unsigned char *) Params().MessageStart();
		Start.assign(tep, tep + 4);
		std::fill_n(inserter(vClearData, vClearData.begin()), DATA_SIZE, 0);
	}
	;
public:
	IMPLEMENT_SERIALIZE
	(
			READWRITE(nVersion);
			READWRITE(Index);
			READWRITE(Start);
			READWRITE(vClearData);
	)

	bool print() const {
		std::string tem = "";
		tem += tfm::format("\n\n class: %s\n", "CLottoDecryptData");
		tem += tfm::format("Version  %d\n", nVersion);
		tem += tfm::format("Index: %d\n", Index);
		tem += tfm::format("vector<unsigned char> vClearData :%s\n", VectorToString(vClearData));
		LogTrace2("print", "%s", tem);
		return true;
	}
	CLottoDecryptData(long ind) {
		nVersion = 100;
		Index = ind;
		unsigned char * tep = (unsigned char *) Params().MessageStart();
		Start.assign(tep, tep + 4);
		std::fill_n(inserter(vClearData, vClearData.begin()), DATA_SIZE, 0);

	}

	int getindex() const {
		return Index;
	}

	void setIndex(int index) {
		Index = index;
	}
};

class CLottoFileData {

	static const int DATA_SIZE = 48; /*80+32*3 +32*4*/
public:
	CLottoDecryptData data;
	uint256 sum;
	vector<unsigned char> SecretData; //encrypted data
	CLottoFileData(long ind) :
			data(ind) {

		std::fill_n(inserter(SecretData, SecretData.begin()), DATA_SIZE, 0);
	}
	CLottoFileData() {
		std::fill_n(inserter(SecretData, SecretData.begin()), DATA_SIZE, 0);
	}
	uint256& CalcHash() {
		return sum = SerializeHash(data);
	}
	IMPLEMENT_SERIALIZE
	(
			READWRITE(SecretData);
			READWRITE(sum);
	)

	bool print() const {
		std::string tem = "";
		tem += tfm::format("\n\n class: %s\n", "CLottoFileData");
		tem += tfm::format("uint256 sum:%s\n", sum.ToString());
		tem += tfm::format("vector<unsigned char> SecretData :%s\n", VectorToString(SecretData));
		tem += tfm::format("CLottoDecryptData data %s\n", ":");
		data.print();
		LogTrace2("print", "%s", tem);
		return true;
	}

};

class CLotDB {
private:
	boost::filesystem::path path;

public:
	CLotDB();
	CLotDB(string fname);
	bool Write(long index, CLottoFileData &sFileData);
	bool Read(long index, CLottoFileData &sFileData);
	FILE* OpenFile(const long &pos, bool fReadOnly);
};

class CLotData: public CLotDB {

public:

	bool DecryptData(const CLottoFileKey &key, const vector<unsigned char>& vInput, vector<unsigned char>& vDecrypted);
	bool EncryptData(const CLottoFileKey &key, const vector<unsigned char> &vInput, vector<unsigned char>& vEncrypted);
	CLotData(string fname) :
			CLotDB(fname) {
	}
	;
	bool ReadCLotData(std::vector<unsigned char>& vchSecret, const CLottoFileKey &key);
	bool WriteCLotData(std::vector<unsigned char>& vcClearText, const CLottoFileKey &key);
	bool ReadKey(const char* filename, int index, CLottoFileKey& key);
	bool CreateClottoFile(const char* filename, int size);

	uint256 GetClottoFileHash();
};

bool sEncryptData(const CKeyingMaterial& vMasterKey, const CKeyingMaterial &vchPlaintext, const uint256& nIV,
		std::vector<unsigned char> &vchCiphertext);
bool sDecryptData(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char>& vchCiphertext,
		const uint256& nIV, CKeyingMaterial& vchPlaintext);
vector<unsigned char> GetRandomVec(int size);

#endif
