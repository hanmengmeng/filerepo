#include "stdafx.h"

namespace filerepo
{

static char to_hex[] = "0123456789abcdef";

std::string FormatObjectId(const ObjectId &oid)
{
    std::string str;
    for (size_t i = 0; i < sizeof(oid.id); i++) {
        str.push_back(to_hex[oid.id[i] >> 4]);
        str.push_back(to_hex[oid.id[i] & 0xf]);
    }
    for (size_t i = 0; i < sizeof(oid.extend_id); i++) {
        if (oid.extend_id[i] == 0) {
            break;
        }
        str.push_back(to_hex[oid.extend_id[i] >> 4]);
        str.push_back(to_hex[oid.extend_id[i] & 0xf]);
    }
    return str;
}

void FormatToObjectId( const std::string &format_str, ObjectId *oid )
{
}

void ExtendObjectId( ObjectId *oid )
{
    assert(OBJECT_ID_EXTEND_RAWSZ == sizeof(int));
    int *extend = (int*)&oid->extend_id;
    *extend += 1;
}

size_t GetMaxExtendNumber()
{
    return 65535;
}

tstring FormatObjectIdW( const ObjectId &oid )
{
    std::string format_str = FormatObjectId(oid);
    return utf8_to_16(format_str.c_str());
}

void FormatToObjectIdW( const std::string &format_str, ObjectId *oid )
{

}

void CopyObjectId( const ObjectId *src, ObjectId *target )
{
    memcpy(target, src, sizeof(*src));
}

bool IsExtendObjectId( const ObjectId &oid )
{
    assert(OBJECT_ID_EXTEND_RAWSZ == sizeof(int));
    int *extend = (int*)&oid.extend_id;
    return *extend > 0;
}

bool IsObjectEqual( const ObjectId &a, const ObjectId &b )
{
    const unsigned char *sha1 = a.id;
    const unsigned char *sha2 = b.id;
    for (int i = 0; i < OBJECT_ID_RAWSZ; i++, sha1++, sha2++) {
        if (*sha1 != *sha2) {
            return false;
        }
    }

#if 0 // Not compare extend id
    const unsigned char *e1 = a.extend_id;
    const unsigned char *e2 = b.extend_id;
    for (size_t i = 0; i < OBJECT_ID_EXTEND_RAWSZ; i++, sha1++, sha2++) {
        if (*e1 != *e2) {
            return false;
        }
    }
#endif
    return true;
}

} // namespace filerepo
