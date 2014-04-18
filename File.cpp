#include "stdafx.h"

namespace filerepo
{

File::File()
{
    init_ = false;
    file_ = NULL;
}

File::~File()
{
    Close();
}

bool File::Init( const tstring& name )
{
    if (init_)
        return false;

    DWORD sharing = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    DWORD access = GENERIC_READ | GENERIC_WRITE | DELETE;
    file_ = CreateFile(name.c_str(), access, sharing, NULL,
        OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == file_) {
        return false;
    }

    init_ = true;
    return true;
}

HANDLE File::GetFileHandle() const
{
    return file_;
}

bool File::IsValid() const
{
    if (!init_) {
        return false;
    }
    return INVALID_HANDLE_VALUE != file_;
}

bool File::Read( void* buffer, size_t buffer_len, size_t offset )
{
    if (buffer_len > ULONG_MAX || offset > LONG_MAX) {
        return false;
    }

    DWORD ret = SetFilePointer(file_, static_cast<LONG>(offset),
        NULL, FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == ret) {
        return false;
    }

    DWORD actual;
    DWORD size = static_cast<DWORD>(buffer_len);
    if (!ReadFile(file_, buffer, size, &actual, NULL)) {
        return false;
    }
    return actual == size;
}

bool File::Write( const void* buffer, size_t buffer_len, size_t offset )
{
    if (buffer_len > ULONG_MAX || offset > ULONG_MAX) {
        return false;
    }

    DWORD ret = SetFilePointer(file_, static_cast<LONG>(offset),
        NULL, FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == ret) {
        return false;
    }

    DWORD actual;
    DWORD size = static_cast<DWORD>(buffer_len);
    if (!WriteFile(file_, buffer, size, &actual, NULL)) {
        return false;
    }
    return actual == size;
}

bool File::SetLength( size_t length )
{
    if (length > ULONG_MAX) {
        return false;
    }

    DWORD size = static_cast<DWORD>(length);
    HANDLE file = GetFileHandle();
    if (INVALID_SET_FILE_POINTER == SetFilePointer(file, size, NULL, FILE_BEGIN)) {
        return false;
    }

    return TRUE == SetEndOfFile(file);
}

size_t File::GetLength()
{
    LARGE_INTEGER size;
    HANDLE file = GetFileHandle();
    if (!GetFileSizeEx(file, &size)) {
        return 0;
    }
    if (size.HighPart) {
        return ULONG_MAX;
    }

    return static_cast<size_t>(size.LowPart);
}

void File::Close()
{
    if (!init_) {
        return;
    }
    if (INVALID_HANDLE_VALUE != file_) {
        CloseHandle(file_);
        file_ = INVALID_HANDLE_VALUE;
    }
    init_ = false;
}

}