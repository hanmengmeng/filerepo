#ifndef UTILITIES_H
#define UTILITIES_H

namespace filerepo
{

#define CHECK_ALLOC(ptr) if (ptr == NULL) { return false; }

std::wstring utf8_to_16(const char *src);
std::string utf16_to_8(const wchar_t *src);

}

#endif