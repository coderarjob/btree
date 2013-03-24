/* Filename: comtype.c
 * Author: Arjob Mukherjee
 * Dated: 22 Sep 2012
 * 
 * Project: Izieu-omnirover-linux v0.1
 * 
 * This file contains the 'Type Converter' part of Izieu API. 
 * The c language types are converted to a common, system and compiller independent type, so that other implementations of Izieu can read the database file.
 */
#include <stdio.h>
#include "comtype.h"

//Binary masks
#define MASK 0xFF
#define BYTELENGTH 8

void* malloc(int);

/*
 * Converts value to a type. Returns byte array.
 */
void tobin(long value,unsigned int type, byte *out) {
  
  //Convert to byte array of length depending on type
  
  int i;
  
  for (i = 0; i < type; i++)
  {
    unsigned char num = (value | ~MASK) - (~MASK);
    *out++ = num;
    //printf("%ld : %d\n", value | ~MASK,  num);
    value = value >> BYTELENGTH;
  }

}

/*
 * Converts byte array containing binary value of a perticular type to long long.
 * Returns long long. long long is the most restrictive type.
 * Augument:
 * type could be the common types or sizeof(<c types>)
 */
long long bintolong(byte *c,unsigned int type)
{
  long num = 0 + c[type - 1];
  int i;
  for (i = type-2; i > -1; --i)
  {
    num = num << BYTELENGTH;
    num = num + c[i];
  }
  
  return num;
}



