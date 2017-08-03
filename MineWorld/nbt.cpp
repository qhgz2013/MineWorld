#include "nbt.h"
#include "util.h"

void Tag::_write_header(std::ostream & s) const
{
	char type = (char)_tagType;
	s.write(&type, 1);
	size_t name_length_origin = _name.length();
	uint16_t name_length = 0;
	if (name_length_origin > UINT16_MAX) name_length = UINT16_MAX;
	else name_length = (uint16_t)name_length_origin;
	char* length = ConvertToByteArray(name_length);
	s.write(length, 2);
	debug_delete[] length;
	s.write(_name.c_str(), name_length);
}

Tag * Tag::ReadTagFromStream(std::istream & s)
{
	Tag* ret = nullptr;
	char ctype = 0;
	s.read(&ctype, 1);
	TagType type = (TagType)ctype;
	char name_length_data[2];
	s.read(name_length_data, 2);
	uint16_t name_length = ConvertFromByteArray<int>(name_length_data);
	char* name = debug_new char[name_length + 1];
	s.read(name, name_length);
	name[name_length] = 0;

	switch (type)
	{
	case TagType::Tag_Undefined:
		break;
	case TagType::Tag_Int:
		ret = debug_new TagInt(name);
		break;
	case TagType::Tag_Long:
		ret = debug_new TagLong(name);
		break;
	case TagType::Tag_Float:
		ret = debug_new TagFloat(name);
		break;
	case TagType::Tag_Double:
		ret = debug_new TagDouble(name);
		break;
	case TagType::Tag_UInt:
		ret = debug_new TagUInt(name);
		break;
	case TagType::Tag_ULong:
		ret = debug_new TagULong(name);
		break;
	case TagType::Tag_Byte:
		ret = debug_new TagByte(name);
		break;
	case TagType::Tag_String:
		ret = debug_new TagString(name);
		break;
	case TagType::Tag_Byte_Array:
		ret = debug_new TagByteArray(name);
		break;
	case TagType::Tag_List:
		ret = debug_new TagList(name);
		break;
	default:
		break;
	}
	if (ret)
	{
		ret->_read_data(s);
	}
	debug_delete[] name;
	return ret;
}

void Tag::WriteTagFromStream(std::ostream & s, const Tag& tag)
{
	tag._write_header(s);
	tag._write_data(s);
}

Tag * Tag::CopyTag(const Tag * tag)
{
	Tag* ret = nullptr;
	switch (tag->_tagType)
	{
	case TagType::Tag_Undefined:
		break;
	case TagType::Tag_Int:
		ret = debug_new TagInt(*(TagInt*)tag);
		break;
	case TagType::Tag_Long:
		ret = debug_new TagLong(*(TagLong*)tag);
		break;
	case TagType::Tag_Float:
		ret = debug_new TagFloat(*(TagFloat*)tag);
		break;
	case TagType::Tag_Double:
		ret = debug_new TagDouble(*(TagDouble*)tag);
		break;
	case TagType::Tag_UInt:
		ret = debug_new TagUInt(*(TagUInt*)tag);
		break;
	case TagType::Tag_ULong:
		ret = debug_new TagULong(*(TagULong*)tag);
		break;
	case TagType::Tag_Byte:
		ret = debug_new TagByte(*(TagByte*)tag);
		break;
	case TagType::Tag_String:
		ret = debug_new TagString(*(TagString*)tag);
		break;
	case TagType::Tag_Byte_Array:
		ret = debug_new TagByteArray(*(TagByteArray*)tag);
		break;
	case TagType::Tag_List:
		ret = debug_new TagList(*(TagList*)tag);
		break;
	default:
		break;
	}
	return ret;
}

