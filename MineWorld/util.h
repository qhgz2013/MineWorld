#pragma once
#include "pointer_check.h"
// convert T to byte array:
// return a char array (size is equal to sizeof(T))
template<typename T>
char* ConvertToByteArray(const T _data)
{
	union _t {
		T data;
		char bytes[sizeof(T)];
	};

	_t temp_union;
	temp_union.data = _data;
	char* result = debug_new char[sizeof(T)];
	memcpy_s(result, sizeof(T), temp_union.bytes, sizeof(T));
	return result;
}

template<typename T>
T ConvertFromByteArray(const char* _data)
{
	union _t {
		T data;
		char bytes[sizeof(T)];
	};

	_t temp_union;
	memcpy_s(temp_union.bytes, sizeof(T), _data, sizeof(T));
	return temp_union.data;
}

