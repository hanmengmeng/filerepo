#ifndef FILE_ENUMERATOR_H
#define FILE_ENUMERATOR_H

#include "ObjectId.h"
#include "TypeDefines.h"

namespace filerepo
{

class FileEnumerator
{
public:
    static FileEnumerator *Create(const tstring &repo_dir);

    static FileEnumerator *Create(const tstring &repo_dir, const FileRepoTag &tag);

    virtual ~FileEnumerator() {}

    // Check if the last item has been read
    virtual bool IsEnd() = 0;

    // Move to the first item
    virtual void Begin() = 0;

    // Move to next item
    virtual void Next() = 0;

    // Get current item's FileAttr
    virtual bool GetFileAttr(FileAttr *file_attr) = 0;

    // Get current item's object id
    virtual bool GetObjectId(ObjectId *oid) = 0;

    // Extract current item to <to_file_path> file path
    virtual int ExtractFile(const tstring &to_file_path) = 0;

    // Find item that match <file_attr>
    // if found, move to the item
    virtual bool FindFile(const FileAttr &file_attr) = 0;

    // Free memory
    virtual void Clear() = 0;
};

} // namespace FileEnumerator
#endif
