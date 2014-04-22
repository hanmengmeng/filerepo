#include "stdafx.h"

namespace filerepo
{
#define DEFAULT_PATH_SPLITER _T('\\')

bool MakeTempFile( const tstring &filename, tstring *path_out )
{
    const size_t TEMP_BUF_SIZE = 7;
    tchar temp_name[TEMP_BUF_SIZE] = {0};
    wcsncpy_s(temp_name, TEMP_BUF_SIZE, _T("XXXXXX"), TEMP_BUF_SIZE);
    if (_wmktemp_s(temp_name) != 0) {
        return false;
    }
    *path_out = filename;
    *path_out += temp_name;
    return true;
}

bool RenameFile( const tstring &from, const tstring &to )
{
    return MoveFileEx(
        from.c_str(),
        to.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) ? true : false;
}

tstring AppendPath( const tstring &to, const tstring &from )
{
    tstring ret = to;
    if (!to.empty()) {
        if (ret.at(to.size() - 1) != _T('/') && ret.at(to.size() - 1) != DEFAULT_PATH_SPLITER) {
            ret += DEFAULT_PATH_SPLITER;
        }
    }
    ret += from;
    return MakePathRegular(ret);
}

tstring filerepo::MakePathRegular( const tstring &path )
{
    tstring ret = path;
    size_t count = ret.length();
    for (size_t i = 0; i < count; i++) {
        if (_T('/') == ret[i]) {
            ret[i] = DEFAULT_PATH_SPLITER;
        }
    }
    return ret;
}

bool IsFileExist( const tstring &path )
{
    tstring regular_path = MakePathRegular(path);
    DWORD dwAttributes = ::GetFileAttributes(regular_path.c_str());
    return (dwAttributes != INVALID_FILE_ATTRIBUTES) && (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool MakePathToFile( const tstring &file_path )
{
    tstring regularPath = MakePathRegular(file_path);
    for (size_t i = 0; i < regularPath.length(); i++) {
        if (DEFAULT_PATH_SPLITER == regularPath.at(i)) {
            tstring p = regularPath.substr(0, i);
            if (!IsDirectoryExist(p)) {
                if (!::CreateDirectory(p.c_str(), NULL)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool MakeDirPath( const tstring &dir_path )
{
    if (!MakePathToFile(dir_path)) {
        return false;
    }
    tstring regularPath = MakePathRegular(dir_path);
    return ::CreateDirectory(regularPath.c_str(), NULL) == TRUE;
}

bool IsDirectoryExist( const tstring &dir_path )
{
    tstring regular_path = MakePathRegular(dir_path);
    DWORD dwAttr = ::GetFileAttributes(regular_path.c_str());
    return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

int64 GetFileSize( const tstring &file_path )
{
    int64 file_size = 0;
    HANDLE handle = ::CreateFile(file_path.c_str(),
        FILE_READ_EA,
        FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        0,
        0);
    if (INVALID_HANDLE_VALUE != handle) {
        DWORD fs = 0;
        DWORD fsh = 0;
        fs = ::GetFileSize(handle, &fsh);
        CloseHandle(handle);
        file_size = fsh;
        file_size <<= 32;
        file_size |= fs;
    }
    return file_size;
}

HANDLE OpenFileToRead( const tstring &file_path )
{
    HANDLE handle = ::CreateFile(file_path.c_str(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    return handle;
}

HANDLE OpenFileToWrite( const tstring &file_path )
{
    HANDLE handle = ::CreateFile(file_path.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL);
    return handle;
}

bool IsOpenSucceed( HANDLE handle )
{
    return INVALID_HANDLE_VALUE != handle;
}

bool CopyFile( const tstring &src_path, const tstring &target_path )
{
    return ::CopyFile(src_path.c_str(), target_path.c_str(), false) == TRUE;
}

bool ReadFileToString( const tstring &file_path, std::string *content )
{
    if (!IsFileExist(file_path)) {
        return false;
    }
    size_t file_size = static_cast<size_t>(GetFileSize(file_path));

    HANDLE handle = OpenFileToRead(file_path);
    if (!IsOpenSucceed(handle)) {
        return false;
    }

    char *buf = new char[file_size+1];
    DWORD read_len = 0;
    if (::ReadFile(handle, buf, file_size, &read_len, NULL)) {
        buf[file_size] = '\0';
        *content = buf;
    }
    ::CloseHandle(handle);
    return true;
}

bool WriteStringToFile( const tstring &file_path, const std::string &content )
{
    HANDLE handle = OpenFileToWrite(file_path);
    if (!IsOpenSucceed(handle)) {
        return false;
    }

    bool ret = true;
    DWORD written_len = 0;
    if (!::WriteFile(handle, content.data(), content.length(), &written_len, NULL)) {
        ret = false;
    }
    ::CloseHandle(handle);
    return ret;
}

} // namespace filerepo
