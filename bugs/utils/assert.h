/*
 * assert.h
 *
 *  Created on: Jan 28, 2015
 *      Author: bogdan
 */

#ifndef UTILS_ASSERT_H_
#define UTILS_ASSERT_H_

#include <cassert>

#ifdef DEBUG
#define ASSERTDBG_ENABLE
#endif

#ifdef ASSERTDBG_ENABLE
static void assertDbg(bool e) {
	if (!e) {
		__builtin_trap();
	}
}
#else
#define assertDbg assert
#endif

#endif /* UTILS_ASSERT_H_ */
