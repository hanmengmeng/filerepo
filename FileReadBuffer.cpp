#include "stdafx.h"

namespace filerepo
{

FileReadBuffer::FileReadBuffer()
{
    handle_ = INVALID_HANDLE_VALUE;
    len_ = 0;
    headerlen_ = 0;;
    tail_ = 0;
    type_ = OBJ_UNKNOWN;
    memset(headerbuf_, 0, sizeof(headerbuf_));
    memset(rawbuf_, 0, sizeof(rawbuf_));
}

FileReadBuffer::~FileReadBuffer()
{
    CleanUp();
}

bool FileReadBuffer::Open( const tstring &file_path )
{
    DWORD readlen = 0;
    size_t headerlen = 0;
    int z_return = Z_OK;
    obj_hdr header_obj;

    handle_ = CreateFile(file_path.c_str(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (INVALID_HANDLE_VALUE == handle_) {
        return false;
    }

    InitStream(&zs_, headerbuf_, sizeof(headerbuf_));
    z_return = inflateInit(&zs_);

    // Inflate odb file header
    while (z_return == Z_OK && zs_.total_out != ODB_HEADER_MAX_LEN) {
        if (::ReadFile(handle_, rawbuf_, sizeof(rawbuf_), &readlen, NULL) && readlen > 0) {
            SetStreamInput(&zs_, rawbuf_, readlen);
            z_return = inflate(&zs_, 0);
        }
        else {
            z_return = Z_STREAM_END;
        }
    }

    if (z_return != Z_STREAM_END && z_return != Z_OK) {// Inflate error
        return false;
    }

    if ((headerlen = GetObjectHeader(&header_obj, headerbuf_)) == 0) {
        return false;
    }
    else {
        len_ = header_obj.size;
        type_ = header_obj.type;
        headerlen_ = headerlen;
        tail_ = zs_.total_out - headerlen_;
    }

    return true;
}

size_t FileReadBuffer::Read( void *buf, size_t len )
{
    int z_return = Z_OK;
    DWORD readlen = 0;

    if (tail_ > 0) {
        if (tail_ > len) { // buf too small
            return 0; // TODO
        }
        memcpy(buf, headerbuf_+headerlen_, tail_);
        SetStreamOutput(&zs_, (char*)buf+tail_, len-tail_);
        tail_ = 0;
    }
    else {
        SetStreamOutput(&zs_, buf, len);
    }

    if (zs_.avail_in > 0) {
        z_return = inflate(&zs_, 0);
    }

    while (z_return == Z_OK && zs_.avail_out > 0) {
        if (::ReadFile(handle_, rawbuf_, sizeof(rawbuf_), &readlen, NULL) && readlen > 0) {
            SetStreamInput(&zs_, rawbuf_, readlen);
            z_return = inflate(&zs_, 0);
        }
        else {
            z_return = Z_STREAM_END;
        }
    }

    if (Z_OK != z_return && Z_STREAM_END != z_return) {
        return 0;
    }

    return len - zs_.avail_out;
}

void FileReadBuffer::CleanUp()
{
    if (INVALID_HANDLE_VALUE != handle_) {
        inflateEnd(&zs_);
        ::CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
}

void FileReadBuffer::InitStream(z_stream *s, void *out, size_t len)
{
    memset(s, 0, sizeof(*s));
    s->next_out = (Bytef*)out;
    s->avail_out = (uInt)len;
}

void FileReadBuffer::SetStreamInput(z_stream *s, void *in, size_t len)
{
    s->next_in = (Bytef*)in;
    s->avail_in = (uInt)len;
}

void FileReadBuffer::SetStreamOutput(z_stream *s, void *out, size_t len)
{
    s->next_out = (Bytef*)out;
    s->avail_out = (uInt)len;
}

size_t FileReadBuffer::GetObjectHeader(obj_hdr *hdr, unsigned char *data)
{
    char c, type_name[10];
    size_t size, used = 0;

    /*
    * type name string followed by space.
    */
    while ((c = data[used]) != ' ') {
        type_name[used++] = c;
        if (used >= sizeof(type_name))
            return 0;
    }
    type_name[used] = 0;
    if (used == 0)
        return 0;
    hdr->type = ObjectStringToType(type_name);
    used++; /* consume the space */

    /*
    * length follows immediately in decimal (without
    * leading zeros).
    */
    size = data[used++] - '0';
    if (size > 9)
        return 0;
    if (size) {
        while ((c = data[used]) != '\0') {
            size_t d = c - '0';
            if (d > 9)
                break;
            used++;
            size = size * 10 + d;
        }
    }
    hdr->size = size;

    /*
    * the length must be followed by a zero byte
    */
    if (data[used++] != '\0')
        return 0;

    return used;
}

ObjectType FileReadBuffer::GetType()
{
    return type_;
}

size_t FileReadBuffer::GetSize()
{
    return len_;
}

bool FileReadBuffer::IsOpened()
{
    return handle_ != INVALID_HANDLE_VALUE;
}

}
