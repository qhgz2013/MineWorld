// 
// 本头文件仅用于检测代码是否存在内存泄露的隐患
// 如果已经不需要检测，请务必将 .h 以及 .cpp 文件移出项目，或则将生成操作从 Debug 改成 Release ，否则会影响到new以及delete原本的结构以及性能
// 
// Author & Copyright: Zhou Xuebin
// Software Engineering Class 6, South China University of Technology
// Last Update: 2017-7-22

#pragma once
#include <vector>
#define ESCAPE_OPERATOR_HOOK

struct allocated_pointer
{
	char* file;
	int line;
	size_t size;
	void* pointer;
};
typedef std::vector<allocated_pointer> allocated_pointer_collection;

#ifdef _DEBUG
//overrides global operators

void* operator new(size_t size, const char* file, const int line);
void* operator new(size_t size);
void* operator new[](size_t size);
void* operator new[](size_t size, const char* file, const int line);
void operator delete(void* ptr);
void operator delete[](void* ptr);

#define debug_new new(__FILE__, __LINE__)
#define debug_delete delete

#ifndef ESCAPE_OPERATOR_HOOK
#define new debug_new
#define delete debug_delete
#else

#ifdef new
#undef new
#endif //new
#ifdef delete
#undef delete
#endif //delete

#endif //ESCAPE_OPERATOR_HOOK

#else //_DEBUG
#define debug_new new
#define debug_delete delete
#endif //_DEBUG
void cout_leak_memory();
allocated_pointer_collection get_leak_memory();