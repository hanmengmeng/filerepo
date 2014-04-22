#include "stdafx.h"

namespace filerepo
{

FileRepoImpl::FileRepoImpl( const RepoPath &repo_dir )
    : object_db_(repo_dir)
    , repo_dir_(repo_dir)
    , quick_index_(NULL)
    , inited_(false)
{
}

FileRepoImpl::~FileRepoImpl()
{
    if (NULL != quick_index_) {
        quick_index_->Clear();
    }
}

int FileRepoImpl::Initialize()
{
    RepoVersion repo_ver(repo_dir_);
    if (!IsDirectoryExist(repo_dir_)) {
        MakeDirPath(repo_dir_);
    }
    if (!repo_ver.Exist()) {
        repo_ver.Create();
    }
    version_ = repo_ver.Read();
    // The repository version is too new
    if (repo_ver.Current() < version_) {
        return REPO_VERSION_INCOMPATIBLE;
    }

    std::string manifest_content;
    ReadFileToString(repo_dir_.GetManifestFilePath(), &manifest_content);
    if (!manifest_content.empty()) {
        Json::Reader jv;
        jv.parse(manifest_content, root_);
    }

    if (NULL != quick_index_) {
        quick_index_->Clear();
    }
    quick_index_ = QuickIndex::Open(repo_dir_.GetIndexFilePath(), false);
    assert(NULL != quick_index_);

    inited_ = true;
    return SUCCEED;
}

int FileRepoImpl::AddFile( const tstring &file_path, const FileAttr &file_attr )
{
    if (!IsInited()) {
        return FILE_REPO_NOT_OPENED;
    }
    if (file_path.empty() || file_attr.path.empty()) {
        return FILE_PATH_EMPTY;
    }

    int ret = SUCCEED;
    ObjectId oid;
    if (file_path.empty()) {
        if ((ret = object_db_.AddBlobObject("", 0, &oid)) != SUCCEED) {
            return ret;
        }
    }
    else {
        if ((ret = object_db_.AddBlobObject(file_path, &oid)) != SUCCEED) {
            return ret;
        }
    }
    if (IsExtendObjectId(oid)) {
        EntryIterator *it = quick_index_->GetEntryIterator();
        while (!it->IsEnd()) {
            Entry entry = it->GetEntry();
            if (IsObjectEqual(entry.oid, oid)) {
                if (file_attr.stat == entry.st) {
                    ObjectId temp_oid;
                    CopyObjectId(&oid, &temp_oid);
                    // May be extend id is different
                    CopyObjectId(&entry.oid, &oid);
                    object_db_.RemoveDuplicateObject(temp_oid);
                    break;
                }
            }
            it->Next();
        }
    }
    Entry entry;
    entry.flag = 0;
    entry.oid = oid;
    entry.st = file_attr.stat;
    std::string path = utf16_to_8(file_attr.path.c_str());
    if (!quick_index_->AddEntry(path, entry)) {
        return ADD_FILE_ENTRY_FAILED;
    }
    return SUCCEED;
}

int FileRepoImpl::AddDir( const FileAttr &file_attr )
{
    return AddFile(_T(""), file_attr);
}

int FileRepoImpl::RemoveFile( const FileAttr &file_attr )
{
    if (!IsInited()) {
        return FILE_REPO_NOT_OPENED;
    }
    if (file_attr.path.empty()) {
        return FILE_PATH_EMPTY;
    }
    std::string path = utf16_to_8(file_attr.path.c_str());
    if (!quick_index_->RemoveEntry(path)) {
        return REMOVE_FILE_ENTRY_FAILED;
    }
    return SUCCEED;
}

int FileRepoImpl::CreateTag( const std::string &info )
{
    if (!IsInited()) {
        return FILE_REPO_NOT_OPENED;
    }

    // Save new index entry or deleted entry to disk
    if (!quick_index_->Save()) {
        return SAVE_INDEX_FAILED;
    }

    // Mark backup times
    if (root_["backup_times"].isNull()) {
        root_["backup_times"] = 1;
    }
    else {
        root_["backup_times"] = root_["backup_times"].asInt() + 1;
    }

    tstring tag_file_path = repo_dir_.GetTaggedIndexFilePath(root_["backup_times"].asInt());
    MakePathToFile(tag_file_path);
    if (!quick_index_->CopyTo(tag_file_path)) {
        return COPY_INDEX_FAILED;
    }

    Json::Value backup;
    backup["id"] = root_["backup_times"];
    backup["info"] = info;
    backup["time"] = time(NULL);
    root_["backups"].append(backup);

    // Create tag in manifest file
    Json::StyledWriter writer;
    if (!WriteStringToFile(repo_dir_.GetManifestFilePath(), writer.write(root_))) {
        return CREATE_TAG_FAILED;
    }
    return SUCCEED;
}

int FileRepoImpl::GetAllTags( std::vector<FileRepoTag> *tag_list )
{
    if (!IsInited()) {
        return FILE_REPO_NOT_OPENED;
    }
    if (root_["backups"].isNull()) {
        return BACKUP_TAGS_EMPTY;
    }

    Json::Value backups = root_["backups"];
    for (size_t i = 0; i < backups.size(); i++) {
        Json::Value item = backups[i];
        if (!item["deleted"].isNull()) {
            FileRepoTag repo_tag;
            repo_tag.id = item["id"].asInt();
            repo_tag.info = item["info"].asString();
            repo_tag.time = item["time"].asInt();

            tag_list->push_back(repo_tag);
        }
    }
    return SUCCEED;
}

int FileRepoImpl::RemoveTag( const FileRepoTag &tag )
{
    if (!IsInited()) {
        return FILE_REPO_NOT_OPENED;
    }
    if (root_["backups"].isNull()) {
        return BACKUP_TAGS_EMPTY;
    }
    Json::Value backups = root_["backups"];
    for (size_t i = 0; i < backups.size(); i++) {
        Json::Value item = backups[i];
        if (!item["deleted"].isNull()) {
            if (item["id"].asInt() == tag.id
                && item["info"].asString() == tag.info
                && item["time"].asInt() == tag.time) {
                item["deleted"] = 1;
                return SUCCEED;
            }
        }
    }
    return BACKUP_TAG_NOT_EXIST;
}

int FileRepoImpl::GetVersion()
{
    return version_;
}

void FileRepoImpl::Clear()
{
    delete this;
}

bool FileRepoImpl::IsInited()
{
    return inited_;
}

} // namespace filerepo
