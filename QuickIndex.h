#ifndef QUICK_INDEX_H_H_H
#define QUICK_INDEX_H_H_H

namespace filerepo
{

typedef struct __Entry
{
    ObjectId oid;
    FileStat st;
}Entry;

class EntryIterator
{
public:
    virtual ~EntryIterator() {}

    virtual std::string GetPath() = 0;
    virtual Entry GetEntry() = 0;
    virtual bool Next() = 0;
    virtual bool IsEnd() = 0;
    virtual void Destroy() = 0;
};

class QuickIndex
{
public:
    static QuickIndex *Create(const tstring &index_path, bool read_only);

    // Delete index file from disk
    static bool Delete(const tstring &index_path);

    virtual ~QuickIndex() {}

    virtual bool AddEntry(const std::string &path, const Entry &entry) = 0;

    virtual bool RemoveEntry(const std::string &path) = 0;

    virtual bool FindEntry(const std::string &path, Entry *entry) = 0;

    virtual EntryIterator *GetEntryIterator() = 0;

    virtual size_t GetEntriesCount() = 0;

    // Destroy the object and free memory
    virtual void Destroy() = 0;

    virtual std::string GetEntryPath(const Entry *entry) = 0;
};

} // namespace filerepo

#endif
