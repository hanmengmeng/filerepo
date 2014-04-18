#ifndef FILE_ENUMERATOR_H
#define FILE_ENUMERATOR_H

namespace filerepo
{

class FileEnumerator
{
public:
    static FileEnumerator *GetFileEnumerator(const FileRepoTag &tag);

    virtual ~FileEnumerator() {}

    virtual bool IsEnd() = 0;

    virtual bool Begin() = 0;

    virtual bool Next() = 0;

    virtual FileAttr GetFileAttr() = 0;

    virtual bool ExtractFile(const tstring &to_file_path) = 0;

    virtual bool FindFile(const FileAttr &file_attr) = 0;

    // Free memory
    virtual void Clear() = 0;
};

}
#endif
