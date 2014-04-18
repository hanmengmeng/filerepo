#include "stdafx.h"

namespace filerepo
{
const size_t WRITE_BUFFER_SIZE = (4096 * 2);

enum buferr_t {
    BUFERR_OK = 0,
    BUFERR_WRITE,
    BUFERR_ZLIB,
    BUFERR_MEM
};

FileBuffer::FileBuffer()
{
    do_not_buffer_ = false;
    do_not_compress_ = false;

    buffer_ = NULL;
    z_buf_ = NULL;
    digest_ = NULL;

    buf_size_ = 0;
    buf_pos_ = 0;
    handle_ = INVALID_HANDLE_VALUE;
    last_error_ = 0;
    write_ = NULL;
}

FileBuffer::~FileBuffer()
{
    CleanUp();
}

bool FileBuffer::Open( const tstring &path, int flags )
{
    if (flags & FILEBUF_DO_NOT_BUFFER) {
        do_not_buffer_ = true;
    }

    buf_size_ = WRITE_BUFFER_SIZE;
    buf_pos_ = 0;
    handle_ = INVALID_HANDLE_VALUE;
    last_error_ = BUFERR_OK;

    if (!do_not_buffer_) {
        buffer_ = new unsigned char[buf_size_];
    }

    if (flags & FILEBUF_HASH_CONTENTS) {
        digest_ = hash_new_ctx();
        CHECK_ALLOC(digest_);
    }

    int compression = flags >> FILEBUF_DEFLATE_SHIFT;
    if (compression != 0) {
        /* Initialize the ZLib stream */
        if (deflateInit(&zs_, compression) != Z_OK) {
            return false;
        }

        /* Allocate the Zlib cache buffer */
        z_buf_ = new unsigned char[buf_size_];
        CHECK_ALLOC(z_buf_);

        /* Never flush */
        flush_mode_ = Z_NO_FLUSH;
        do_not_compress_ = false;
        write_ = &FileBuffer::WriteDeflate;
    } else {
        do_not_compress_ = true;
        write_ = &FileBuffer::WriteNormal;
    }

    tstring tmp_path;
    /* Open the file as temporary for locking */
    if (!MakeTempFile(path, &tmp_path)) {
        return false;
    }

    handle_ = CreateFile(tmp_path.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        0,
        NULL);

    if (INVALID_HANDLE_VALUE == handle_) {
        return false;
    }

    path_origin_ = _T("");
    path_lock_ = tmp_path;

    return true;
}

size_t FileBuffer::Write( const void *buff, size_t len )
{
    const unsigned char *buf = (const unsigned char *)buff;

    if (do_not_buffer_) {
        return (this->*write_)((void *)buff, len);
    }

    // Actual data len of written to disk
    size_t written = 0;
    for (;;) {
        size_t space_left = buf_size_ - buf_pos_;

        /* cache if it's small */
        if (space_left > len) {
            AddToCache(buf, len);
            written += len;
            return written;
        }

        AddToCache(buf, space_left);
        if (FlushBuffer() == 0) { // Error
            return written;
        }

        len -= space_left;
        buf += space_left;
        written += space_left;
    }
}

bool FileBuffer::Reserve( void **buffer, size_t len )
{
    size_t space_left = buf_size_ - buf_pos_;
    *buffer = NULL;

    if (len > buf_size_) {
        last_error_ = BUFERR_MEM;
        return false;
    }

    if (space_left <= len) {
        if (FlushBuffer() == 0) {
            return false;
        }
    }

    *buffer = (buffer_ + buf_pos_);
    buf_pos_ += len;
    return true;
}

size_t FileBuffer::Printf( const char *format, ... )
{
    size_t written = 0;
    va_list arglist;
    size_t space_left;
    int len;

    space_left = buf_size_ - buf_pos_;

    do {
        va_start(arglist, format);
        len = _vsnprintf_s((char *)buffer_ + buf_pos_, space_left, _TRUNCATE, format, arglist);
        va_end(arglist);

        if (len < 0) {
            last_error_ = BUFERR_MEM;
            return 0;
        }

        if ((size_t)len + 1 <= space_left) {
            buf_pos_ += len;
            written += len;
            return written;
        }

        if (FlushBuffer() == 0) {
            return 0;
        }

        space_left = buf_size_ - buf_pos_;
    } while ((size_t)len + 1 <= space_left);

    char *tmp_buffer = new char[len + 1];
    if (!tmp_buffer) {
        last_error_ = BUFERR_MEM;
        return -1;
    }

    va_start(arglist, format);
    len = _vsnprintf_s(tmp_buffer, len + 1, _TRUNCATE, format, arglist);
    va_end(arglist);

    if (len < 0) {
        delete []tmp_buffer;
        last_error_ = BUFERR_MEM;
        return false;
    }

    written += Write(tmp_buffer, len);
    delete []tmp_buffer;

    return written;
}

bool FileBuffer::Commit( mode_t mode )
{
    flush_mode_ = Z_FINISH;
    FlushBuffer();

    if (last_error_ != BUFERR_OK) {
        return false;
    }

    CloseHandle(handle_);
    handle_ = INVALID_HANDLE_VALUE;

    if (!RenameFile(path_lock_, path_origin_)) {
        return false;
    }
    return true;
}

bool FileBuffer::CommitAt( const tstring &path, mode_t mode )
{
    path_origin_ = path;
    return Commit(mode);
}

size_t FileBuffer::Flush()
{
    return FlushBuffer();
}

size_t FileBuffer::WriteNormal( void *source, size_t len )
{
    DWORD written = 0;
    if (len > 0) {
        if (!::WriteFile(handle_, (void *)source, len, &written, NULL)) {
            last_error_ = BUFERR_WRITE;
            return 0;
        }

        if (digest_) {
            hash_update(digest_, source, len);
        }
    }
    return written;
}

size_t FileBuffer::WriteDeflate( void *source, size_t len )
{
    DWORD written = 0;

    if (len > 0 || flush_mode_ == Z_FINISH) {
        z_stream *zs = &zs_;
        zs->next_in = (Bytef*)source;
        zs->avail_in = (uInt)len;

        do {
            size_t have;
            zs->next_out = z_buf_;
            zs->avail_out = (uInt)buf_size_;

            if (deflate(zs, flush_mode_) == Z_STREAM_ERROR) {
                last_error_ = BUFERR_ZLIB;
                return 0;
            }

            have = buf_size_ - (size_t)zs->avail_out;
            if (::WriteFile(handle_, z_buf_, have, &written, NULL) < 0) {
                last_error_ = BUFERR_WRITE;
                return 0;
            }
        } while (zs->avail_out == 0);

        assert(zs->avail_in == 0);
        if (digest_) {
            hash_update(digest_, source, len);
        }
    }
    return written;
}

void FileBuffer::AddToCache( const void *buf, size_t len )
{
    memcpy(buffer_ + buf_pos_, buf, len);
    buf_pos_ += len;
}

size_t FileBuffer::FlushBuffer()
{
    size_t result = (this->*write_)(buffer_, buf_pos_);
    buf_pos_ = 0;
    return result;
}

void FileBuffer::CleanUp()
{
    if (NULL != buffer_) {
        delete []buffer_;
        buffer_ = NULL;
    }
    if (NULL != z_buf_) {
        delete []z_buf_;
        z_buf_ = NULL;
        deflateEnd(&zs_);
    }
    if (NULL != digest_) {
        hash_free_ctx(digest_);
        digest_ = NULL;
    }
    if (INVALID_HANDLE_VALUE != handle_) {
        ::CloseHandle(handle_);
        handle_ = NULL;
    }
}

bool FileBuffer::GetHash( ObjectId *oid )
{
    FlushBuffer();

    if (last_error_ != BUFERR_OK) {
        return false;
    }

    hash_final(oid, digest_);
    hash_free_ctx(digest_);
    digest_ = NULL;
    return true;
}

}