#ifndef ASSERT_H
#define ASSERT_H

#include <assert.h>

#ifndef NDEBUG
#define ASSERT(condition, object, message) \
do \
{ \
    if (!condition) \
    { \
        std::cerr << "Assertion \"" #condition "\" failed in " << __FILE__ \
            << " on line " << __LINE__ << std::endl \
            << message << std::endl \
            << object << std::endl; \
        assert(false); \
    } \
} while (false)
#else
#define ASSERT(condition, message) do {} while (false)
#endif

#endif