#include "stdafx.h"

namespace filerepo
{

std::wstring utf8_to_16( const char *src )
{
    int len = ::MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    if (len == 0) return L"";

    std::vector<wchar_t> ret(len);
    ::MultiByteToWideChar(CP_ACP, 0, src, -1, &ret[0], len);

    return &ret[0];
}

std::string utf16_to_8( const wchar_t *src )
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
    if (len == 0) return "";

    std::vector<char> utf8(len);
    ::WideCharToMultiByte(CP_ACP, 0, src, -1, &utf8[0], len, NULL, NULL);

    return &utf8[0];
}

bool operator==( const FileStat &a, const FileStat &b )
{
    return a.st_atime == b.st_atime
        && a.st_ctime == b.st_ctime
        && a.st_dev == b.st_dev
        && a.st_gid == b.st_gid
        && a.st_ino == b.st_ino
        && a.st_mode == b.st_mode
        && a.st_mtime == b.st_mtime
        && a.st_nlink == b.st_nlink
        && a.st_rdev == b.st_rdev
        && a.st_size == b.st_size
        && a.st_uid == b.st_uid;
}

}