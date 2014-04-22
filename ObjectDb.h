#ifndef OBJECT_DB_H_H
#define OBJECT_DB_H_H

namespace filerepo
{
class ObjectWriteStreamImpl;
class ObjectReadStreamImpl;

class ObjectStream
{
public:
    virtual ~ObjectStream() {}

    virtual size_t Read(char *buffer, size_t len) = 0;

    virtual size_t Write(const char *buffer, size_t len) = 0;

    virtual int FinalizeWrite(ObjectId *id) = 0;

    virtual void Close() = 0;
};

class ObjectDb
{
public:
    ObjectDb(const tstring &object_db_dir);

    ~ObjectDb();

    bool ReadObjectHeader(const ObjectId &id, size_t *len_p, ObjectType *type_p);

    bool IsObjectExist(const ObjectId &id);

    ObjectStream *OpenWriteStream(size_t size, ObjectType type);

    ObjectStream *OpenReadStream(const ObjectId &id);

    int AddBlobObject(const tstring &file_path, ObjectId *oid);

    int AddBlobObject(void *buf, size_t len, ObjectId *oid);

private:
    tstring GetObjectFilePath( const ObjectId &id );

    bool RemoveDuplicateObject(const ObjectId &id);

private:
    tstring object_db_dir_;
    ObjectWriteStreamImpl *write_stream_;
    ObjectReadStreamImpl *read_stream_;

    DISALLOW_COPY_AND_ASSIGN(ObjectDb);
    friend class FileRepoImpl;
};

}
#endif