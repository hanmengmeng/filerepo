#ifndef HASH_H_H_H
#define HASH_H_H_H

namespace filerepo
{

unsigned int FastHash(const std::string data);
unsigned int FastHash(const char * data, int len);

}
#endif