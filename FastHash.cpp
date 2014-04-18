#include "stdafx.h"
#include <stdint.h>

namespace filerepo
{

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
    || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
    +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

unsigned int FastHash(const char * data, int len) {
    unsigned int hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL)
        return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--) {
        hash  += get16bits(data);
        tmp    = (get16bits(data + 2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2 * sizeof(unsigned short);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
    case 3:
        hash += get16bits(data);
        hash ^= hash << 16;

        // Treat the final character as signed. This ensures all platforms behave
        // consistently with the original x86 code.
        hash ^= static_cast<signed char>(data[sizeof(unsigned short)]) << 18;
        hash += hash >> 11;
        break;
    case 2:
        hash += get16bits(data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1:
        hash += static_cast<signed char>(*data);
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

unsigned int FastHash( const std::string data )
{
    return FastHash(data.c_str(), data.length());
}

}