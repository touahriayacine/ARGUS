#include <stdio.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "esp_task_wdt.h"

#include "src/api.h"
#include "src/crypto_aead.h"

#define DATA_SIZE 10

static const unsigned char m[DATA_SIZE] = {'a','b','c','d','e','f','g','h','i','j'};
const unsigned long long mlen = DATA_SIZE;

const unsigned char *ad = NULL;
const unsigned long long adlen = 0;

unsigned char k[CRYPTO_KEYBYTES];
unsigned char npub[CRYPTO_NPUBBYTES];

#if crypto_aead_NSECBYTES > 0
    unsigned char nsec[CRYPTO_NSECBYTES];
    unsigned char nsec_out[CRYPTO_NSECBYTES];
#else
    unsigned char *nsec = NULL;
    unsigned char *nsec_out = NULL;
#endif

unsigned char c[DATA_SIZE + CRYPTO_ABYTES];
unsigned long long clen = DATA_SIZE + CRYPTO_ABYTES;

void app_main(void)
{
    
    esp_task_wdt_deinit();
    
    for (size_t i = 0; i < sizeof k; i++) k[i] = (unsigned char)i;
    for (size_t i = 0; i < sizeof npub; i++) npub[i] = (unsigned char)(0xA0 + i);
    #if crypto_aead_NSECBYTES > 0
        for (size_t i = 0; i < sizeof nsec; i++) nsec[i] = (unsigned char)(0x50 + i);
    #endif

    crypto_aead_encrypt(c, &clen, m, mlen, ad, adlen, nsec, npub, k);

    printf("finished");
}