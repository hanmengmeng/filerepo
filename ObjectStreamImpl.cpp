#include "stdafx.h"

namespace filerepo
{

tstring GetObjectFilePath(const tstring& object_dir, const ObjectId &id)
{
    return AppendPath(object_dir, FormatObjectIdW(id));
}

ObjectWriteStreamImpl::ObjectWriteStreamImpl(const tstring &object_dir)
{
    object_dir_ = object_dir;
    inited_to_write_ = false;
}

ObjectWriteStreamImpl::~ObjectWriteStreamImpl()
{
    Close();
}

size_t ObjectWriteStreamImpl::Read( char *buffer, size_t len )
{
    return 0;
}

size_t ObjectWriteStreamImpl::Write( const char *buffer, size_t len )
{
    if (!IsOpened()) {
        return 0;
    }
    return file_buffer_.Write(buffer, len) < 0 ? 0 : len;
}

int ObjectWriteStreamImpl::FinalizeWrite( ObjectId *id )
{
    if (!IsOpened()) {
        return FILE_NOT_OPENED;
    }
    ObjectId oid;
    if (!file_buffer_.GetHash(&oid)) {
        return HASH_FILE_FAILED;
    }
    tstring file_name = GetObjectFilePath(object_dir_, oid);
    if (!MakePathToFile(file_name)) {
        return CREATE_DIRECTORY_FAILED;
    }

    size_t i = 0;
    do {
        if (!IsFileExist(file_name)) {
            break;
        }
        ExtendObjectId(&oid);
        file_name = GetObjectFilePath(object_dir_, oid);
        i++;
    } while (i <= GetMaxExtendNumber());

    CopyObjectId(&oid, id);
    return file_buffer_.CommitAt(file_name, 0) ? SUCCEED : COMMIT_FILE_FAILED;
}

bool ObjectWriteStreamImpl::Open( size_t length, ObjectType type )
{
    if (file_buffer_.IsOpened()) {
        return false;
    }
    bool ret = false;
    tstring tmp_path = AppendPath(object_dir_, _T("tmp_object"));
    ret = file_buffer_.Open(tmp_path,
        FILEBUF_HASH_CONTENTS | FILEBUF_TEMPORARY | (1 << FILEBUF_DEFLATE_SHIFT)); // 1 is compress

    char hdr[64];
    int hdrlen = FormatObjectHeader(hdr, sizeof(hdr), length, type);
    if (file_buffer_.Write(hdr, hdrlen) < 0) {
        ret = false;
    }
    inited_to_write_ = ret;
    return ret;
}

void ObjectWriteStreamImpl::Close()
{
    file_buffer_.CleanUp();
    inited_to_write_ = false;
}

bool ObjectWriteStreamImpl::IsOpened()
{
    return inited_to_write_;
}

ObjectReadStreamImpl::ObjectReadStreamImpl( const tstring &object_dir )
{
    inited_to_read_ = false;
    object_dir_ = object_dir;
}

ObjectReadStreamImpl::~ObjectReadStreamImpl()
{
    Close();
}

size_t ObjectReadStreamImpl::Read( char *buffer, size_t len )
{
    if (!IsOpened()) {
        return 0;
    }
    return file_read_buffer_.Read(buffer, len);
}

size_t ObjectReadStreamImpl::Write( const char *buffer, size_t len )
{
    assert(0); // FAILED
    return 0;
}

int ObjectReadStreamImpl::FinalizeWrite( ObjectId *id )
{
    assert(0); // FAILED
    return UNIMPLEMENT;
}

bool ObjectReadStreamImpl::Open( const ObjectId &oid )
{
    if (file_read_buffer_.IsOpened()) {
        return false;
    }
    bool ret = false;
    tstring file_path = GetObjectFilePath(object_dir_, oid);
    ret = file_read_buffer_.Open(file_path);
    inited_to_read_ = ret;
    return ret;
}

void ObjectReadStreamImpl::Close()
{
    file_read_buffer_.CleanUp();
    inited_to_read_ = false;
}

bool ObjectReadStreamImpl::IsOpened()
{
    return inited_to_read_;
}

} // namespace filerepo
