#pragma once

//#define NO_ASSERT 1

#if (defined(_DEBUG) || defined(DEBUG_RELEASE)) && !defined(NO_ASSERT)
 /**
 * \brief Used to ensure logical statement validity and abort on fail
 * \param[in] value Any boolean statement which normally should return TRUE
 * \param[in] message Error message to output
 * \remarks Define DEBUG_RELEASE to assert on Release build
 */
#define ASSERT_CRITICAL(value, message) \
	impl_utils::assertCriticalLog(value, message, __FILE__, __LINE__);

 /**
 * \brief Used to ensure logical statement validity without terminating
 * \param[in] value Any boolean statement which normally should return TRUE
 * \param[in] message Error message to output
 * \returns Whether assertion has failed
 * \remarks Define DEBUG_RELEASE to assert on Release build
 */
#define ASSERT_FAIL(value, message) \
	impl_utils::assertFailLog(value, message, __FILE__, __LINE__)

#else
#define ASSERT_CRITICAL(value, message) \
	(value);
#define ASSERT_FAIL(value, message) \
	(!(value))
#endif

#include <string>
namespace impl_utils
{
	void assertCriticalLog(bool value, std::string message, const char* fileName, const int lineNumber);
	bool assertFailLog(bool value, std::string message, const char* fileName, const int lineNumber);
}