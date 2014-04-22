#include "stdafx.h"

namespace filerepo
{

FileRepo * FileRepo::Create( const tstring &repo_dir )
{
    FileRepoImpl *impl = new FileRepoImpl(repo_dir);
    if (IS_SUCCEED(impl->Initialize())) {
        return impl;
    }
    delete impl;
    return NULL;
}

} // namespace filerepo
