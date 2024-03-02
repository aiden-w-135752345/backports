// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.
#ifndef RYU_D2FIXED_FULL_TABLE_H
#define RYU_D2FIXED_FULL_TABLE_H
#include <cstdint>
namespace ryu{
#define TABLE_SIZE 64
extern const uint16_t POW10_OFFSET[TABLE_SIZE];
extern const uint64_t POW10_SPLIT[1224][3];
#define TABLE_SIZE_2 69
#define ADDITIONAL_BITS_2 120
extern const uint16_t POW10_OFFSET_2[TABLE_SIZE_2];
extern const uint8_t MIN_BLOCK_2[TABLE_SIZE_2];
extern const uint64_t POW10_SPLIT_2[3133][3];
}
#endif // RYU_D2FIXED_FULL_TABLE_H
