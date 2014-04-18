#include "stdafx.h"

namespace filerepo
{
struct stObjectType
{
    char *type_string;
    ObjectType type;
};
struct stObjectType g_object_type_strings[] = {
    {"", OBJ_UNKNOWN},
    {"blob", OBJ_BLOB},
};

int FormatObjectHeader(char *hdr, size_t n, size_t obj_len, ObjectType obj_type)
{
    std::string type_str = ObjectTypeToString(obj_type);
    int len = _snprintf_s(hdr, n, _TRUNCATE, "%s %Iu", type_str.c_str(), obj_len);

    assert(len > 0);
    assert(((size_t)len) < n);

    return len+1;
}

std::string ObjectTypeToString( ObjectType obj_type )
{
    const int count = sizeof(g_object_type_strings)/sizeof(g_object_type_strings[0]);
    for (size_t i = 0; i < count; i++) {
        if (g_object_type_strings[i].type == obj_type) {
            return g_object_type_strings[i].type_string;
        }
    }
    return "";
}

ObjectType ObjectStringToType( const std::string &object_type_str )
{
    const int count = sizeof(g_object_type_strings)/sizeof(g_object_type_strings[0]);
    for (size_t i = 0; i < count; i++) {
        if (object_type_str == g_object_type_strings[i].type_string) {
            return g_object_type_strings[i].type;
        }
    }
    return OBJ_UNKNOWN;
}

}