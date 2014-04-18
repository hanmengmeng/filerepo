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

} // namespace filerepo
