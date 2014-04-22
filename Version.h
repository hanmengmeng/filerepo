#ifndef VERSION_H
#define VERSION_H

namespace filerepo
{
const int INVALID_VERSION = 0;
const int FILE_REPO_VERSION = 1;

class RepoVersion
{
public:
    RepoVersion(const RepoPath &path);

    // version of lib
    int Current();

    // version of file repo on disk
    int Read();

    // write version of lib to disk
    bool Create();

    bool Exist();

private:
    RepoPath repo_path_;
    int version_;
};

}
#endif
