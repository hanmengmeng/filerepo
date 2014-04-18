#ifndef ERROR_CODE_H
#define ERROR_CODE_H

namespace filerepo
{

enum
{
    SUCCEED = 0,
    HASH_FILE_FAILED,
    CREATE_DIRECTORY_FAILED,
    COMMIT_FILE_FAILED,
};

}
#endif