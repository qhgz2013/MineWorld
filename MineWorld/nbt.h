﻿#pragma once
// nbt(named binary tag) class
#include <string>
#include <iostream>
#include <list>
#include "pointer_check.h"

//Tag类型
enum class TagType
{
	Tag_Undefined,
	Tag_Int, Tag_Long, Tag_Float, Tag_Double, Tag_UInt, Tag_ULong, Tag_Byte,
	Tag_String, Tag_Byte_Array, Tag_List
};
//Tag抽象基类（只能通过指针进行基类转换）
class Tag
{
protected:
	//名称
	std::string _name;
	//类型
	TagType _tagType;
	//数据指针
	void* _data;

	Tag() : _name(""), _tagType(TagType::Tag_Undefined), _data(nullptr) {}
	Tag(TagType tagType, const std::string& name, void* data = nullptr) : _tagType(tagType), _name(name), _data(data) {}
	Tag(const Tag& tag) {}

	//serialize and deserialize functions
	virtual void _read_data(std::istream& s) = 0;
	virtual void _write_data(std::ostream& s) const = 0;
	//virtual void _copy_data(const Tag& tag) = 0;
	void _write_header(std::ostream& s) const;

public:
	//从数据流中读入Tag
	static Tag* ReadTagFromStream(std::istream& s);
	//写入到数据流中
	static void WriteTagFromStream(std::ostream& s, const Tag& tag);
	//复制一个新的Tag
	static Tag* CopyTag(const Tag* tag);
	virtual ~Tag() {}
	//获取Tag的名称
	std::string GetName() const { return _name; }
	//设置Tag的名称
	void SetName(std::string& name) { _name = name; }
	//获取Tag类型
	TagType GetType() const { return _tagType; }

	//获取Tag中的数据（复制到data中，data会自动创建）
	virtual void GetData(void*& data) const = 0;
	//设置Tag中的数据（从data中复制数据到Tag里，data删除后不影响Tag中的数据）
	virtual void SetData(const void*& data) = 0;
	//直接获取Tag中的数据（不经过复制，用于提升性能，可以直接影响到Tag中的数据）
	void* GetDataRef() const { return _data; }
};

class TagInt : public Tag
{
public:
	TagInt(const std::string& name, int data) : Tag(TagType::Tag_Int, name, new int(data)) {}
	TagInt(const std::string& name) : Tag(TagType::Tag_Int, name, nullptr) {}
	TagInt(const TagInt& tag) : Tag(TagType::Tag_Int, tag._name, new int(*(int*)tag._data)) {}
	~TagInt() { if (_data) delete(int*)_data; _data = nullptr; }
	TagInt& operator= (const TagInt& tag);

