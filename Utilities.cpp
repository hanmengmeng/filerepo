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

}