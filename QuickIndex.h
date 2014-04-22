#ifndef QUICK_INDEX_H_H_H
#define QUICK_INDEX_H_H_H

namespace filerepo
{

const int ENTRY_FLAG_IS_DIR = (1 << 0);

typedef struct __Entry
{
    ObjectId oid;
    FileStat st;
    short flag;
}Entry;

class EntryIterator
{
public:
    virtual ~EntryIterator() {}

    virtual std::string GetPath() = 0;
    virtual Entry GetEntry() = 0;
    virtual void Begin() = 0;
    virtual void Next() = 0;
    virtual bool IsEnd() = 0;
    virtual void Clear() = 0;
};

class QuickIndex
{
public:
    static QuickIndex *Open(const tstring &index_path, bool read_only);

    // Delete index file from disk (may be several files)
    static bool DeleteIndexFromDisk(const tstring &index_path);

    virtual ~QuickIndex() {}

    virtual bool AddEntry(const std::string &path, const Entry &entry) = 0;

    virtual bool RemoveEntry(const std::string &path) = 0;

    virtual bool FindEntry(const std::string &path, Entry *entry) = 0;

    virtual EntryIterator *GetEntryIterator() = 0;

    virtual size_t GetEntriesCount() = 0;

    virtual std::string GetEntryPath(const Entry *entry) = 0;

    // Save changes to disk
    virtual bool Save() = 0;

    virtual bool CopyTo(const tstring &target_index_path) = 0;

    // Destroy the object and free memory
    virtual void Clear() = 0;
};

} // namespace filerepo

#endif
