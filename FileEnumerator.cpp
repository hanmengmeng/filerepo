#include "stdafx.h"

namespace filerepo
{

FileEnumerator * FileEnumerator::Create( const tstring &repo_dir )
{
    return new FileEnumeratorImpl(repo_dir);
}

FileEnumerator * FileEnumerator::Create(const tstring &repo_dir, const FileRepoTag &tag)
{
    return new FileEnumeratorImpl(repo_dir, tag);
}

}