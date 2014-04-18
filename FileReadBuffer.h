#ifndef FILE_READ_BUFFER_H
#define FILE_READ_BUFFER_H

namespace filerepo
{
const int ODB_HEADER_MAX_LEN = 64;
const int RAW_BUFFER_LEN = 4096;

typedef struct { /* object header data */
    ObjectType type; /* object type */
    size_t size; /* object size */
} obj_hdr;

class FileReadBuffer
{
public:
    FileReadBuffer();
    ~FileReadBuffer();

    bool Open(const tstring &file_path);
    size_t Read(void *buf, size_t len);
    void CleanUp();
    ObjectType GetType();
    size_t GetSize();

private:
    void InitStream(z_stream *s, void *out, size_t len);
    void SetStreamInput(z_stream *s, void *in, size_t len);
    void SetStreamOutput(z_stream *s, void *out, size_t len);
    size_t GetObjectHeader(obj_hdr *hdr, unsigned char *data);

private:
    HANDLE handle_;
    z_stream zs_; /* zlib */
    size_t len_; // object size
    ObjectType type_;
    size_t headerlen_;
    size_t tail_;
    unsigned char headerbuf_[ODB_HEADER_MAX_LEN];
    unsigned char rawbuf_[RAW_BUFFER_LEN];
};

} // namespace filerepo
#endif
