#ifndef OBJECT_TYPE_H
#define OBJECT_TYPE_H

namespace filerepo
{

typedef enum {
    OBJ_UNKNOWN = 0,
    OBJ_BLOB = 1,
} ObjectType;


int FormatObjectHeader(char *hdr, size_t n, size_t obj_len, ObjectType obj_type);

std::string ObjectTypeToString(ObjectType obj_type);

ObjectType ObjectStringToType(const std::string &object_type_str);

}
#endif
