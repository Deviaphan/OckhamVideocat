#pragma once

#include <stdint.h>


uint32_t MurmurHash3_x86_32( const void * key, int len, uint32_t seed );
