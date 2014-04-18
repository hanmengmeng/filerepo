#include "stdafx.h"

namespace filerepo
{

ObjectDb::ObjectDb( const tstring &object_db_dir )
{
    object_db_dir_ = object_db_dir;
}

bool ObjectDb::ReadObjectHeader( const ObjectId &id, size_t *len_p, ObjectType *type_p )
{
    tstring object_path = GetObjectFilePath(id);
    FileReadBuffer read_buffer;
    if (!read_buffer.Open(object_path)) {
        return false;
    }
    *len_p = read_buffer.GetSize();
    *type_p = read_buffer.GetType();
    return true;
}

bool ObjectDb::IsObjectExist( const ObjectId &id )
{
    tstring object_path = GetObjectFilePath(id);
    return IsFileExist(object_path);
}

ObjectStream * ObjectDb::OpenWriteStream( size_t size, ObjectType type )
{
    return new ObjectStreamImpl(object_db_dir_, size, type);
}

ObjectStream * ObjectDb::OpenReadStream( const ObjectId &id )
{
    return new ObjectStreamImpl(object_db_dir_, id);
}

tstring ObjectDb::GetObjectFilePath( const ObjectId &id )
{
    return AppendPath(object_db_dir_, FormatObjectIdW(id));
}

}