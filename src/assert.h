#ifndef ASSERT_H
#define ASSERT_H

#ifndef NDEBUG
#define ASSERT(condition, message) \
do \
{ \
	if (!condition) \
	{ \
		std::cerr << "Assertion \"" #condition "\" failed in " << __FILE__ \
			<< " on line " << __LINE__ << std::endl \
			<< message << std::endl; \
	} \
} while (false)
#else
#define ASSERT(condition, message) do {} while (false)
#endif

#endif