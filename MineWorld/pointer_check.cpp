#include <cstdlib>
#include <cstring>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "pointer_check.h"
using namespace std;
typedef char* pointer;
pointer head_pointer = nullptr; //最后一个申请内存空间的地址
uint64_t allocated_block = 0; //已经申请的内存的数量
#define POINTER_CHECK_OUTPUT_DETAILED_INFO
/*
* ------------------------------------------------
*                 S T R U C T U R E
* ------------------------------------------------
*
* if uses new(size_t, const char*, const int) operator:
*
* base allocated address
* V    char *             char *           char *             int            size_t        void *           data type
* +------------------+--------------+-----------------+-----------------+-------------+==============+
* | previous pointer | next pointer | call stack file | call stack line | size in use | user pointer |      structure
* +------------------+--------------+-----------------+-----------------+-------------+==============+
*    +0  +1  +2  +3    +4 +5 +6 +7     +8  +9  +A  +B    +C  +D  +E  +F +10 +11 +12 +13 +14 ~ +13+size      offset
*
*
* then the user would get the pointer
*
* returned address
* V
* +==============+
* | user pointer |
* +==============+
*   +0 ~ +size-1
*
*
*
* if uses new(size_t) operator only:
*
* base allocated address
* V    char *             char *                   char *                         int                size_t        void *           data type
* +------------------+--------------+--------------------------------+--------------------------+-------------+==============+
* | previous pointer | next pointer | call stack file(const nullptr) | call stack line(const 0) | size in use | user pointer |      structure
* +------------------+--------------+--------------------------------+--------------------------+-------------+==============+
*    +0  +1  +2  +3    +4 +5 +6 +7      +8      +9      +A      +B      +C    +D    +E    +F    +10 +11 +12 +13 +14 ~ +13+size      offset
*
*
*
* then when deleting pointer, the operation will based on the user pointer and will forwarded to base allocated address:
*
* base allocated address                                                              returned address
* V                                                                                   V
* +------------------+--------------+-----------------+-----------------+-------------+==============+
* | previous pointer | next pointer | call stack file | call stack line | size in use | user pointer |
* +------------------+--------------+-----------------+-----------------+-------------+==============+
*  -14  -13  -12  -11  -10 -F -E -D    -C  -B  -A  -9    -8  -7  -6  -5    -4 -3 -2 -1   +0 ~ +size-1
*
*
* then just delete the base address
*
*
* -------------------------------------------------
*                D A T A   M O D E L
* -------------------------------------------------
*
*
* each time a block is created, the head_pointer will refer to the newest pointer,
* and we use a linked list model to save all the allocated address.
*
*             head_pointer
*             V
*             +=========+  next pointer   +=========+                     +=========+
*             | pointer | --------------> | pointer | -->    ......   --> | pointer | --> nullptr
* nullptr <-- |    I    | <-------------- |    II   | <--    ``````   <-- |    n    |
*             +=========+  prev pointer   +=========+                     +=========+
*
*
* and each time a block is deleted, then just affect the nearby node and remove it from the linked list
*
* (before)
*             head_pointer                               the pointer being deleted
*             V                                          V
*             +=========+                +=========+     +=========+     +=========+                  +=========+
*             | pointer | --> ...... --> | pointer | --> | pointer | --> | pointer | --> ......   --> | pointer | --> nullptr
* nullptr <-- |    I    | <-- `````` <-- |   x-1   | <-- |    x    | <-- |   x+1   | <-- ``````   <-- |    n    |
*             +=========+                +=========+     +=========+     +=========+                  +=========+
*
*
* (after)
*
*                                                            next pointer
*                                         +--------------------------------------------+
*             head_pointer                |                  the pointer being deleted |
*             V                           |                  V                         V
*             +=========+                +=========+         +=========+         +=========+                  +=========+
*             | pointer | --> ...... --> | pointer | -- x -> | pointer | -- x -> | pointer | --> ......   --> | pointer | --> nullptr
* nullptr <-- |    I    | <-- `````` <-- |   x-1   | <- x -- |    x    | <- x -- |   x+1   | <-- ``````   <-- |    n    |
*             +=========+                +=========+         +=========+         +=========+                  +=========+
*	                                           A                                    |
*                                              +------------------------------------+
*                                                            prev pointer
*
* if head_pointer will be deleted, then we will set it to the next pointer.
*/

