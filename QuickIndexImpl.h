#ifndef QUICK_INDEX_IMPL_H_H_H
#define QUICK_INDEX_IMPL_H_H_H

namespace filerepo
{

#define kMaxEntriesCount 65535
#define kMaxShortPath 256
#define kMaxLargePath 4096
#define kInitEntriesCount 1024
#define kInitLargeEntriesCount 16

#define kEntryHashMask 0xFFFF
#define kEntryUseMask 0x1
#define kEntryPositionMask 0xFFFE
#define kEntriesAllocMapCount (kMaxEntriesCount/8 + (kMaxEntriesCount%8 > 0 ? 1 : 0))

typedef unsigned short EntryAddr;

typedef struct __QuickIndexHeader
{
    size_t magic; // Magic value to identify the quick index file
    size_t version; // Quick index file version
    size_t num_entries; // Current entries count that have been stored
    size_t num_entries_capacity; // The max entries count in current file
    unsigned short entry_size; // Every entry size
    unsigned short current_file; // Current index file number
    unsigned short next_file; // Next index file number that store EntryShort
    unsigned short next_large_file; // Next index file number that store EntryLarge
    EntryAddr entries[kMaxEntriesCount]; // This array store entry addr, use path's hash value to locate
    char entries_alloc_map[kEntriesAllocMapCount]; // This array used to identify is an entry position is used
}QuickIndexHeader;

typedef struct __EntryShort
{
    EntryAddr next; // Next conflict entry position
    Entry content;
    char path[kMaxShortPath];
}EntryShort;

typedef struct __EntryLarge
{
    EntryAddr next; // Next conflict entry position
    Entry content;
    char path[kMaxLargePath];
}EntryLarge;

class QuickIndexImpl;

class EntryIteratorImpl : public EntryIterator
{
public:
    EntryIteratorImpl(QuickIndexImpl *qii);

    virtual std::string GetPath();
    virtual Entry GetEntry();
    virtual bool Next();
    virtual bool IsEnd();
    virtual void Destroy();

private:
    QuickIndexImpl *quick_index_;
};

class QuickIndexImpl : public QuickIndex
{
public:
    QuickIndexImpl();
    ~QuickIndexImpl();

    bool Init(const tstring &path, bool read_only);

    virtual bool AddEntry(const std::string &path, const Entry &entry);
    virtual bool RemoveEntry(const std::string &path);
    virtual EntryIterator *GetEntryIterator();
    virtual bool FindEntry(const std::string &path, Entry *entry);
    virtual size_t GetEntriesCount();
    virtual void Destroy();

    virtual std::string GetEntryPath(const Entry *entry);

    // Delete all related index file from disk
    bool DeleteFiles();

private:
    bool GenerateHeader(size_t i, size_t entry_size);
    bool IsFileExist(const tstring &path);
    bool IsHeaderValid();
    bool GrowIndexFile();
    bool IsFileFull();

    size_t GetEntrySize() const;
    bool IsLargeEntry();

    bool Init(const tstring &path, size_t file_number, size_t entry_size, bool read_only);

    size_t GetHash(const std::string &path);
    EntryAddr GetEntryAddr(size_t pos);
    EntryAddr GetEmptyEntryAddr();

    bool IsAddrUsed(EntryAddr addr);
    bool IsAddrValid(EntryAddr addr);
    void *GetEntryByAddr(EntryAddr addr);

    bool UpdateLastConflictEntry(EntryAddr addr, EntryAddr next);

    size_t GetFileSize( LPCTSTR file_path );

    QuickIndexImpl *Next();
    QuickIndexImpl *NextLarge();

    void UpdateEntriesAllocMap(EntryAddr addr, bool is_used);

    bool PrepareIndexFile();

    void UpdateEntry(EntryAddr addr, const std::string &path, const Entry &entry);

    tstring GenerateNextIndexFileName(size_t index_number);

    bool FindEntryInCurrentIndex(const std::string &path, EntryAddr &found_addr);

    bool UseLargeEntryToStore(const std::string &path);

private:
    QuickIndexHeader *header_;
    MappedFile index_file_;
    tstring index_file_path_;
    tstring index_file_path_temp_;
    QuickIndexImpl *next_large_index_;
    QuickIndexImpl *next_index_;
    bool index_file_modify_;
};

} // namespace filerepo
#endif
