#include "lotteryfile.h"
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <iostream>     // std::cout#include <algorithm>    // std::equal#include <vector>       // std::vector

using namespace boost;const unsigned int LOTTOERY_CRYPTO_KEY_SIZE = 32;const unsigned int LOTTOERY_CRYPTO_SALT_SIZE = 8;
class CLotteryCrypter {
private:
	unsigned char chKey[LOTTOERY_CRYPTO_KEY_SIZE];
	unsigned char chIV[LOTTOERY_CRYPTO_KEY_SIZE];
	bool fKeySet;

public:
//    bool SetKeyFromPassphrase(const SecureString &strKeyData, const std::vector<unsigned char>& chSalt, const unsigned int nRounds, const unsigned int nDerivationMethod);
	bool Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext);
	bool Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext);
	bool SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV);

	void CleanKey() {
		OPENSSL_cleanse(chKey, sizeof(chKey));
		OPENSSL_cleanse(chIV, sizeof(chIV));
		fKeySet = false;
	}

	CLotteryCrypter() {
		fKeySet = false;

		// Try to keep the key data out of swap (and be a bit over-careful to keep the IV that we don't even use out of swap)
		// Note that this does nothing about suspend-to-disk (which will put all our key data on disk)
		// Note as well that at no point in this program is any attempt made to prevent stealing of keys by reading the memory of the running process.
		LockedPageManager::Instance().LockRange(&chKey[0], sizeof chKey);
		LockedPageManager::Instance().LockRange(&chIV[0], sizeof chIV);
	}

	~CLotteryCrypter() {
		CleanKey();

		LockedPageManager::Instance().UnlockRange(&chKey[0], sizeof chKey);
		LockedPageManager::Instance().UnlockRange(&chIV[0], sizeof chIV);
	}
};

bool CLotteryCrypter::SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV) {
	if (chNewKey.size() != LOTTOERY_CRYPTO_KEY_SIZE || chNewIV.size() != LOTTOERY_CRYPTO_KEY_SIZE)
		return false;

	memcpy(&chKey[0], &chNewKey[0], sizeof chKey);
	memcpy(&chIV[0], &chNewIV[0], sizeof chIV);

	fKeySet = true;
	return true;
}

bool CLotteryCrypter::Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext) {
	if (!fKeySet)
		return false;

	// max ciphertext len for a n bytes of plaintext is
	// n + AES_BLOCK_SIZE - 1 bytes
	int nLen = vchPlaintext.size();
	int nCLen = nLen + AES_BLOCK_SIZE, nFLen = 0;
	vchCiphertext = std::vector<unsigned char>(nCLen);

	EVP_CIPHER_CTX ctx;

	bool fOk = true;

	EVP_CIPHER_CTX_init(&ctx);
	if (fOk)
		fOk = EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, chKey, chIV);
	if (fOk)
		fOk = EVP_EncryptUpdate(&ctx, &vchCiphertext[0], &nCLen, &vchPlaintext[0], nLen);
	if (fOk)
		fOk = EVP_EncryptFinal_ex(&ctx, (&vchCiphertext[0]) + nCLen, &nFLen);
	EVP_CIPHER_CTX_cleanup(&ctx);

	if (!fOk)
		return false;

	vchCiphertext.resize(nCLen + nFLen);
	return true;
}

bool CLotteryCrypter::Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext) {
	if (!fKeySet)
		return false;

	// plaintext will always be equal to or lesser than length of ciphertext
	int nLen = vchCiphertext.size();
	int nPLen = nLen, nFLen = 0;

	vchPlaintext = CKeyingMaterial(nPLen);

	EVP_CIPHER_CTX ctx;

	bool fOk = true;

	EVP_CIPHER_CTX_init(&ctx);
	if (fOk)
		fOk = EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, chKey, chIV);
	if (fOk)
		fOk = EVP_DecryptUpdate(&ctx, &vchPlaintext[0], &nPLen, &vchCiphertext[0], nLen);
	if (fOk)
		fOk = EVP_DecryptFinal_ex(&ctx, (&vchPlaintext[0]) + nPLen, &nFLen);
	EVP_CIPHER_CTX_cleanup(&ctx);

	if (!fOk)
		return false;

	vchPlaintext.resize(nPLen + nFLen);
	return true;
}

bool sEncryptData(const CKeyingMaterial& vMasterKey, const CKeyingMaterial &vchPlaintext, const uint256& nIV,
		std::vector<unsigned char> &vchCiphertext) {
	CLotteryCrypter cKeyCrypter;
	std::vector<unsigned char> chIV(LOTTOERY_CRYPTO_KEY_SIZE);
	memcpy(&chIV[0], &nIV, LOTTOERY_CRYPTO_KEY_SIZE);
	if (!cKeyCrypter.SetKey(vMasterKey, chIV))
		return false;
	return cKeyCrypter.Encrypt(*((const CKeyingMaterial*) &vchPlaintext), vchCiphertext);
}

