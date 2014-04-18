#ifndef FILE_H_H_H_H
#define FILE_H_H_H_H

namespace filerepo
{

class File
{
public:
    File();
    ~File();

    bool Init(const tstring& name);
    HANDLE GetFileHandle() const;

    // Returns true if the file was opened properly.
    bool IsValid() const;

    // Performs synchronous IO.
    bool Read(void* buffer, size_t buffer_len, size_t offset);
    bool Write(const void* buffer, size_t buffer_len, size_t offset);

    // Sets the file's length. The file is truncated or extended with zeros to
    // the new length.
    bool SetLength(size_t length);
    size_t GetLength();

    void Close();

private:
    bool init_;
    HANDLE file_;

private:
    File(const File&);
    File &operator=(const File&);
};

}
#endif