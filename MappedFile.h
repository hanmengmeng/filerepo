#ifndef MAPPED_FILE_H_H_H_H
#define MAPPED_FILE_H_H_H_H

namespace filerepo
{

class FileBlock {
public:
    virtual ~FileBlock() {}

    // Returns a pointer to the actual data.
    virtual void* buffer() const = 0;

    // Returns the size of the block;
    virtual size_t size() const = 0;

    // Returns the file offset of this block.
    virtual int offset() const = 0;
};

class MappedFile : public File
{
public:
    MappedFile();
    virtual ~MappedFile();

    void* Init(const tstring& name, size_t size);

    void* buffer() const {
        return buffer_;
    }

    bool Load(const FileBlock* block);
    bool Store(const FileBlock* block);

    bool SetLength(size_t length);

    void Close();

private:
    bool init_;
    HANDLE section_;
    void* buffer_;  // Address of the memory mapped buffer.
    size_t view_size_;  // Size of the memory pointed by buffer_.

private:
    MappedFile(const MappedFile&);
    MappedFile &operator=(const MappedFile&);
};

}
#endif
