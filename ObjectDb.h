#ifndef OBJECT_DB_H_H
#define OBJECT_DB_H_H

namespace filerepo
{
class ObjectStream
{
public:
    virtual ~ObjectStream() {}
    virtual size_t Read(char *buffer, size_t len) = 0;
    virtual size_t Write(const char *buffer, size_t len) = 0;
    virtual int FinalizeWrite(ObjectId *id) = 0;
    virtual void Free() = 0;
};

class ObjectDb
{
public:
    ObjectDb(const tstring &object_db_dir);

    bool ReadObjectHeader(const ObjectId &id, size_t *len_p, ObjectType *type_p);

    bool IsObjectExist(const ObjectId &id);

    ObjectStream *OpenWriteStream(size_t size, ObjectType type);

    ObjectStream *OpenReadStream(const ObjectId &id);

private:
    tstring GetObjectFilePath( const ObjectId &id );

private:
    tstring object_db_dir_;
};

}
#endif