	void GetData(void*& data) const { data = new int(*(int*)_data); }
	void SetData(const void*& data) { if (_data) delete(int*)_data; _data = new int(*(int*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	friend Tag;
};

class TagLong : public Tag
{
public:
	TagLong(const std::string& name, int64_t data) : Tag(TagType::Tag_Long, name, new int64_t(data)) {}
	TagLong(const std::string& name) : Tag(TagType::Tag_Long, name, nullptr) {}
	TagLong(const TagLong& tag) : Tag(TagType::Tag_Long, tag._name, new int64_t(*(int64_t*)tag._data)) {}
	~TagLong() { if (_data) delete(int64_t*)_data; _data = nullptr; }
	TagLong& operator= (const TagLong& tag);

	void GetData(void*& data) const { data = new int64_t(*(int64_t*)_data); }
	void SetData(const void*& data) { if (_data) delete(int64_t*)_data; _data = new int64_t(*(int64_t*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	friend Tag;
};

class TagFloat : public Tag
{
public:
	TagFloat(const std::string& name, float data) : Tag(TagType::Tag_Float, name, new float(data)) {}
	TagFloat(const std::string& name) : Tag(TagType::Tag_Float, name, nullptr) {}
	TagFloat(const TagFloat& tag) : Tag(TagType::Tag_Float, tag._name, new float(*(float*)tag._data)) {}
	~TagFloat() { if (_data) delete(float*)_data; _data = nullptr; }
	TagFloat& operator= (const TagFloat& tag);

	void GetData(void*& data) const { data = new float(*(float*)_data); }
	void SetData(const void*& data) { if (_data) delete(float*)_data; _data = new float(*(float*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	friend Tag;
};

class TagDouble : public Tag
{
public:
	TagDouble(const std::string& name, double data) : Tag(TagType::Tag_Double, name, new double(data)) {}
	TagDouble(const std::string& name) : Tag(TagType::Tag_Double, name, nullptr) {}
	TagDouble(const TagDouble& tag) : Tag(TagType::Tag_Double, tag._name, new double(*(double*)tag._data)) {}
	~TagDouble() { if (_data) delete(double*)_data; _data = nullptr; }
	TagDouble& operator= (const TagDouble& tag);

	void GetData(void*& data) const { data = new double(*(double*)_data); }
	void SetData(const void*& data) { if (_data) delete(double*)_data; _data = new double(*(double*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	//TagDouble(std::istream& s) : Tag() { _read_data(s); }
	friend Tag;
};

class TagUInt : public Tag
{
public:
	TagUInt(const std::string& name, uint32_t data) : Tag(TagType::Tag_UInt, name, new uint32_t(data)) {}
	TagUInt(const std::string& name) : Tag(TagType::Tag_UInt, name, nullptr) {}
	TagUInt(const TagUInt& tag) : Tag(TagType::Tag_UInt, tag._name, new uint32_t(*(uint32_t*)tag._data)) {}
	~TagUInt() { if (_data) delete(uint32_t*)_data; _data = nullptr; }
	TagUInt& operator= (const TagUInt& tag);

	void GetData(void*& data) const { data = new uint32_t(*(uint32_t*)_data); }
	void SetData(const void*& data) { if (_data) delete(uint32_t*)_data; _data = new uint32_t(*(uint32_t*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	//TagUInt(std::istream& s) : Tag() { _read_data(s); }
	friend Tag;
};

class TagULong : public Tag
{
public:
	TagULong(const std::string& name, uint64_t data) : Tag(TagType::Tag_ULong, name, new uint64_t(data)) {}
	TagULong(const std::string& name) : Tag(TagType::Tag_ULong, name, nullptr) {}
	TagULong(const TagULong& tag) : Tag(TagType::Tag_ULong, tag._name, new uint64_t(*(uint64_t*)tag._data)) {}
	~TagULong() { if (_data) delete(uint64_t*)_data; _data = nullptr; }
	TagULong& operator= (const TagULong& tag);

	void GetData(void*& data) const { data = new uint64_t(*(uint64_t*)_data); }
	void SetData(const void*& data) { if (_data) delete(uint64_t*)_data; _data = new uint64_t(*(uint64_t*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	//TagULong(std::istream& s) :Tag() { _read_data(s); }
	friend Tag;
};

class TagByte : public Tag
{
public:
	TagByte(const std::string& name, char data) : Tag(TagType::Tag_Byte, name, new char(data)) {}
	TagByte(const std::string& name) : Tag(TagType::Tag_Byte, name, nullptr) {}
	TagByte(const TagByte& tag) : Tag(TagType::Tag_Byte, tag._name, new char(*(char*)tag._data)) {}
	~TagByte() { if (_data) delete(char*)_data; _data = nullptr; }
	TagByte& operator= (const TagByte& tag);

	void GetData(void*& data) const { data = new char(*(char*)_data); }
	void SetData(const void*& data) { if (_data) delete(char*)_data; _data = new char(*(char*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	//TagByte(std::istream& s) : Tag() { _read_data(s); }
	friend Tag;
};

class TagString : public Tag
{
public:
	TagString(const std::string& name, const std::string& data) : Tag(TagType::Tag_String, name, new std::string(data)) {}
	TagString(const std::string& name) : Tag(TagType::Tag_String, name, nullptr) {}
	TagString(const TagString& tag) : Tag(TagType::Tag_String, tag._name, new std::string(*(std::string*)tag._data)) {}
	~TagString() { if (_data) delete(std::string*)_data; _data = nullptr; }
	TagString& operator= (const TagString& tag);

	void GetData(void*& data) const { data = new std::string(*(std::string*)_data); }
	void SetData(const void*& data) { if (_data) delete(std::string*)_data; _data = new std::string(*(std::string*)data); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	//TagString(std::istream& s) :Tag() { _read_data(s); }
	friend Tag;
};

class TagByteArray : public Tag
{
protected:
	uint32_t _count;
public:
	TagByteArray(const std::string& name, const char* data, uint32_t count) : Tag(TagType::Tag_Byte_Array, name, new char[count])
	{
		memcpy_s(_data, count, data, count); _count = count;
	}
	TagByteArray(const std::string& name) : Tag(TagType::Tag_Byte_Array, name, nullptr) { _count = 0; }
	TagByteArray(const TagByteArray& tag) : Tag(TagType::Tag_Byte_Array, tag._name, new char[tag._count])
	{
		if (tag._data)
			memcpy_s(_data, tag._count, tag._data, tag._count);
		_count = tag._count;
	}
	~TagByteArray() { if (_data) delete[](char*)_data; _data = nullptr; }
	TagByteArray& operator= (const TagByteArray& tag);

	void GetData(void*& data) const { data = new char[_count]; if (_data) memcpy_s(data, _count, _data, _count); }
	char GetData(int index) const { if (_data && index >= 0 && index < _count) return ((char*)_data)[index]; return '\0'; }
	void SetData(const void*& data) { SetData(data, 1); }
	void SetData(const void*& data, int count) { if (_data) delete[](char*)_data; _data = new char[count]; _count = count; memcpy_s(_data, _count, data, _count); }
	uint32_t GetCount() const { return _count; }

protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	//TagByteArray(std::istream& s) :Tag() { _read_data(s); }
	friend Tag;
};

typedef std::list<Tag*> TagContainer;

class TagList : public Tag
{
public:
	TagList(const std::string& name, const TagContainer& data) : Tag(TagType::Tag_List, name, new TagContainer())
	{
		for (auto i = data.begin(); i != data.end(); i++) ((TagContainer*)_data)->push_back(Tag::CopyTag(*i));
	}
	TagList(const std::string& name) : Tag(TagType::Tag_List, name, nullptr) {}
	TagList(const TagList& tag) : Tag(TagType::Tag_List, tag._name, new TagContainer())
	{
		if (tag._data)
			for (auto i = ((TagContainer*)tag._data)->begin(); i != ((TagContainer*)tag._data)->end(); i++)
				((TagContainer*)_data)->push_back(Tag::CopyTag(*i));
	}
	~TagList()
	{
		if (_data)
		{
			for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
				delete *i;
			delete(TagContainer*)_data;
		}
		_data = nullptr;
	}
	TagList& operator= (const TagList& tag);

	void GetData(void*& data) const
	{
		data = new TagContainer;
		if (_data)
			for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
			{
				((TagContainer*)_data)->push_back(Tag::CopyTag(*i));
			}
	}
	Tag* GetData(std::string& name) const
	{
		if (_data)
			for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
			{
				if ((*i)->GetName() == name)
					return Tag::CopyTag(*i);
			}
		return nullptr;
	}
	void SetData(const void*& data)
	{
		if (_data)
		{
			for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
			{
				delete *i;
			}
			delete(TagContainer*)_data;
		}
		_data = new TagContainer;
		for (auto i = ((TagContainer*)data)->begin(); i != ((TagContainer*)data)->end(); i++)
		{
			((TagContainer*)_data)->push_back(Tag::CopyTag(*i));
		}
	}
	void SetData(Tag* tag);

	void Clear() { if (_data)((TagContainer*)_data)->clear(); }
	bool Exists(std::string& tagname) const;
	void AddData(Tag* tag);
	Tag* GetData(int index) const;
	Tag* operator[](int index) const { return GetData(index); }
	Tag* operator[](std::string& name) const { return GetData(name); }
protected:
	void _read_data(std::istream& s);
	void _write_data(std::ostream& s) const;
	//TagList(std::istream& s) :Tag() { _read_data(s); }
	friend Tag;
};