#pragma once
// Copyright (c) 2019 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"

extern void test_tempmem(uint& num_tests, uint& num_errors);
extern void test_cstrings(uint& num_tests, uint& num_errors);
extern void test_sort(uint& num_tests, uint& num_errors);
extern void test_Array(uint& num_tests, uint& num_errors);
extern void test_hashmap(uint& num_tests, uint& num_errors);
extern void test_StrArray(uint& num_tests, uint& num_errors);
extern void test_RCArray(uint& num_tests, uint& num_errors);
extern void TestStringClass(uint& num_tests, uint& num_errors);
extern void test_relational_operators(uint& num_tests, uint& num_errors);
extern void test_kio_util(uint& num_tests, uint& num_errors);
extern void test_z80_disass(uint& num_tests, uint& num_errors);
extern void test_z80_opcode_length(uint& num_tests, uint& num_errors);
extern void test_z80_major_opcode(uint& num_tests, uint& num_errors);
extern void test_z80_clock_cycles(uint& num_tests, uint& num_errors);

#undef assert
#define assert(X)                                                         \
  do {                                                                    \
	if (!(X)) { throw InternalError(__FILE__, __LINE__, "FAILED: " #X); } \
  }                                                                       \
  while (0)
#undef IERR
#define IERR() throw InternalError(__FILE__, __LINE__, internalerror)

#define TRY    \
  num_tests++; \
  try          \
  {
#define END                  \
  }                          \
  catch (std::exception & e) \
  {                          \
	num_errors++;            \
	logline("%s", e.what()); \
  }
#define EXPECT(X)                                                                      \
  num_errors++;                                                                        \
  logline("%s line %i: FAILED: did not throw", __FILE__, __LINE__);                    \
  }                                                                                    \
  catch (X&)                                                                           \
  {}                                                                                   \
  catch (std::exception&)                                                              \
  {                                                                                    \
	num_errors++;                                                                      \
	logline("%s line %i: FAILED: wrong type of exception thrown", __FILE__, __LINE__); \
  }
