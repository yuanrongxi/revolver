#ifndef __BASE_HEX_STRING_H
#define __BASE_HEX_STRING_H

#include "base_typedef.h"
#include "base_namespace.h"

#include <string>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL

string bin2asc(const uint8_t *in, int32_t in_size);

void asc2bin(const string& in, uint8_t* out, int32_t out_size, int32_t &out_length);

BASE_NAMESPACE_END_DECL
#endif
