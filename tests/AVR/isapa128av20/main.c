#include <string.h>
#include <stdint.h>
#include <Arduino.h>
#include "src/aead/crypto_aead.h"

unsigned char key[16] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
unsigned char nonce[16] = {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a'};
unsigned char msg2[10];
unsigned char msg[10] = {'A','L','N','w','M','S','i','l','W','O'};
unsigned char ct[26] ;
unsigned long long clen = 26, mlen2;

void setup(void) {}

void loop(void){
  crypto_aead_encrypt(ct, &clen, msg,  sizeof(msg), NULL, 0, NULL,nonce, key);
  asm volatile("jmp 0x8000"); // stop QEMU
}