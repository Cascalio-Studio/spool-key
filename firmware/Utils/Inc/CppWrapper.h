/**
 * @file    Utils/Inc/CppWrapper.h
 * @brief   C++ Wrapper for C code.
 * @details This file provides a C-compatible interface for C++ code.
 * @author  MootSeeker
 * 
 * @copyright (c) 2025 Cascalio Studio - All Rights Reserved
 */

#ifndef INC_CPP_WRAPPER_H_
#define INC_CPP_WRAPPER_H_

/**
 * @ifdef __cplusplus
 * @brief Ensures the C++ compiler uses C linkage for the enclosed declarations.
 */
#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * @brief C Event loop
	 * @details wrapper for CPP since CubeMX does not allow CPP code generation
	 */
	void EventLoopC( void );

#ifdef __cplusplus
}
#endif

#endif /* INC_CPP_WRAPPER_H_ */
