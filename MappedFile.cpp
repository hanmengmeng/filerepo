#include "stdafx.h"

namespace filerepo
{

MappedFile::MappedFile()
{
    init_ = false;
    buffer_ = NULL;
    section_ = NULL;
    view_size_ = 0;
}

MappedFile::~MappedFile()
{
    Close();
}

void* MappedFile::Init( const tstring& name, size_t size )
{
    if (init_ || !File::Init(name)) {
        return NULL;
    }

    buffer_ = NULL;
    init_ = true;
    section_ = CreateFileMapping(GetFileHandle(), NULL, PAGE_READWRITE, 0,
        static_cast<DWORD>(size), NULL);
    if (!section_) {
        return NULL;
    }

    buffer_ = MapViewOfFile(section_, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size);
    view_size_ = size;

    return buffer_;
}

bool MappedFile::Load( const FileBlock* block )
{
    size_t offset = block->offset() + view_size_;
    return Read(block->buffer(), block->size(), offset);
}

bool MappedFile::Store( const FileBlock* block )
{
    size_t offset = block->offset() + view_size_;
    return Write(block->buffer(), block->size(), offset);
}

bool MappedFile::SetLength( size_t length )
{
    if (!File::SetLength(length)){
        return false;
    }
    if (buffer_) {
        UnmapViewOfFile(buffer_);
        CloseHandle(section_);
    }
    section_ = CreateFileMapping(GetFileHandle(), NULL, PAGE_READWRITE, 0,
        static_cast<DWORD>(length), NULL);
    buffer_ = MapViewOfFile(section_, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    return NULL != buffer_;
}

void MappedFile::Close()
{
    if (!init_) {
        return;
    }

    if (buffer_) {
        BOOL ret = UnmapViewOfFile(buffer_);
        buffer_ = NULL;
    }

    if (section_) {
        CloseHandle(section_);
        section_ = NULL;
    }
    init_ = false;
    File::Close();
}

}