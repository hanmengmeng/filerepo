#include "stdafx.h"

namespace filerepo
{

ObjectDb::ObjectDb( const tstring &object_db_dir )
{
    object_db_dir_ = object_db_dir;
    write_stream_ = new ObjectWriteStreamImpl(object_db_dir_);
    read_stream_ = new ObjectReadStreamImpl(object_db_dir_);
}

ObjectDb::~ObjectDb()
{
    delete write_stream_;
    delete read_stream_;
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
    if (write_stream_->IsOpened()) {
        write_stream_->Close();
    }
    if (!write_stream_->Open(size, type)) {
        return NULL;
    }
    return write_stream_;
}

ObjectStream * ObjectDb::OpenReadStream( const ObjectId &id )
{
    if (read_stream_->IsOpened()) {
        read_stream_->Close();
    }
    if (!read_stream_->Open(id)) {
        return NULL;
    }
    return read_stream_;
}

tstring ObjectDb::GetObjectFilePath( const ObjectId &id )
{
    return AppendPath(object_db_dir_, FormatObjectIdW(id));
}

int ObjectDb::AddBlobObject( const tstring &file_path, ObjectId *oid )
{
    size_t file_size = static_cast<size_t>(GetFileSize(file_path));
    ObjectStream *os = OpenWriteStream(file_size, OBJ_BLOB);
    if (!os) {
        return OPEN_WRITE_STREAM_FAILED;
    }

    HANDLE handle = OpenFileToRead(file_path);
    if (!IsOpenSucceed(handle)) {
        os->Close();
        return OPEN_FILE_FAILED;
    }

    int ret = SUCCEED;
    const DWORD buf_size = 40960;
    char buf[buf_size] = {0};
    DWORD read_size;
    while (::ReadFile(handle, buf, buf_size, &read_size, NULL) && read_size > 0) {
        if (os->Write(buf, read_size) != read_size) {
            ret = WRITE_FILE_FAILED;
            break;
        }
    }
    if (IS_SUCCEED(ret)) {
        ObjectId id;
        ret = os->FinalizeWrite(&id);
        if (IS_SUCCEED(ret)) {
            CopyObjectId(&id, oid);
        }
    }
    ::CloseHandle(handle);
    os->Close();
    return ret;
}

int ObjectDb::AddBlobObject( void *buf, size_t len, ObjectId *oid )
{
    ObjectStream *os = OpenWriteStream(len, OBJ_BLOB);
    if (!os) {
        return OPEN_WRITE_STREAM_FAILED;
    }

    int ret = SUCCEED;
    if (os->Write((char*)buf, len) != len) {
        ret = WRITE_FILE_FAILED;
    }
    if (IS_SUCCEED(ret)) {
        ObjectId id;
        ret = os->FinalizeWrite(&id);
        if (IS_SUCCEED(ret)) {
            CopyObjectId(&id, oid);
        }
    }
    os->Close();
    return ret;
}

bool ObjectDb::RemoveDuplicateObject( const ObjectId &id )
{
    // Only remove object has extend id
    if (!IsExtendObjectId(id)) {
        return false;
    }
    tstring obj_path = GetObjectFilePath(id);
    return ::DeleteFile(obj_path.c_str()) == TRUE;
}

}