bool sDecryptData(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char>& vchCiphertext,
		const uint256& nIV, CKeyingMaterial& vchPlaintext) {
	CLotteryCrypter cKeyCrypter;
	std::vector<unsigned char> chIV(LOTTOERY_CRYPTO_KEY_SIZE);
	memcpy(&chIV[0], &nIV, LOTTOERY_CRYPTO_KEY_SIZE);
	if (!cKeyCrypter.SetKey(vMasterKey, chIV))
		return false;
	return cKeyCrypter.Decrypt(vchCiphertext, *((CKeyingMaterial*) &vchPlaintext));
}

CLotDB::CLotDB() {
	path = GetDataDir() / "lotteryfile.dat";
}

CLotDB::CLotDB(string fname) {
	path = GetDataDir() / fname;
}

FILE* CLotDB::OpenFile(const long &pos, bool fReadOnly) {
	FILE* file = fopen(path.string().c_str(), "rb+");
	if (!file && !fReadOnly)
		file = fopen(path.string().c_str(), "wb+");
	if (!file) {
		LogPrintf("Unable to open file %s\n", path.string());
		return NULL;
	}
	if (pos > 0) {
		if (fseek(file, pos, SEEK_SET)) {
			LogPrintf("Unable to seek to position %u of %s\n", pos, path.string());
			fclose(file);
			return NULL;
		}
	}
	return file;
}

bool CLotDB::Write(long index, CLottoFileData &sFileData) {
// serialize addresses, checksum data up to that point, then append csum
	CDataStream ssLotData(SER_DISK, CLIENT_VERSION);

	uint256 hash = SerializeHash(sFileData);

	int dataSize = sFileData.GetSerializeSize(SER_DISK, CLIENT_VERSION)
			+ hash.GetSerializeSize(SER_DISK, CLIENT_VERSION);

	CAutoFile fileout = CAutoFile(OpenFile(index * dataSize, false), SER_DISK, CLIENT_VERSION);
	if (!fileout)
		return error("CLotDB::Write() : open failed");
	long fileOutPos = ftell(fileout);
	if (fileOutPos < 0)
		return error("WriteBlockToDisk : ftell failed");
// Write and commit header, data
	try {
		fileout << sFileData << hash;
	} catch (std::exception &e) {
		return error("CLotDB::Write() : I/O error");
	}
	fflush(fileout);
	FileCommit(fileout);
	fileout.fclose();
	ssLotData << sFileData << hash;
	LogTrace("print", "%s\n", VectorToString(vector<unsigned char>(ssLotData.begin(), ssLotData.end())));
	return true;
}
bool CLotDB::Read(long index, CLottoFileData &sFileData) {

	uint256 hash;
	int dataSize = sFileData.GetSerializeSize(SER_DISK, CLIENT_VERSION)
			+ hash.GetSerializeSize(SER_DISK, CLIENT_VERSION);
//	LogTrace("spark", "the read dataSize:%d\n", dataSize);
	CAutoFile filein = CAutoFile(OpenFile(index * dataSize, true), SER_DISK, CLIENT_VERSION);
	if (!filein)
		return error("CLotDB::Read() : open failed");

	try {
		filein >> sFileData >> hash;
	} catch (std::exception &e) {
		return error("CLotDB::Read(): I/O error or stream data corrupted");
	}
//	LogTrace("spark", "the read data:%s\n", hash.ToString());
//	LogTrace("spark", "the read SecretData:%s\n", VectorToString(sFileData.SecretData));
	filein.fclose();
	CDataStream ssLotData(SER_DISK, CLIENT_VERSION);
	ssLotData << sFileData << hash;
	LogTrace("print", "%s\n", VectorToString(vector<unsigned char>(ssLotData.begin(), ssLotData.end())));

	return true;

}

bool CLotData::EncryptData(const CLottoFileKey &key, const vector<unsigned char> &vInput,
		vector<unsigned char>& vEncrypted) {
//	LogTrace("spark", "the vInput:%s\n", VectorToString(vInput));
	CLottoFileKey nkey = key;
	vector<uint256> keydata = nkey.getKey();
	if (keydata.size() != 1) {
		return false;
	}
	int ii = 0;
	CKeyingMaterial tmpdata(vInput.begin(), vInput.end());
	while (ii < 1) {
		CKeyingMaterial tmpkey(keydata[ii].begin(), keydata[ii].end());
//		LogTrace("spark", "the tmp:%s\n", VectorToString(vector<unsigned char>(tmpkey.begin(), tmpkey.end())));
//		LogTrace("spark", "the tmpdata:%s\n", VectorToString(vector<unsigned char>(tmpdata.begin(), tmpdata.end())));
		if (!sEncryptData(tmpkey, tmpdata, uint256(100), vEncrypted)) {
			return false;
		}
		tmpdata.assign(vEncrypted.begin(), vEncrypted.end());
		ii++;
	}

//	LogTrace("spark", "the vEncrypted:%s\n", VectorToString(vEncrypted));
	return true;
}

