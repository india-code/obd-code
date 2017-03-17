#include "stdafx.h"
#include "AESEncryption.h"

AESEncryption::AESEncryption(char* key):EncryptionKey(key){}

char* AESEncryption::DecodeAndDecrypt(char* EncodedandEncrypted)
{
	AesCtx context;
	char* DecryptedBuff = new char[31];
	AesSetKey(&context, AES_KEY_128BIT, BLOCKMODE_ECB, EncryptionKey, NULL);
	std::string decoded = base64_decode(EncodedandEncrypted);
	char *c = const_cast<char*>(decoded.c_str());
	AesDecryptECB(&context, DecryptedBuff, c, 16);
	for (int i = 0; i < strlen(DecryptedBuff); i++)
	{
		if ((int)DecryptedBuff[i] > 57 || (int)DecryptedBuff[i] < 48)
		{
			DecryptedBuff[i] = '\0';
		}
	}
	return DecryptedBuff;
}

char* AESEncryption::EncryptAndEncode(char* InputBuff)
{
	AesCtx context;
	char EncryptedBuff[31];
	AesSetKey(&context, AES_KEY_128BIT, BLOCKMODE_ECB, EncryptionKey, NULL);
	AesEncryptECB(&context, EncryptedBuff, InputBuff, 16);
	const std::string s = EncryptedBuff;
	std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
	return const_cast<char*>(encoded.c_str());
}