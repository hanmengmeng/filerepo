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

bool IsDirectoryExist( const tstring &dir_path )
{
    tstring regular_path = MakePathRegular(dir_path);
    DWORD dwAttr = ::GetFileAttributes(regular_path.c_str());
    return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

} // namespace filerepo
