#include "stdafx.h"

namespace filerepo
{

#define INDEX_MAGIC_VALUE 1158160624
#define INDEX_VERSION_VALUE 1

QuickIndexImpl::QuickIndexImpl()
{
    header_ = NULL;
    next_large_index_ = NULL;
    next_index_ = NULL;
    index_file_modify_ = false;
}

QuickIndexImpl::~QuickIndexImpl()
{
    if (NULL != next_index_) {
        delete next_index_;
    }
    if (NULL != next_large_index_) {
        delete next_large_index_;
    }
    index_file_.Close();
    if (IsFileExist(index_file_path_temp_)) {
        ::DeleteFile(index_file_path_temp_.c_str());
        int n = ::GetLastError();
        n = ::GetLastError();
    }
}

bool QuickIndexImpl::Init( const tstring &path, size_t file_number, size_t entry_size, bool read_only )
{
    if (0 == entry_size) {
        return false;
    }
    bool exist = false;
    index_file_path_ = path;//MakePathRegular(path);

    // Write to a temp file
    index_file_path_temp_ = index_file_path_ + _T(".temp");

    if (!IsFileExist(index_file_path_)) {
        if (read_only) {
            return false;
        }
        if (IsFileExist(index_file_path_temp_.c_str())) {
            ::DeleteFile(index_file_path_temp_.c_str());
        }
        DWORD access = GENERIC_READ | GENERIC_WRITE | DELETE;
        HANDLE file = CreateFile(index_file_path_temp_.c_str(), access, NULL, NULL,
            CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
        if (INVALID_HANDLE_VALUE == file) {
            return false;
        }
        CloseHandle(file);
        header_ = (QuickIndexHeader*)index_file_.Init(index_file_path_temp_, sizeof(QuickIndexHeader));
        index_file_modify_ = true;
    }
    else {
        exist = true;
        if (GetFileSize(index_file_path_.c_str()) > 0) {
            header_ = (QuickIndexHeader*)index_file_.Init(index_file_path_, 0);
        }
        else {
            header_ = (QuickIndexHeader*)index_file_.Init(index_file_path_, sizeof(QuickIndexHeader));
        }
    }

    if (NULL == header_) {
        return false;
    }

    if (!exist) {
        if (!GenerateHeader(file_number, entry_size)) {
            return false;
        }

        if (!GrowIndexFile()) {
            return false;
        }
    }

    if (!IsHeaderValid()) {
        return false;
    }
    return true;
}

bool QuickIndexImpl::Init( const tstring &path, bool read_only )
{
    return Init(path, 0, sizeof(EntryShort), read_only);
}

// Quick index file structure is as follow
// index 0-1-3... store short entries
// index 2-4... store large entries
// if index_0 is full, create index_1 to continue store
// if new entry has long path (> 256), create index_2 to store it
// only index_0 has two branch
/*             index_0                   */
/*             /  \                      */
/*       index_1  index_2 (large)        */
/*          /       \                    */
/*       index_3    index_4 (large)      */
bool QuickIndexImpl::AddEntry( const std::string &path, const Entry &entry )
{
    if (NULL == header_) {
        return false;
    }

    if (!PrepareIndexFile()) {
        return false;
    }

    if (UseLargeEntryToStore(path) && !IsLargeEntry()) {
        assert(0 == header_->current_file);
        if (0 != header_->current_file) {
            return false;
        }
        if (header_->next_large_file == 0){
            header_->next_large_file = header_->current_file + 2;
        }
        QuickIndex *qi = NextLarge();
        if (NULL == qi){
            return false;
        }
        return qi->AddEntry(path, entry);
    }

    // Find entry in current index file,
    // if we find the match entry,
    // use new entry value to replace it
    EntryAddr found_addr;
    if (FindEntryInCurrentIndex(path, found_addr)) {
        UpdateEntry(found_addr, path, entry);
        return true;
    }

    // If current index file is full,
    // use next index to store the entry
    if (IsFileFull()) {
        QuickIndex *qi = NULL;
        if (!IsLargeEntry()) {
            if (header_->next_file == 0) {
                header_->next_file =
                    (0 == header_->current_file) ? 1 : header_->current_file + 2;
            }
            qi = Next();
        }
        else {
            if (header_->next_large_file == 0) {
                header_->next_large_file = header_->current_file + 2;
            }
            qi = NextLarge();
        }

        if (NULL == qi) {
            return false;
        }
        return qi->AddEntry(path, entry);
    }

    // If current index file not full, but not empty entry,
    // just grow the index file
    if (header_->num_entries == header_->num_entries_capacity) {
        if (!GrowIndexFile()) {
            return false;
        }
    }

    size_t entry_pos = GetHash(path);
    EntryAddr addr = GetEntryAddr(entry_pos);
    EntryAddr entry_addr;
    if (IsAddrValid(addr) && IsAddrUsed(addr)) {
        // Handle hash conflict situation
        entry_addr = GetEmptyEntryAddr();
        if (!IsAddrValid(entry_addr)) {
            return false;
        }
        if (!UpdateLastConflictEntry(addr, entry_addr)) {
            return false;
        }
    }
    else {
        entry_addr = GetEmptyEntryAddr();
        if (!IsAddrValid(entry_addr)) {
            return false;
        }
        header_->entries[entry_pos] = entry_addr;
    }

    UpdateEntry(entry_addr, path, entry);
    header_->num_entries++;
    return true;
}

bool QuickIndexImpl::FindEntry( const std::string &path, Entry *entry )
{
    if (NULL == header_ || NULL == entry) {
        return false;
    }

    if (UseLargeEntryToStore(path) && !IsLargeEntry()) {
        QuickIndex *qi = NextLarge();
        if (NULL == qi){
            return false;
        }
        return qi->FindEntry(path, entry);
    }

    bool found = false;
    size_t entry_pos = GetHash(path);
    EntryAddr addr = GetEntryAddr(entry_pos);
    while (IsAddrValid(addr) && IsAddrUsed(addr))
    {
        if (IsLargeEntry()) {
            EntryLarge *el = (EntryLarge*)GetEntryByAddr(addr);
            if (path == el->path) {
                memcpy(entry, &el->content, sizeof(Entry));
                found = true;
                break;
            }
            else {
                addr = el->next;
            }
        }
        else {
            EntryShort *es = (EntryShort*)GetEntryByAddr(addr);
            if (path == es->path) {
                memcpy(entry, &es->content, sizeof(Entry));
                found = true;
                break;
            }
            else {
                addr = es->next;
            }
        }
    }

    if (!found) {
        if (IsLargeEntry()) {
            QuickIndex *index = NextLarge();
            if (NULL != index) {
                found = index->FindEntry(path, entry);
            }
        }
        else {
            QuickIndex *index = Next();
            if (NULL != index) {
                found = index->FindEntry(path, entry);
            }
        }
    }

    return found;
}

size_t QuickIndexImpl::GetEntriesCount()
{
    size_t total_count = header_->num_entries;

    QuickIndexImpl *next = this;
    while ((next = next->Next()) != NULL) {
        total_count += next->header_->num_entries;
    }

    next = this;
    while ((next = next->NextLarge()) != NULL) {
        total_count += next->header_->num_entries;
    }
    return total_count;
}

bool QuickIndexImpl::GenerateHeader(size_t i, size_t entry_size)
{
    if (NULL == header_) {
        return false;
    }
    header_->magic = INDEX_MAGIC_VALUE;
    header_->version = INDEX_VERSION_VALUE;
    header_->entry_size = entry_size;
    header_->num_entries = 0;
    header_->num_entries_capacity = 0;
    header_->current_file = i;
    header_->next_file = 0;
    header_->next_large_file = 0;
    //header_->empty_entry_addr = GetEmptyEntryAddr();
    memset(header_->entries, 0, sizeof(header_->entries));
    memset(header_->entries_alloc_map, 0, sizeof(header_->entries_alloc_map));
    return true;
}

bool QuickIndexImpl::IsFileExist(const tstring &path)
{
    DWORD dwAttributes = ::GetFileAttributes(path.c_str());
    return (dwAttributes != INVALID_FILE_ATTRIBUTES) && (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool QuickIndexImpl::IsHeaderValid()
{
    return NULL != header_ && INDEX_MAGIC_VALUE == header_->magic && INDEX_VERSION_VALUE == header_->version;
}

bool QuickIndexImpl::GrowIndexFile()
{
    if (kMaxEntriesCount == header_->num_entries_capacity){
        return false;
    }
    if (header_->num_entries_capacity == 0) {
        if (IsLargeEntry()) {
            header_->num_entries_capacity = kInitLargeEntriesCount;
        }
        else {
            header_->num_entries_capacity = kInitEntriesCount;
        }
    }
    else {
        header_->num_entries_capacity *= 2;
        if (header_->num_entries_capacity > kMaxEntriesCount) {
            header_->num_entries_capacity = kMaxEntriesCount;
        }
    }
    if (index_file_.SetLength(sizeof(QuickIndexHeader) + GetEntrySize() * header_->num_entries_capacity)) {
        header_ = (QuickIndexHeader*)index_file_.buffer();
        //header_->empty_entry_addr = GetEmptyEntryAddr();
        return true;
    }
    return false;
}

bool QuickIndexImpl::IsFileFull()
{
    return header_->num_entries == header_->num_entries_capacity
        && header_->num_entries_capacity == kMaxEntriesCount;
}

size_t QuickIndexImpl::GetEntrySize() const
{
    return header_->entry_size;
}

void QuickIndexImpl::Clear()
{
    delete this;
}

QuickIndexImpl * QuickIndexImpl::Next()
{
    if (IsLargeEntry()) {
        return NULL;
    }
    if (NULL != next_index_) {
        return next_index_;
    }
    if (0 == header_->next_file) {
        return NULL;
    }
    tstring path = GenerateNextIndexFileName(index_file_path_, header_->next_file);
    if (path.empty()) {
        return NULL;
    }
    next_index_ = new QuickIndexImpl();
    if (!next_index_->Init(path, header_->next_file, sizeof(EntryShort), false)) {
        delete next_index_;
        next_index_ = NULL;
    }

    return next_index_;
}

QuickIndexImpl * QuickIndexImpl::NextLarge()
{
    if (0 != header_->current_file && !IsLargeEntry()) {
        return NULL;
    }
    if (NULL != next_large_index_) {
        return next_large_index_;
    }
    if (0 == header_->next_large_file) {
        return NULL;
    }
    tstring path = GenerateNextIndexFileName(index_file_path_, header_->next_large_file);
    if (path.empty()) {
        return NULL;
    }
    next_large_index_ = new QuickIndexImpl();
    if (!next_large_index_->Init(path, header_->next_large_file, sizeof(EntryLarge), false)) {
        delete next_large_index_;
        next_large_index_ = NULL;
    }

    return next_large_index_;
}

EntryAddr QuickIndexImpl::GetEntryAddr( size_t pos )
{
    if (pos >= kMaxEntriesCount) {
        pos = 0;
    }
    return header_->entries[pos];
}

bool QuickIndexImpl::IsAddrUsed( EntryAddr addr )
{
    if (!IsAddrValid(addr)) {
        return false;
    }
    //return addr & kEntryUseMask;
    EntryAddr pos = addr - 1;
    short addr_bit = header_->entries_alloc_map[pos / 8] & (kEntryUseMask << (pos % 8));
    return addr_bit != 0;
}

void * QuickIndexImpl::GetEntryByAddr( EntryAddr addr )
{
    size_t pos = addr - 1;
    return (char*)header_ + sizeof(QuickIndexHeader) + GetEntrySize() * pos;
}

size_t QuickIndexImpl::GetHash( const std::string &path )
{
    size_t hash = FastHash(path);
    hash = hash & kEntryHashMask;
    if (hash >= kMaxEntriesCount) {
        hash = 0;
    }
    return hash;
}

bool QuickIndexImpl::UpdateLastConflictEntry( EntryAddr addr, EntryAddr next_addr )
{
    size_t count = 0;
    EntryAddr target = addr;
    do {
        count++;
        EntryShort *es = (EntryShort*)GetEntryByAddr(target);
        unsigned short next = es->next;

        if (0 == next) {
            es->next = next_addr;
            return true;
        }
        else {
            target = next;
        }
    } while (count < kMaxEntriesCount);
    return false;
}

EntryAddr QuickIndexImpl::GetEmptyEntryAddr()
{
    if (header_->num_entries >= header_->num_entries_capacity) {
        return 0;
    }
    EntryAddr addr = header_->num_entries + 1; // Entry addr start from 1
    while (IsAddrValid(addr) && IsAddrUsed(addr)) {
        addr++;
    }

    if (IsAddrValid(addr) && !IsAddrUsed(addr)) {
        return addr;
    }

    EntryAddr last = addr;
    addr = 1;
    while (IsAddrValid(addr) && IsAddrUsed(addr) && addr < last) {
        addr++;
    }
    if (IsAddrValid(addr) && !IsAddrUsed(addr)) {
        return addr;
    }
    return 0; // Empty entry not found
}

size_t QuickIndexImpl::GetFileSize( LPCTSTR file_path )
{
    size_t file_size = 0;
    HANDLE handle = ::CreateFile(file_path, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (INVALID_HANDLE_VALUE != handle) {
        file_size = ::GetFileSize(handle, NULL);
        CloseHandle(handle);
    }
    return file_size;
}

bool QuickIndexImpl::IsLargeEntry()
{
    return GetEntrySize() == sizeof(EntryLarge);
}

bool QuickIndexImpl::DeleteFiles()
{
    QuickIndexImpl *next = Next();
    if (NULL != next) {
        next->DeleteFiles();
    }
    QuickIndexImpl *next_large = NextLarge();
    if (NULL != next_large) {
        next_large->DeleteFiles();
    }

    index_file_.Close();
    ::DeleteFile(index_file_path_.c_str());
    if (IsFileExist(index_file_path_temp_.c_str())) {
        ::DeleteFile(index_file_path_temp_.c_str());
    }
    return true;
}

void QuickIndexImpl::UpdateEntriesAllocMap( EntryAddr addr, bool is_used )
{
    size_t pos = addr - 1;

    size_t map_pos = pos / 8;
    size_t map_offset = pos % 8;
    if (is_used) {
        header_->entries_alloc_map[map_pos] |= (kEntryUseMask << map_offset);
    }
    else {
        header_->entries_alloc_map[map_pos] &= ~(kEntryUseMask << map_offset);
    }
}

bool QuickIndexImpl::IsAddrValid( EntryAddr addr )
{
    // Entry addr start from 1
    return addr > 0 && addr <= header_->num_entries_capacity;
}

bool QuickIndexImpl::PrepareIndexFile()
{
    if (index_file_modify_) {
        return index_file_modify_;
    }
    index_file_.Close();
    header_ = NULL;
    if (!CopyFile(index_file_path_.c_str(), index_file_path_temp_.c_str())) {
        return false;
    }
    if (GetFileSize(index_file_path_temp_.c_str()) > 0) {
        header_ = (QuickIndexHeader*)index_file_.Init(index_file_path_temp_, 0);
    }
    else {
        header_ = (QuickIndexHeader*)index_file_.Init(index_file_path_temp_, sizeof(QuickIndexHeader));
    }
    if (NULL != header_) {
        index_file_modify_ = true;
        return true;
    }
    return false;
}

bool QuickIndexImpl::RemoveEntry( const std::string &path )
{
    if (NULL == header_) {
        return false;
    }

    if (UseLargeEntryToStore(path) && !IsLargeEntry()) {
        QuickIndex *qi = NextLarge();
        if (NULL == qi){
            return false;
        }
        return qi->RemoveEntry(path);
    }

    EntryAddr found_addr;
    if (FindEntryInCurrentIndex(path, found_addr)) {
        UpdateEntriesAllocMap(found_addr, false);
        header_->num_entries--;
        return true;
    }

    if (IsLargeEntry()) {
        QuickIndex *qi = NextLarge();
        if (NULL == qi){
            return false;
        }
        return qi->RemoveEntry(path);
    }
    else {
        QuickIndex *qi = Next();
        if (NULL == qi){
            return false;
        }
        return qi->RemoveEntry(path);
    }
    return false;
}

EntryIterator *QuickIndexImpl::GetEntryIterator()
{
    return new EntryIteratorImpl(this);
}

void QuickIndexImpl::UpdateEntry( EntryAddr addr, const std::string &path, const Entry &entry )
{
    if (!IsLargeEntry()) {
        EntryShort *es = (EntryShort*)GetEntryByAddr(addr);
        memcpy(&es->content, &entry, sizeof(Entry));
        if (!IsAddrValid(es->next)) {
            es->next = 0;
        }
        size_t len = min(path.length(), kMaxShortPath - 1);
        memcpy(es->path, path.c_str(), len);
        es->path[len] = '\0';
    }
    else {
        EntryLarge *el = (EntryLarge*)GetEntryByAddr(addr);
        memcpy(&el->content, &entry, sizeof(Entry));
        if (!IsAddrValid(el->next)) {
            el->next = 0;
        }
        size_t len = min(path.length(), kMaxLargePath - 1);
        memcpy(el->path, path.c_str(), len);
        el->path[len] = '\0';
    }
    UpdateEntriesAllocMap(addr, true);
}

tstring QuickIndexImpl::GenerateNextIndexFileName(const tstring &prefix,  size_t index_number )
{
    tstring path = prefix;
    if (0 == header_->current_file) {
        path += _T("_");
        tstringstream ss;
        ss << index_number;
        path += ss.str();
    }
    else {
        tstringstream ss;
        ss << index_number;

        size_t pos = path.find_last_of(_T("_"));
        if (pos < path.length() - 1) {
            path.replace(pos + 1, path.length() - 1, ss.str());
        }
        else {
            path += _T("_");
            path += ss.str();
        }
    }
    return path;
}

bool QuickIndexImpl::FindEntryInCurrentIndex( const std::string &path, EntryAddr &found_addr )
{
    size_t entry_pos = GetHash(path);
    EntryAddr addr = GetEntryAddr(entry_pos);
    size_t count = 0;
    do
    {
        count++;
        if (IsAddrValid(addr) && IsAddrUsed(addr)) {
            if (!IsLargeEntry()) {
                EntryShort *es = (EntryShort*)GetEntryByAddr(addr);
                if (strncmp(path.c_str(), es->path, kMaxShortPath) == 0) {
                    found_addr = addr;
                    return true;
                }
                if (IsAddrValid(es->next)) {
                    addr = es->next;
                    continue;
                }
            }
            else {
                EntryLarge *el = (EntryLarge*)GetEntryByAddr(addr);
                if (strncmp(path.c_str(), el->path, kMaxLargePath) == 0) {
                    found_addr = addr;
                    return true;
                }
                if (IsAddrValid(el->next)) {
                    addr = el->next;
                    continue;
                }
            }
        }
        break; // Not found
    } while (count < kMaxEntriesCount);
    return false;
}

bool QuickIndexImpl::UseLargeEntryToStore( const std::string &path )
{
    return path.length() >= kMaxShortPath;
}

std::string QuickIndexImpl::GetEntryPath( const Entry *entry )
{
    return FormatObjectId(entry->oid);
}

bool QuickIndexImpl::Save()
{
    bool ret = true;
    if (index_file_modify_) {
        index_file_modify_ = false;
        ret = ::CopyFile(index_file_path_temp_.c_str(), index_file_path_.c_str(), FALSE) == TRUE;
#if 0
        if (ret) {
            index_file_.Close();
            ::DeleteFile(index_file_path_temp_.c_str());
            header_ = (QuickIndexHeader*)index_file_.Init(index_file_path_, sizeof(QuickIndexHeader));
        }
        else {
            header_ = (QuickIndexHeader*)index_file_.Init(index_file_path_temp_, sizeof(QuickIndexHeader));
        }
#endif
    }
    return ret;
}

bool QuickIndexImpl::CopyTo( const tstring &target_index_path )
{
    QuickIndexImpl *next = Next();
    if (NULL != next) {
        if (!next->CopyTo(GenerateNextIndexFileName(target_index_path, header_->next_file))) {
            return false;
        }
    }
    QuickIndexImpl *next_large = NextLarge();
    if (NULL != next_large) {
        if (!next_large->CopyTo(GenerateNextIndexFileName(target_index_path, header_->next_large_file))) {
            return false;
        }
    }

    return CopyFile(index_file_path_, target_index_path);
}

EntryIteratorImpl::EntryIteratorImpl( QuickIndexImpl *qii )
{
    root_ = qii;
    current_index_ = qii;
    entry_pos_ = 0;
    Begin();
}

std::string EntryIteratorImpl::GetPath()
{
    if (IsEnd()) {
        return "";
    }
    EntryAddr addr = current_index_->GetEntryAddr(entry_pos_);
    assert(current_index_->IsAddrUsed(addr));
    if (!current_index_->IsAddrUsed(addr)) {
        return "";
    }
    if (current_index_->IsLargeEntry()) {
        EntryLarge *el = static_cast<EntryLarge*>(current_index_->GetEntryByAddr(addr));
        return el->path;
    }
    EntryShort *es = static_cast<EntryShort*>(current_index_->GetEntryByAddr(addr));
    return es->path;
}

Entry EntryIteratorImpl::GetEntry()
{
    if (IsEnd()) {
        return Entry();
    }
    EntryAddr addr = current_index_->GetEntryAddr(entry_pos_);
    assert(current_index_->IsAddrUsed(addr));
    if (!current_index_->IsAddrUsed(addr)) {
        return Entry();
    }

    if (current_index_->IsLargeEntry()) {
        EntryLarge *el = static_cast<EntryLarge*>(current_index_->GetEntryByAddr(addr));
        return el->content;
    }
    EntryShort *es = static_cast<EntryShort*>(current_index_->GetEntryByAddr(addr));
    return es->content;
}

void EntryIteratorImpl::Next()
{
    if (IsEnd()) {
        return;
    }

    entry_pos_++;
    while (NULL != current_index_) {
        for (; entry_pos_ < kMaxEntriesCount; entry_pos_++) {
            EntryAddr addr = current_index_->GetEntryAddr(entry_pos_);
            if (current_index_->IsAddrUsed(addr)) {
                break;
            }
        }
        bool large = false;
        if (entry_pos_ >= kMaxEntriesCount) {
            if (current_index_->IsLargeEntry()) {
                current_index_ = current_index_->NextLarge();
                large = true;
            }
            else {
                current_index_ = current_index_->Next();
                large = false;
            }
            entry_pos_ = 0;
        }
        else {
            break;
        }

        if (NULL == current_index_ && !large) {
            current_index_ = root_->NextLarge();
            entry_pos_ = 0;
        }
    }
}

bool EntryIteratorImpl::IsEnd()
{
    return NULL == current_index_;
}

void EntryIteratorImpl::Clear()
{
    delete this;
}

void EntryIteratorImpl::Begin()
{
    current_index_ = root_;
    for (entry_pos_ = 0; entry_pos_ < kMaxEntriesCount; entry_pos_++) {
        EntryAddr addr = current_index_->GetEntryAddr(entry_pos_);
        if (current_index_->IsAddrUsed(addr)) {
            break;
        }
    }
    if (entry_pos_ >= kMaxEntriesCount) {
        current_index_ = NULL;
    }
}

}
