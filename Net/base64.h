#pragma once
#include <string>
#include <vector>

// преобразовать бинарные данные в текстовые base64
void Base64Encode( const void * binaryData, unsigned int dataLength, std::string & base64Data );
//void Base64Encode_old( const void * binaryData, unsigned int dataLength, std::string & base64Data );

// преобразовать текстовые base64 в бинарные данные
void Base64Decode( const unsigned char * base64Data, unsigned int dataLength, std::vector<unsigned char> & binaryData );


void Base64Fake( const void * binaryData, unsigned int dataLength, std::string & base64Data );
