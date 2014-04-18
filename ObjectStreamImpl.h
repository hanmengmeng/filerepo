#ifndef OBJECT_STREAM_IMPL_H
#define OBJECT_STREAM_IMPL_H

#include "FileBuffer.h"
#include "FileReadBuffer.h"

namespace filerepo
{

class ObjectStreamImpl : public ObjectStream
{
public:
    // Write mode
    ObjectStreamImpl(const tstring &object_dir, size_t length, ObjectType type);
    // Read mode
    ObjectStreamImpl(const tstring &object_dir, const ObjectId &oid);
    ~ObjectStreamImpl();

    // Implement ObjectStream
    virtual size_t Read(char *buffer, size_t len) override;
    virtual size_t Write(const char *buffer, size_t len) override;
    virtual int FinalizeWrite(ObjectId *id) override;
    virtual void Free() override;

private:
    tstring GetObjectFilePath(const ObjectId &id);

private:
    tstring object_dir_;
    ObjectId oid_;
    FileBuffer file_buffer_;
    bool inited_to_write_;
    FileReadBuffer file_read_buffer_;
    bool inited_to_read_;
};

}
#endif
