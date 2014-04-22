#include "stdafx.h"

namespace filerepo
{

QuickIndex * QuickIndex::Open(const tstring &index_path, bool read_only)
{
    QuickIndexImpl *impl = new QuickIndexImpl();
    if (impl->Init(index_path, read_only)) {
        return impl;
    }
    delete impl;
    return NULL;
}

bool QuickIndex::DeleteIndexFromDisk( const tstring &index_path )
{
    QuickIndexImpl *impl = new QuickIndexImpl();
    if (!impl->Init(index_path, true)) {
        return false;
    }
    impl->DeleteFiles();
    delete impl;
    return true;
}

}