#ifndef TYPE_DEFINE_H
#define TYPE_DEFINE_H

typedef wchar_t tchar;
typedef std::wstring tstring;
typedef std::wstringstream tstringstream;
typedef struct _stat64 FileStat;

typedef struct __FileAttr
{
    tstring path; // the file's original path
    FileStat stat; // the file's original file stat
    bool is_dir;
}FileAttr;

enum
{
    TAG_NORMAL,
    TAG_NEED_TO_MERGE,
};

struct FileRepoTag
{
    std::string info;
    int type;
    time_t time;
};

#define mode_t unsigned short
typedef __int64 file_off_t;
typedef __time64_t file_time_t;

#define MAX_FILE_PATH (260 + 1)
#define OVERRIDE override

#endif
