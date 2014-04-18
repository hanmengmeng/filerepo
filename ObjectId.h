#ifndef FILE_ID_H
#define FILE_ID_H

namespace filerepo
{

// Size (in bytes) of a raw/binary oid
// Extra 4 bits used to solve hash conflict
const int OBJECT_ID_RAWSZ = 20;

const int OBJECT_ID_EXTEND_RAWSZ = 4;

// Size (in bytes) of a hex formatted oid
//const int OBJECT_ID_HEXSZ = (OBJECT_ID_RAWSZ * 2);

//const int FILE_ID_MINPREFIXLEN = 4;

typedef struct _Object_Id {
    unsigned char id[OBJECT_ID_RAWSZ];
    unsigned char extend_id[OBJECT_ID_EXTEND_RAWSZ];
}ObjectId;

std::string FormatObjectId(const ObjectId &oid);

tstring FormatObjectIdW(const ObjectId &oid);

void FormatToObjectId(const std::string &format_str, ObjectId *oid);

void FormatToObjectIdW(const std::string &format_str, ObjectId *oid);

void HashFile(const tstring &file_path, ObjectId *oid);

void ExtendObjectId(ObjectId *oid);

size_t GetMaxExtendNumber();

}
#endif