#ifdef _DEBUG
#undef new
#undef delete

void* operator new(size_t size, const char* file, const int line)
{
	size_t total_size = size + sizeof(pointer) * 3 + sizeof(int) + sizeof(size_t);
	pointer memory = (pointer)malloc(total_size); //用c语言的方式申请内存

	pointer* prev_pointer = (pointer*)memory; //指向链表的上一个元素的地址
	pointer* next_pointer = (pointer*)(memory + sizeof(pointer)); //指向链表的下一个元素的地址
	char** call_stack_file = (char**)(memory + sizeof(pointer) * 2); //调用new运算符所在的源文件
	int* call_stack_line = (int*)(memory + sizeof(pointer) * 3); //调用new函数所在源文件的行数
	size_t* size_in_use = (size_t*)(memory + sizeof(pointer) * 3 + sizeof(int)); //要分配的内存控件的大小

	pointer base_pointer = (pointer)(memory + sizeof(pointer) * 3 + sizeof(int) + sizeof(size_t)); //返回给用户使用的内存地址

	//初始化指针的前置数值
	if (file != nullptr)
	{
		int file_length = strlen(file) + 1;
		*call_stack_file = (char*)malloc(file_length);
		memcpy_s(*call_stack_file, file_length, file, file_length);
	}
	else
	{
		*call_stack_file = nullptr;
	}

	*call_stack_line = line;
	*size_in_use = size;

	//添加到内存申请的链表中
	*prev_pointer = nullptr;
	*next_pointer = head_pointer;

	if (head_pointer != nullptr)
	{
		pointer* node_prev_pointer = (pointer*)head_pointer;
		*node_prev_pointer = memory;
	}
	head_pointer = memory;

	allocated_block++;
#ifdef POINTER_CHECK_OUTPUT_DETAILED_INFO
	cout << "allocated block: 0x" << (void*)memory << "(size: " << size << " B, file: " << (const char*)(file ? file : "") << ", line: " << line << endl;
	cout << "prev= 0x" << (void*)*prev_pointer << ", next= 0x" << (void*)*next_pointer << ", head= 0x" << (void*)head_pointer << endl;
#ifdef _WIN32
	char buf[4096];
	sprintf_s(buf, "allocated block: %p(size: %d B, file: %s, line: %d)\r\nprev= %p, next= %p, head= %p\r\n", memory, size, file ? file : "", line, *prev_pointer, *next_pointer, head_pointer);
	OutputDebugStringA(buf);
#endif
#endif
	return base_pointer;
}

void* operator new(size_t size)
{
	return operator new(size, nullptr, 0);
}

void * operator new[](size_t size)
{
	return operator new(size, nullptr, 0);
}

void * operator new[](size_t size, const char * file, const int line)
{
	return operator new(size, file, line);
}

