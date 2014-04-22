#ifndef FILE_ENUMERATOR_IMPL_H
#define FILE_ENUMERATOR_IMPL_H

namespace filerepo
{

class FileEnumeratorImpl : public FileEnumerator
{
public:
    FileEnumeratorImpl(const RepoPath &repo_dir);
    FileEnumeratorImpl(const RepoPath &repo_dir, const FileRepoTag &repo_tag);
    ~FileEnumeratorImpl();

    // Implement FileEnumerator
    virtual bool IsEnd() OVERRIDE;
    virtual void Begin() OVERRIDE;
    virtual void Next() OVERRIDE;
    virtual bool GetFileAttr(FileAttr *file_attr) OVERRIDE;
    virtual bool GetObjectId(ObjectId *oid) OVERRIDE;
    virtual int ExtractFile(const tstring &to_file_path) OVERRIDE;
    virtual bool FindFile(const FileAttr &file_attr) OVERRIDE;
    virtual void Clear() OVERRIDE;

private:
    QuickIndex *quick_index_;
    ObjectDb odb_;
    EntryIterator *entry_iterator_;
    RepoPath repo_dir_;

    DISALLOW_COPY_AND_ASSIGN(FileEnumeratorImpl);
};

} // namespace filerepo
#endif
