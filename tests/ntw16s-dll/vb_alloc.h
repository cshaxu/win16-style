#ifndef NTW16S_TEST_VB_ALLOC_H
#define NTW16S_TEST_VB_ALLOC_H
#include <stdlib.h>
/* Forced include only for the private ownership fixture, never the DLL. */
void *ntw16s_test_calloc(size_t count,size_t size);
void ntw16s_test_free(void *memory);
#define calloc ntw16s_test_calloc
#define free ntw16s_test_free
#endif
