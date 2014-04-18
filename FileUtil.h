#ifndef FILE_UTIL_H
#define FILE_UTIL_H

namespace filerepo
{

bool MakeTempFile(const tstring &filename, tstring *path_out);

bool RenameFile(const tstring &from, const tstring &to);

tstring AppendPath(const tstring &to, const tstring &from);

tstring MakePathRegular( const tstring &path );

bool IsFileExist(const tstring &path);

bool IsDirectoryExist(const tstring &dir_path);

bool MakePathToFile(const tstring &file_path);

}

#endif