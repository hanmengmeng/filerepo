#ifndef SHA1_HASH_H
#define SHA1_HASH_H

namespace filerepo
{

typedef struct hash_ctx hash_ctx;

typedef struct {
    void *data;
    size_t len;
} buf_vec;

hash_ctx *hash_new_ctx(void);
void hash_free_ctx(hash_ctx *ctx);

void hash_init(hash_ctx *c);
void hash_update(hash_ctx *c, const void *data, size_t len);
void hash_final(ObjectId *out, hash_ctx *c);

void hash_buf(ObjectId *out, const void *data, size_t len);
void hash_vec(ObjectId *out, buf_vec *vec, size_t n);

}
#endif