#pragma once
#include "EfAes.h"
#include "EncodeDecode.h"
class AESEncryption
{
	char * EncryptionKey;
public:
	AESEncryption(char* key);	
	std::string DecodeAndDecrypt(char* EncodedandEncrypted);
	std::string EncryptAndEncode(char InputBuff[]);
};
