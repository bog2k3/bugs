/*
 * bitFlags.h
 *
 *  Created on: May 13, 2016
 *      Author: bog
 */

#ifndef UTILS_BITFLAGS_H_
#define UTILS_BITFLAGS_H_

#include <type_traits>

// this header allows:
//
//	1. binary operations like & and | on enum class operands

template<typename E>
constexpr E operator | (typename std::enable_if<std::is_enum<E>::value, E>::type e1, E e2) {
	return static_cast<E>(static_cast<typename std::underlying_type<E>::type>(e1) | static_cast<typename std::underlying_type<E>::type>(e2));
}

template<typename E>
constexpr E operator & (typename std::enable_if<std::is_enum<E>::value, E>::type  e1, E e2) {
	return static_cast<E>(static_cast<typename std::underlying_type<E>::type>(e1) & static_cast<typename std::underlying_type<E>::type>(e2));
}

#endif /* UTILS_BITFLAGS_H_ */