void operator delete(void* ptr)
{
	if (ptr == nullptr) return;

	pointer base_pointer = (pointer)ptr;

	pointer memory = (pointer)(base_pointer - sizeof(pointer) * 3 - sizeof(int) - sizeof(size_t));

	pointer* prev_pointer = (pointer*)memory;
	pointer* next_pointer = (pointer*)(memory + sizeof(pointer));
	char** call_stack_file = (char**)(memory + sizeof(pointer) * 2);
	int* call_stack_line = (int*)(memory + sizeof(pointer) * 3);
	size_t* size_in_use = (size_t*)(memory + sizeof(pointer) * 3 + sizeof(int));

#ifdef POINTER_CHECK_OUTPUT_DETAILED_INFO
	cout << "released block: 0x" << (void*)memory << "(size: " << *size_in_use << " B, file: " << (const char*)(*call_stack_file ? *call_stack_file : "") << ", line: " << *call_stack_line << endl;
	cout << "prev= 0x" << (void*)*prev_pointer << ", next= 0x" << (void*)*next_pointer << ", head= 0x" << (void*)head_pointer << endl;
#ifdef _WIN32
	char buf[4096];
	sprintf_s(buf, "released block: %p(size: %d B, file: %s, line: %d)\r\nprev= %p, next= %p, head= %p\r\n", memory, *size_in_use, *call_stack_file ? *call_stack_file : "", *call_stack_line, *prev_pointer, *next_pointer, head_pointer);
	OutputDebugStringA(buf);
#endif
#endif //POINTER_CHECK_OUTPUT_DETAILED_INFO

	if (*call_stack_file)
	{
		free(*call_stack_file);
		*call_stack_file = nullptr;
	}

	if (*prev_pointer)
	{
		pointer* prev_next_pointer = (pointer*)((*prev_pointer) + sizeof(pointer));
		*prev_next_pointer = *next_pointer;
	}
	if (*next_pointer)
	{
		pointer* next_prev_pointer = (pointer*)((*next_pointer));
		*next_prev_pointer = *prev_pointer;
	}
	if (head_pointer == memory)
	{
		head_pointer = *next_pointer;
	}

	allocated_block--;
	free(memory);
}

void operator delete[](void * ptr)
{
	operator delete(ptr);
}

#endif

void cout_leak_memory()
{
	if (allocated_block)
	{
		cout << endl << "The following pointer(s) are not released (count: " << allocated_block << ")" << endl;

		pointer current_pointer = head_pointer;
		while (current_pointer)
		{
			pointer* prev_pointer = (pointer*)current_pointer;
			pointer* next_pointer = (pointer*)(current_pointer + sizeof(pointer));
			char** call_stack_file = (char**)(current_pointer + sizeof(pointer) * 2);
			int* call_stack_line = (int*)(current_pointer + sizeof(pointer) * 3);
			size_t* size_in_use = (size_t*)(current_pointer + sizeof(pointer) * 3 + 4);

			pointer base_pointer = (pointer)(current_pointer + sizeof(pointer) * 3 + 8);


			cout << (*call_stack_file) << "  Line:" << (*call_stack_line) << "  0x" << (void*)base_pointer << " (" << (*size_in_use) << " Bytes)" << endl;

			current_pointer = *next_pointer;
		}

	}
	else
	{
		cout << endl << "All pointers are released, congratulations!" << endl << "You can remove the pointer_check code from your project!" << endl;
	}
}

allocated_pointer_collection get_leak_memory()
{
	pointer _head_pointer = head_pointer;
	size_t _allocated_block = allocated_block;
#pragma warning(push)
#pragma warning(disable: 4244)
	allocated_pointer_collection ret = allocated_pointer_collection(_allocated_block);
#pragma warning(pop)
	pointer current_pointer = _head_pointer;
	int count = 0;
	while (current_pointer)
	{
		pointer* prev_pointer = (pointer*)current_pointer;
		pointer* next_pointer = (pointer*)(current_pointer + sizeof(pointer));
		char** call_stack_file = (char**)(current_pointer + sizeof(pointer) * 2);
		int* call_stack_line = (int*)(current_pointer + sizeof(pointer) * 3);
		size_t* size_in_use = (size_t*)(current_pointer + sizeof(pointer) * 3 + sizeof(int));

		pointer base_pointer = (pointer)(current_pointer + sizeof(pointer) * 3 + sizeof(int) + sizeof(size_t));

		allocated_pointer ap = allocated_pointer{ *call_stack_file, *call_stack_line, *size_in_use, base_pointer };

		//ret.push_back(ap);
		ret[count++] = ap;
		current_pointer = *next_pointer;
	}

	return ret;
}