TagInt & TagInt::operator=(const TagInt & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((int*)_data) = *((int*)tag._data);
		else
			_data = debug_new int(*(int*)tag._data);
	else
		if (_data)
		{
			debug_delete(int*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagInt::_read_data(std::istream & s)
{
	char buf[4];
	s.read(buf, 4);
	_data = debug_new int(ConvertFromByteArray<int>(buf));
}

void TagInt::_write_data(std::ostream & s) const
{
	char* data = ConvertToByteArray<int>(_data ? (*(int*)_data) : 0i32);
	s.write(data, 4);
	debug_delete[] data;
}

TagLong & TagLong::operator=(const TagLong & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((int64_t*)_data) = *((int64_t*)tag._data);
		else
			_data = debug_new int64_t(*(int64_t*)tag._data);
	else
		if (_data)
		{
			debug_delete(int64_t*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagLong::_read_data(std::istream & s)
{
	char buf[8];
	s.read(buf, 8);
	_data = debug_new int64_t(ConvertFromByteArray<int64_t>(buf));
}

void TagLong::_write_data(std::ostream & s) const
{
	char* data = ConvertToByteArray<int64_t>(_data ? (*(int64_t*)_data) : 0i64);
	s.write(data, 8);
	debug_delete[] data;
}

TagFloat & TagFloat::operator=(const TagFloat & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((float*)_data) = *((float*)tag._data);
		else
			_data = debug_new float(*(float*)tag._data);
	else
		if (_data)
		{
			debug_delete(float*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagFloat::_read_data(std::istream & s)
{
	char buf[4];
	s.read(buf, 4);
	_data = debug_new float(ConvertFromByteArray<float>(buf));
}

void TagFloat::_write_data(std::ostream & s) const
{
	char* data = ConvertToByteArray<float>(_data ? (*(float*)_data) : 0.0f);
	s.write(data, 4);
	debug_delete[] data;
}

TagDouble & TagDouble::operator=(const TagDouble & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((double*)_data) = *((double*)tag._data);
		else
			_data = debug_new double(*(double*)tag._data);
	else
		if (_data)
		{
			debug_delete(double*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagDouble::_read_data(std::istream & s)
{
	char buf[8];
	s.read(buf, 8);
	_data = debug_new double(ConvertFromByteArray<double>(buf));
}

void TagDouble::_write_data(std::ostream & s) const
{
	char* data = ConvertToByteArray<double>(_data ? (*(double*)_data) : 0.0);
	s.write(data, 8);
	debug_delete[] data;
}

TagUInt & TagUInt::operator=(const TagUInt & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((uint32_t*)_data) = *((uint32_t*)tag._data);
		else
			_data = debug_new uint32_t(*(uint32_t*)tag._data);
	else
		if (_data)
		{
			debug_delete(uint32_t*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagUInt::_read_data(std::istream & s)
{
	char buf[4];
	s.read(buf, 4);
	_data = new uint32_t(ConvertFromByteArray<uint32_t>(buf));
}

void TagUInt::_write_data(std::ostream & s) const
{
	char* data = ConvertToByteArray<uint32_t>(_data ? (*(uint32_t*)_data) : 0ui32);
	s.write(data, 4);
	debug_delete[] data;
}

TagULong & TagULong::operator=(const TagULong & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((uint64_t*)_data) = *((uint64_t*)tag._data);
		else
			_data = debug_new uint64_t(*(uint64_t*)tag._data);
	else
		if (_data)
		{
			debug_delete(uint64_t*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagULong::_read_data(std::istream & s)
{
	char buf[8];
	s.read(buf, 8);
	_data = debug_new uint64_t(ConvertFromByteArray<uint64_t>(buf));
}

void TagULong::_write_data(std::ostream & s) const
{
	char* data = ConvertToByteArray<uint64_t>(_data ? (*(uint64_t*)_data) : 0ui64);
	s.write(data, 8);
	debug_delete[] data;
}

TagByte & TagByte::operator=(const TagByte & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((char*)_data) = *((char*)tag._data);
		else
			_data = debug_new char(*(char*)tag._data);
	else
		if (_data)
		{
			debug_delete(char*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagByte::_read_data(std::istream & s)
{
	char buf;
	s.read(&buf, 1);
	_data = debug_new char(buf);
}

void TagByte::_write_data(std::ostream & s) const
{
	char data = _data ? (*(char*)_data) : '\0';
	s.write(&data, 1);
}

TagString & TagString::operator=(const TagString & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
			*((std::string*)_data) = *((std::string*)tag._data);
		else
			_data = debug_new std::string(*(std::string*)tag._data);
	else
		if (_data)
		{
			debug_delete(std::string*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagString::_read_data(std::istream & s)
{
	char len_buf[4];
	s.read(len_buf, 4);
	uint32_t len = ConvertFromByteArray<uint32_t>(len_buf);
	char* buf = debug_new char[len + 1];
	s.read(buf, len);
	buf[len] = 0;
	_data = debug_new std::string(buf);
	debug_delete[] buf;
}

void TagString::_write_data(std::ostream & s) const
{
	std::string* data = (std::string*)_data;
	size_t len_origin = data ? data->length() : (size_t)0;
	uint32_t len = 0;
	if (len_origin > UINT32_MAX) len = UINT32_MAX;
	else len = (uint32_t)len_origin;
	char* len_data = ConvertToByteArray<uint32_t>(len);
	s.write(len_data, 4);
	debug_delete[] len_data;
	if (data) s.write(data->c_str(), len);
}

TagByteArray & TagByteArray::operator=(const TagByteArray & tag)
{
	_name = tag._name;
	_count = tag._count;
	if (tag._data)
		if (_data)
		{
			debug_delete[](char*)_data;
			_data = debug_new char[_count];
			memcpy_s(_data, _count, tag._data, _count);
		}
		else
		{
			_data = debug_new char[_count];
			memcpy_s(_data, _count, tag._data, _count);
		}
	else
		if (_data)
		{
			debug_delete[](char*)_data; _data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagByteArray::_read_data(std::istream & s)
{
	char len_data[4];
	s.read(len_data, 4);
	_count = ConvertFromByteArray<uint32_t>(len_data);

	char *data = debug_new char[_count];
	s.read(data, _count);
	_data = data;
}

void TagByteArray::_write_data(std::ostream & s) const
{
	char* len_data = ConvertToByteArray<uint32_t>(_count);
	s.write(len_data, 4);
	debug_delete[] len_data;
	char* data = (char*)_data;
	if (data) s.write(data, _count);
}

TagList & TagList::operator=(const TagList & tag)
{
	_name = tag._name;
	if (tag._data)
		if (_data)
		{
			TagContainer* data = (TagContainer*)_data;
			for (auto i = data->begin(); i != data->end(); i++)
				debug_delete *i;
			data->clear();
			for (auto i = ((TagContainer*)tag._data)->begin(); i != ((TagContainer*)tag._data)->end(); i++)
				data->push_back(Tag::CopyTag(*i));
		}
		else
		{
			_data = debug_new TagContainer;
			TagContainer* data = (TagContainer*)_data;
			for (auto i = ((TagContainer*)tag._data)->begin(); i != ((TagContainer*)tag._data)->end(); i++)
				data->push_back(Tag::CopyTag(*i));
		}
	else
		if (_data)
		{
			TagContainer* data = (TagContainer*)_data;
			for (auto i = data->begin(); i != data->end(); i++)
				debug_delete *i;
			debug_delete data;
			_data = nullptr;
		}
		else
			_data = nullptr;
	return *this;
}

void TagList::SetData(Tag * tag)
{
	if (_data)
		for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
		{
			if ((*i)->GetName() == tag->GetName())
			{
				*i = Tag::CopyTag(tag);
				return;
			}
		}

	if (!_data) _data = debug_new TagContainer;
	((TagContainer*)_data)->push_back(Tag::CopyTag(tag));
}

bool TagList::Exists(std::string & tagname) const
{
	if (_data)
		for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
		{
			if ((*i)->GetName() == tagname)
				return true;
		}
	return false;
}

void TagList::AddData(Tag * tag)
{
	if (!_data)
		_data = debug_new TagContainer;
	((TagContainer*)_data)->push_back(Tag::CopyTag(tag));
}

Tag * TagList::GetData(int index) const
{
	if (_data)
	{
		int ii = 0;
		for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
		{
			if (ii == index) return Tag::CopyTag(*i);
			ii++;
		}
	}
	return nullptr;
}

void TagList::_read_data(std::istream & s)
{
	char len_data[4];
	s.read(len_data, 4);
	uint32_t len = ConvertFromByteArray<uint32_t>(len_data);
	if (!_data) _data = debug_new TagContainer;
	else 
	{
		for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end(); i++)
		{
			debug_delete *i;
		}
		((TagContainer*)_data)->clear();
	}
	for (uint32_t i = 0; i < len; i++)
	{
		((TagContainer*)_data)->push_back(Tag::ReadTagFromStream(s));
	}
}

void TagList::_write_data(std::ostream & s) const
{
	size_t len_origin = _data ? ((TagContainer*)_data)->size() : (size_t)0;
	uint32_t len = 0;
	if (len_origin > UINT32_MAX) len = UINT32_MAX;
	else len = (uint32_t)len_origin;

	char* len_data = ConvertToByteArray<uint32_t>(len);
	s.write(len_data, 4);
	debug_delete[] len_data;
	uint32_t count = 0;
	if (_data)
		for (auto i = ((TagContainer*)_data)->begin(); i != ((TagContainer*)_data)->end() && count < len; i++, count++)
		{
			Tag::WriteTagFromStream(s, **i);
		}
}
