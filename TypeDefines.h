#ifndef TYPE_DEFINE_H
#define TYPE_DEFINE_H

#include <string>
#include <vector>

typedef wchar_t tchar;
typedef std::wstring tstring;
typedef std::wstringstream tstringstream;
typedef struct _stat64 FileStat;

enum
{
    TAG_NORMAL,
    TAG_NEED_TO_MERGE,
};

struct FileRepoTag
{
    int id;
    std::string info;
    time_t time;
};

typedef struct __FileAttr
{
    tstring path; // the file's original path
    FileStat stat; // the file's original file stat
}FileAttr;

#define mode_t unsigned short
typedef __int64 file_off_t;
typedef __time64_t file_time_t;

#define MAX_FILE_PATH (260 + 1)
#define OVERRIDE override

typedef long long int64;

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);               \
    void operator=(const TypeName&)

#endif
