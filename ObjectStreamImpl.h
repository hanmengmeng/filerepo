#ifndef OBJECT_STREAM_IMPL_H
#define OBJECT_STREAM_IMPL_H

#include "FileBuffer.h"
#include "FileReadBuffer.h"

namespace filerepo
{

class ObjectWriteStreamImpl : public ObjectStream
{
public:
    // Write mode
    ObjectWriteStreamImpl(const tstring &object_dir);
    ~ObjectWriteStreamImpl();

    bool Open(size_t length, ObjectType type);
    bool IsOpened();

    // Implement ObjectStream
    virtual size_t Read(char *buffer, size_t len) OVERRIDE;
    virtual size_t Write(const char *buffer, size_t len) OVERRIDE;
    virtual int FinalizeWrite(ObjectId *id) OVERRIDE;
    virtual void Close() OVERRIDE;

private:
    tstring object_dir_;
    FileBuffer file_buffer_;
    bool inited_to_write_;

    DISALLOW_COPY_AND_ASSIGN(ObjectWriteStreamImpl);
};

class ObjectReadStreamImpl : public ObjectStream
{
public:
    ObjectReadStreamImpl(const tstring &object_dir);
    ~ObjectReadStreamImpl();

    bool Open(const ObjectId &oid);
    bool IsOpened();

    // Implement ObjectStream
    virtual size_t Read(char *buffer, size_t len) OVERRIDE;
    virtual size_t Write(const char *buffer, size_t len) OVERRIDE;
    virtual int FinalizeWrite(ObjectId *id) OVERRIDE;
    void Close() OVERRIDE;

private:
    tstring object_dir_;
    FileReadBuffer file_read_buffer_;
    bool inited_to_read_;

    DISALLOW_COPY_AND_ASSIGN(ObjectReadStreamImpl);
};

}
#endif
