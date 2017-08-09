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
// convert byte array to T
// return a T-type object (copy constructor is necessary if T is a class)
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

// ��ȡ��1970.1.1�����ھ��������� (����: 100ns)
double fGetCurrentTimestamp();
// ��ȡ��1970.1.1�����ھ�����ÿ100ns������ (����: 100ns)
uint64_t GetCurrentTimestamp();