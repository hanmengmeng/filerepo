#ifndef FILE_REPO_H
#define FILE_REPO_H

namespace filerepo
{

// Backup steps:
//
// std::vector<FileAttr> file_list_need_to_add = ...;
// std::vector<FileAttr> file_list_need_to_remove;
//
// FileRepo *repo = FileRepo::Create(_T("path to repo"));
// FileEnumerator *enumer = repo.CreateEnumerator();
// while (!enumer.IsEnd()) {
//     FileAttr attr = enumer.GetFileAttr();
//     if (attr in file_list_need_to_backup) {
//         file_list_need_to_add.erase(attr); // already add
//     }
//     else {
//         file_list_need_to_remove.push_back(attr);
//     }
// }
// for (size_t i = 0; i < file_list_need_to_add.size(); i++) {
//     if (file_list_need_to_add.at(i).is_dir) {
//         repo.AddFile(path_on_local, file_list_need_to_add.at(i));
//     }
//     else {
//         repo.AddDir(path_on_local, file_list_need_to_add.at(i));
//     }
// }
// for (size_t i = 0; i < file_list_need_to_remove.size(); i++) {
//     repo.RemoveFile(file_list_need_to_remove.at(i));
// }
// repo.CreateTag("");
// repo.Clear();
//
/////////////////////////////////////////////////////////////////////
//
// Restore steps:
//
// FileRepo *repo = FileRepo::Create(_T("path to repo"));
// std::vector<FileRepoTag> tags;
// repo.GetAllTags(&tags);
// FileEnumerator *enumer = repo.CreateEnumerator();
// while (!enumer.IsEnd()) {
//     FileAttr attr = enumer.GetFileAttr();
//     tstring temp_file_path = MakeTempFilePath();
//     enumer.ExtractFile(temp_file_path);
//     // Do something...
//
// }

class FileRepo
{
public:
    virtual ~FileRepo() {}

    static FileRepo *Create(const tstring &repo_dir);

    virtual bool IsCreated() = 0;

    virtual bool AddFile(const tstring &file_path, const FileAttr &file_attr) = 0;

    virtual bool AddDir(const FileAttr &file_attr) = 0;

    virtual bool RemoveFile(const FileAttr &file_attr) = 0;

    virtual bool CreateTag(const std::string &info, int type = 0) = 0;

    virtual bool GetAllTags(std::vector<FileRepoTag> *tag_list) = 0;

    virtual FileEnumerator *CreateEnumerator() = 0;

    virtual FileEnumerator *CreateEnumerator(const FileRepoTag &tag) = 0;

    virtual int GetVersion() = 0;

    // Free memory
    virtual void Clear() = 0;
};

} // namespace filerepo
#endif
