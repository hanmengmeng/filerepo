#include "stdafx.h"

namespace filerepo
{

FileEnumeratorImpl::FileEnumeratorImpl( const RepoPath &repo_dir, const FileRepoTag &repo_tag )
    : odb_(repo_dir.Data())
    , repo_dir_(repo_dir)
    , entry_iterator_(NULL)
{
    quick_index_ = QuickIndex::Open(repo_dir_.GetTaggedIndexFilePath(repo_tag.id), true);
    if (NULL != quick_index_) {
        entry_iterator_ = quick_index_->GetEntryIterator();
    }
}

FileEnumeratorImpl::FileEnumeratorImpl( const RepoPath &repo_dir )
    : odb_(repo_dir.Data())
    , repo_dir_(repo_dir)
    , entry_iterator_(NULL)
{
    quick_index_ = QuickIndex::Open(repo_dir_.GetIndexFilePath(), true);
    if (NULL != quick_index_) {
        entry_iterator_ = quick_index_->GetEntryIterator();
    }
}

FileEnumeratorImpl::~FileEnumeratorImpl()
{
    if (NULL != entry_iterator_) {
        entry_iterator_->Clear();
        entry_iterator_ = NULL;
    }
    if (NULL != quick_index_) {
        quick_index_->Clear();
        quick_index_ = NULL;
    }
}

bool FileEnumeratorImpl::IsEnd()
{
    return entry_iterator_->IsEnd();
}

void FileEnumeratorImpl::Begin()
{
    entry_iterator_->Begin();
}

void FileEnumeratorImpl::Next()
{
    entry_iterator_->Next();
}

bool FileEnumeratorImpl::GetFileAttr(FileAttr *file_attr)
{
    if (IsEnd()) {
        return false;
    }

    Entry entry = entry_iterator_->GetEntry();
    file_attr->stat = entry.st;
    //file_attr->is_dir = entry.flag & ENTRY_FLAG_IS_DIR;

    std::string path = entry_iterator_->GetPath();
    file_attr->path = utf8_to_16(path.c_str());

    return true;
}

int FileEnumeratorImpl::ExtractFile( const tstring &to_file_path )
{
    ObjectId oid;
    if (!GetObjectId(&oid)) {
        return FILE_NOT_FOUND;
    }
    ObjectStream *stream = odb_.OpenReadStream(oid);
    HANDLE handle = OpenFileToWrite(to_file_path);
    if (!IsOpenSucceed(handle)) {
        return OPEN_FILE_FAILED;
    }

    const int buffer_size = 40960;
    char buffer[buffer_size];
    size_t read_len = 0;
    DWORD written_len = 0;
    while ((read_len = stream->Read(buffer, buffer_size)) > 0) {
        if (!WriteFile(handle, buffer, read_len, &written_len, NULL)) {
            stream->Close();
            return WRITE_FILE_FAILED;
        }
    }
    stream->Close();
    return SUCCEED;
}

bool FileEnumeratorImpl::FindFile( const FileAttr &file_attr )
{
    // Find from the first item
    Begin();

    FileAttr fa;
    while (!IsEnd()) {
        GetFileAttr(&fa);
        if (fa.path == file_attr.path && fa.stat == file_attr.stat) { // Matched!
            return true;
        }
        Next();
    }
    return false;
}

void FileEnumeratorImpl::Clear()
{
    delete this;
}

bool FileEnumeratorImpl::GetObjectId(ObjectId *oid)
{
    if (IsEnd()) {
        return false;
    }
    Entry entry = entry_iterator_->GetEntry();
    *oid = entry.oid;
    return true;
}

}
