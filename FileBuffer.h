#ifndef FILE_BUFFER_H
#define FILE_BUFFER_H

namespace filerepo
{
#define FILEBUF_HASH_CONTENTS		(1 << 0)
#define FILEBUF_APPEND				(1 << 2)
#define FILEBUF_FORCE				(1 << 3)
#define FILEBUF_TEMPORARY			(1 << 4)
#define FILEBUF_DO_NOT_BUFFER		(1 << 5)
#define FILEBUF_DEFLATE_SHIFT		(6)

#define FILELOCK_EXTENSION ".lock\0"
#define FILELOCK_EXTLENGTH 6

struct hash_ctx;

class FileBuffer;
typedef size_t (FileBuffer::*fn_write)(void *source, size_t len);

class FileBuffer
{
public:
    FileBuffer();
    ~FileBuffer();

    bool Open(const tstring &path, int flags);
    size_t Write(const void *buff, size_t len);
    bool Reserve(void **buff, size_t len);
    size_t Printf(const char *format, ...);
    bool GetHash(ObjectId *oid);
    bool Commit(mode_t mode);
    bool CommitAt(const tstring &path, mode_t mode);
    size_t Flush();
    void CleanUp();

private:
    size_t WriteNormal(void *source, size_t len);
    size_t WriteDeflate(void *source, size_t len);
    void AddToCache(const void *buf, size_t len);
    size_t FlushBuffer();

private:
    unsigned char *buffer_;
    unsigned char *z_buf_;
    z_stream zs_;
    int flush_mode_;
    hash_ctx *digest_;

    bool do_not_buffer_;
    size_t buf_size_;
    size_t buf_pos_;
    HANDLE handle_;
    int last_error_;
    bool do_not_compress_;
    tstring path_origin_;
    tstring path_lock_;
    fn_write write_;
};

} // namespace filerepo

#endif
