#include "stdafx.h"

namespace filerepo
{

ObjectStreamImpl::ObjectStreamImpl( const tstring &object_dir, size_t length, ObjectType type )
{
    object_dir_ = object_dir;
    tstring tmp_path = AppendPath(object_dir_, _T("tmp_object"));
    inited_to_write_ = file_buffer_.Open(tmp_path,
        FILEBUF_HASH_CONTENTS | FILEBUF_TEMPORARY | (1 << FILEBUF_DEFLATE_SHIFT)); // 1 is compress

    char hdr[64];
    int hdrlen = FormatObjectHeader(hdr, sizeof(hdr), length, type);
    if (file_buffer_.Write(hdr, hdrlen) != hdrlen) {
        inited_to_write_ = false;
    }
}

ObjectStreamImpl::ObjectStreamImpl(const tstring &object_dir, const ObjectId &oid)
{
    object_dir_ = object_dir;
    tstring file_path = GetObjectFilePath(oid);
    inited_to_read_ = file_read_buffer_.Open(file_path);
}

ObjectStreamImpl::~ObjectStreamImpl()
{

}

size_t ObjectStreamImpl::Read( char *buffer, size_t len )
{
    if (!inited_to_write_) {
        return 0;
    }
    return 0;// TODO
}

size_t ObjectStreamImpl::Write( const char *buffer, size_t len )
{
    if (!inited_to_write_) {
        return 0;
    }
    return file_buffer_.Write(buffer, len);
}

int ObjectStreamImpl::FinalizeWrite( ObjectId *id )
{
    ObjectId oid;
    if (!file_buffer_.GetHash(&oid)) {
        return HASH_FILE_FAILED;
    }
    tstring file_name = GetObjectFilePath(oid);
    if (!MakePathToFile(file_name)) {
        return CREATE_DIRECTORY_FAILED;
    }

    size_t i = 0;
    do {
        if (!IsFileExist(file_name)) {
            break;
        }
        ExtendObjectId(&oid);
        file_name = GetObjectFilePath(oid);
        i++;
    } while (i <= GetMaxExtendNumber());

    return file_buffer_.CommitAt(file_name, 0) ? SUCCEED : COMMIT_FILE_FAILED;
}

void ObjectStreamImpl::Free()
{
    delete this;
}

tstring ObjectStreamImpl::GetObjectFilePath( const ObjectId &id )
{
    return AppendPath(object_dir_, FormatObjectIdW(id));
}

} // namespace filerepo
