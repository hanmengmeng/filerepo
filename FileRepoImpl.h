#ifndef FILE_REPO_IMPL_H
#define FILE_REPO_IMPL_H

namespace filerepo
{
class FileRepoImpl : public FileRepo
{
public:
    FileRepoImpl(const tstring &repo_dir);

    // Implement FileRepo
    virtual bool IsCreated() OVERRIDE;
    virtual bool AddFile(const tstring &file_path, const FileAttr &file_attr) OVERRIDE;
    virtual bool AddDir(const FileAttr &file_attr) OVERRIDE;
    virtual bool RemoveFile(const FileAttr &file_attr) OVERRIDE;
    virtual bool CreateTag(const std::string &info, int type = 0) OVERRIDE;
    virtual bool GetAllTags(std::vector<FileRepoTag> *tag_list) OVERRIDE;
    virtual FileEnumerator *CreateEnumerator() OVERRIDE;
    virtual FileEnumerator *CreateEnumerator(const FileRepoTag &tag) OVERRIDE;
    virtual int GetVersion() OVERRIDE;
    virtual void Clear() OVERRIDE;

private:
    ObjectDb object_db_;
};
}

#endif