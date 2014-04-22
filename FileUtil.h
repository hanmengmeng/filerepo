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

bool MakeDirPath(const tstring &dir_path);

int64 GetFileSize(const tstring &file_path);

HANDLE OpenFileToRead(const tstring &file_path);

HANDLE OpenFileToWrite(const tstring &file_path);

bool IsOpenSucceed(HANDLE handle);

bool CopyFile(const tstring &src_path, const tstring &target_path);

bool ReadFileToString(const tstring &file_path, std::string *content);

bool WriteStringToFile(const tstring &file_path, const std::string &content);

}

#endif