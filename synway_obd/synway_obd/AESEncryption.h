#pragma once
#include "EfAes.h"
#include "EncodeDecode.h"
class AESEncryption
{
	char * EncryptionKey;
public:
	AESEncryption(char* key);	
	char* DecodeAndDecrypt(char* EncodedandEncrypted);
	std::string EncryptAndEncode(char InputBuff[]);
};