bool CLotData::DecryptData(const CLottoFileKey &key, const vector<unsigned char>& vInput,
		vector<unsigned char>& vDecrypted) {
	CLottoFileKey nkey = key;
	vector<uint256> keydata = nkey.getKey();

	if (keydata.size() == 1) {
		int ii = 0;
		vector<unsigned char> tmpdata = vInput;
//		LogTrace("spark", "read out from file:%s\n", VectorToString(tmpdata));
		while (ii >= 0) {
			CKeyingMaterial tmpkey(keydata[ii].begin(), keydata[ii].end());
			CKeyingMaterial tmpout;
//			LogTrace("spark", "the tmp:%s\n", VectorToString(vector<unsigned char>(tmpkey.begin(), tmpkey.end())));
			if (!sDecryptData(tmpkey, tmpdata, uint256(100), tmpout)) {
				return false;
			}
			tmpdata.assign(tmpout.begin(), tmpout.end());
//			LogTrace("spark", "the tmpout:%s\n", VectorToString(tmpdata));
			ii--;
		}
		vDecrypted = tmpdata;
	} else {
		return false;
	}
	return true;
}

bool CLotData::WriteCLotData(std::vector<unsigned char>& vcClearText, const CLottoFileKey &key) {

	std::vector<unsigned char> ckSecret;
	CLottoFileData sFileData(key.Index);

//    Assert(sFileData.data.vClearData.size() == vcClearText.size());

	sFileData.data.vClearData = vcClearText;
	sFileData.CalcHash();

	CDataStream te(SER_DISK, 100);
	te << sFileData.data;
	std::vector<unsigned char> ClearData(te.begin(), te.end());

	if (!EncryptData(key, ClearData, ckSecret)) {
		return false;
	}

	sFileData.SecretData = ckSecret;

	sFileData.print();

	return Write(key.Index, sFileData);
}

//index:the index in file
//ndata:the decrypted data
bool CLotData::ReadCLotData(std::vector<unsigned char>& vcClearText, const CLottoFileKey &key) {

	std::vector<unsigned char> ckSecret;

	CLottoFileData sFileData(key.Index);
	if (!Read(key.Index, sFileData)) {
		LogTrace("sws", "%s\n", "Read err");
		return false;
	}

	if (!DecryptData(key, sFileData.SecretData, ckSecret)) {
		LogTrace("sws", "%s\n", "DecryptData err");
		return false;
	}

	CDataStream te(SER_DISK, 100);
	std::vector<char> cktemp(ckSecret.begin(), ckSecret.end());
	te.insert(te.begin(), cktemp.begin(), cktemp.end());
	te >> sFileData.data;

	sFileData.print();

	uint256 sum = SerializeHash(sFileData.data);
	if (sFileData.data.getindex() == key.Index && sum == sFileData.sum) {
		vcClearText.clear();
		vcClearText.assign(sFileData.data.vClearData.begin(), sFileData.data.vClearData.end());
	} else {
		LogTrace("sws", "%s\n", "file data check sum err");
		return false;
	}
	return true;
}

#if 1

vector<unsigned char> GetRandomVec(int size) {
	vector<unsigned char> data;
	RandAddSeedPerfmon();
	data.resize(size);
	RAND_bytes(&data[0], size);
	return data;
}

bool CLotData::ReadKey(const char* filename, int index, CLottoFileKey& key) {
	ReadObjFromFile(filename, index, key);
	return true;
}

bool CLotData::CreateClottoFile(const char* filename, int size) {
	Assert(size > 0);
	boost::filesystem::path tmppath = GetDataDir() / filename;
	boost::filesystem::remove(tmppath);
	for (int ii = 0; ii < size; ii++)
	{
		vector<unsigned char> vClearData;
		CLottoFileKey key(ii);
		vector<uint256> tep;
		tep.clear();
		//Creat key
		for (int i = 0; i < 1; i++) {
			tep.push_back(uint256(GetRandomVec(32)));
		}
		key.setKey(tep);
		//Creat Clear Text
		vClearData = GetRandomVec(32);

		WriteCLotData(vClearData, key);

		std::vector<unsigned char> ndata;
		ReadCLotData(ndata, key);

		if (ndata != vClearData) {
			Assert(0);
		}
		AppendToFile(filename, key);

		CLottoFileKey key2;

		if (ReadObjFromFile(filename, ii, key2))
		{
			key2.print();
			key.print();
			if (key2.Index != ii || key2.getKey()[0] != key.getKey()[0])
			{
				Assert(0);
			}
		}
		else
		{
			Assert(0);
		}
	}

	return true;
}

uint256 CLotData::GetClottoFileHash() {
	CAutoFile fileout = CAutoFile(OpenFile(0, true), SER_DISK, CLIENT_VERSION);
	if (!fileout)
		return error("CLotDB::GetClottoFileHash : open failed");
	fseek(fileout, 0, SEEK_END);	//move the point to the end of file
	long filesize = ftell(fileout);	//get the file size
	if (filesize < 0)
		return error("CLotDB::GetClottoFileHash : ftell failed");
	rewind(fileout);	//move the point to the start of file

	vector<unsigned char> vchData;
	vchData.resize(filesize);

	try {
		fileout.read((char *) &vchData[0], filesize);
	} catch (std::exception &e) {
		return error("CLotDB::GetClottoFileHash : read failed");
	}
	fileout.fclose();

	uint256 fhash = Hash(vchData.begin(), vchData.end());

	return fhash;
}

#endif
