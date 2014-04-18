#include "stdafx.h"
#include "sha1.h"

namespace filerepo
{

struct hash_ctx {
    SHA_CTX c;
};

hash_ctx *hash_new_ctx(void)
{
    hash_ctx *ctx = new hash_ctx();

    if (!ctx)
        return NULL;

    SHA1_Init(&ctx->c);

    return ctx;
}

void hash_free_ctx(hash_ctx *ctx)
{
    delete ctx;
}

void hash_init(hash_ctx *ctx)
{
    assert(ctx);
    SHA1_Init(&ctx->c);
}

void hash_update(hash_ctx *ctx, const void *data, size_t len)
{
    assert(ctx);
    SHA1_Update(&ctx->c, data, len);
}

void hash_final(ObjectId *out, hash_ctx *ctx)
{
    assert(ctx);
    SHA1_Final(out->id, &ctx->c);
}

void hash_buf(ObjectId *out, const void *data, size_t len)
{
    SHA_CTX c;

    SHA1_Init(&c);
    SHA1_Update(&c, data, len);
    SHA1_Final(out->id, &c);
}

void hash_vec(ObjectId *out, buf_vec *vec, size_t n)
{
    SHA_CTX c;
    size_t i;

    SHA1_Init(&c);
    for (i = 0; i < n; i++)
        SHA1_Update(&c, vec[i].data, vec[i].len);
    SHA1_Final(out->id, &c);
}

}