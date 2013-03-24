
#ifndef H_COMTYPES
#define H_COMTYPES

typedef unsigned char byte;

enum comtypes { 
	INT8 = 1, 
	INT16 = 2, 
	INT32 = 4, 
	INT64 = 8 
}; //these are the names of common types. Numbers represent size in Bytes

void tobin(long value,unsigned int type, byte *out);
long long bintolong(byte *c,unsigned int type);

#endif //H_COMTYPES