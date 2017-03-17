#pragma once
#include "EfAes.h"
#include "EncodeDecode.h"
class AESEncryption
{
	char * EncryptionKey;
public:
	AESEncryption(char* key);	
	char* DecodeAndDecrypt(char* EncodedandEncrypted);
	char* EncryptAndEncode(char* InputBuff);
};
