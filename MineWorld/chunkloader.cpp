﻿#include "chunkloader.h"
#include <cstdio>
#include <io.h>
#include <direct.h>
#include <fstream>
#include <cmath>
#include "pointer_check.h"
//zlib compress mode
#include <sstream>
#include "zlib-1.2.11\zlib.h"
#pragma comment(lib, "zlib.lib")

using namespace std;
const int IO_MODE_EXIST = 0;
const int IO_MODE_WRITE = 2;
const int IO_MODE_READ = 4;
const int IO_MODE_READWRITE = 6;
const char* DATA_VERSION = "1.0.0";

ChunkLoader::~ChunkLoader()
{
	auto i1 = _loaded_data.begin();
	auto i2 = _data_changed.begin();
	auto i3 = _loaded_pos.begin();
	for (; i1 != _loaded_data.end(); i1++, i2++, i3++)
	{
		if (*i2)
		{
			_write_chunk_to_file(i3->first, i3->second, *i1);
		}
		delete *i1;
	}
}

void ChunkLoader::GetChunkData(int cx, int cy, char**& data)
{
	int size = 1 << DEFAULT_CHUNK_SIZE;
	TagByteArray* tag_data = (TagByteArray*)_get_chunk_data(cx, cy);
	if (!tag_data) return;
	char* raw_data = (char*)tag_data->GetDataRef();

	data = new char*[size];
	for (int i = 0; i < size; i++)
	{
		data[i] = new char[size];
		memcpy_s(data[i], size, raw_data + i * size, size);
	}
}

char ChunkLoader::GetBlockData(int bx, int by)
{
	int size = 1 << DEFAULT_CHUNK_SIZE;

	auto chunk_pos = GetChunkPos(bx, by);
	TagByteArray* tag_data = (TagByteArray*)_get_chunk_data(chunk_pos.first, chunk_pos.second);
	if (!tag_data) return '\0';
	char* raw_data = (char*)tag_data->GetDataRef();
	return raw_data[size * (bx - chunk_pos.first * size) + (by - chunk_pos.second * size)];
}

void ChunkLoader::SetChunkData(int cx, int cy, const char**& data)
{
	//storage sequence: 0,0 1,0 2,0 ... n,0 1,0 1,1 ...
	int size = 1 << DEFAULT_CHUNK_SIZE;
	TagByteArray* tag_data = (TagByteArray*)_get_chunk_data(cx, cy);
	if (!tag_data) return; //could not find tag, returned (this condition should be false commonly)
	char* raw_data = (char*)tag_data->GetDataRef();
	for (int i = 0; i < size; i++)
	{
		memcpy_s(raw_data + i * size, size, data[i], size);
	}
	//modify flag
	auto i1 = _loaded_data.begin();
	auto i2 = _last_access.begin();
	auto i3 = _data_changed.begin();
	for (; i1 != _loaded_data.end(); i1++, i2++, i3++)
	{
		if (*i1 == tag_data)
		{
			*i2 = time(nullptr);
			*i3 = true;
		}
	}
}

void ChunkLoader::SetBlockData(int bx, int by, char data)
{
	int size = 1 << DEFAULT_CHUNK_SIZE;
	auto chunk_pos = GetChunkPos(bx, by);
	TagByteArray* tag_data = (TagByteArray*)_get_chunk_data(chunk_pos.first, chunk_pos.second);
	if (!tag_data) return; //could not find tag, returned (this condition should be false commonly)

	char* raw_data = (char*)tag_data->GetDataRef();
	raw_data[size * (bx - chunk_pos.first * size) + (by - chunk_pos.second * size)] = data;
	//modify flag
	auto i1 = _loaded_data.begin();
	auto i2 = _last_access.begin();
	auto i3 = _data_changed.begin();
	for (; i1 != _loaded_data.end(); i1++, i2++, i3++)
	{
		if (*i1 == tag_data)
		{
			*i2 = time(nullptr);
			*i3 = true;
		}
	}
}

void ChunkLoader::GetBlockData(int bx1, int by1, int bx2, int by2, char**& data)
{
	if (bx1 > bx2)
		swap(bx1, bx2);
	if (by1 > by2)
		swap(by1, by2);
	int ylen = by2 - by1 + 1;
	int xlen = bx2 - bx1 + 1;
	if (xlen == 0 || ylen == 0) return;
	auto c1 = GetChunkPos(bx1, by1);
	auto c2 = GetChunkPos(bx2, by2);

	data = new char*[xlen];
	for (int i = 0; i < xlen; i++) data[i] = new char[ylen];

	int chunk_size = 1 << DEFAULT_CHUNK_SIZE;
	for (int x = c1.first; x <= c2.first; x++)
	{
		for (int y = c1.second; y <= c2.second; y++)
		{
			TagByteArray* tag_data = (TagByteArray*)_get_chunk_data(x, y);
			if (!tag_data) return;
			char* raw_data = (char*)tag_data->GetDataRef();

			//该区块的坐标数据
			int bx_min_from_cx = x * chunk_size;
			int by_min_from_cy = y * chunk_size;
			int bx_max_from_cx = (x + 1)*chunk_size - 1;
			int by_max_from_cy = (y + 1)*chunk_size - 1;

			int bx_min = bx_min_from_cx > bx1 ? bx_min_from_cx : bx1; //max(bx1, bx_min_from_cx)
			int bx_max = bx_max_from_cx < bx2 ? bx_max_from_cx : bx2; //min(bx2, bx_max_from_cx)
			int by_min = by_min_from_cy > by1 ? by_min_from_cy : by1;
			int by_max = by_max_from_cy < by2 ? by_max_from_cy : by2;

			//复制数据
			for (int t = bx_min; t <= bx_max; t++)
			{
				memcpy_s(
					data[t - bx1] + (by_min - by1),
					by_max - by_min + 1,
					raw_data + (t - bx_min_from_cx) * chunk_size + (by_min - by_min_from_cy),
					by_max - by_min + 1
				);
			}
		}
	}
}

void ChunkLoader::SetBlockData(int bx1, int by1, int bx2, int by2, const char **& data)
{
	if (bx1 > bx2)
		swap(bx1, bx2);
	if (by1 > by2)
		swap(by1, by2);
	int ylen = by2 - by1 + 1;
	int xlen = bx2 - bx1 + 1;
	if (xlen == 0 || ylen == 0) return;
	auto c1 = GetChunkPos(bx1, by1);
	auto c2 = GetChunkPos(bx2, by2);

	int chunk_size = 1 << DEFAULT_CHUNK_SIZE;

	for (int x = c1.first; x <= c2.first; x++)
	{
		for (int y = c1.second; y <= c2.second; y++)
		{
			TagByteArray* tag_data = (TagByteArray*)_get_chunk_data(x, y);
			if (!tag_data) return;
			char* raw_data = (char*)tag_data->GetDataRef();

			//该区块的坐标数据
			int bx_min_from_cx = x * chunk_size;
			int by_min_from_cy = y * chunk_size;
			int bx_max_from_cx = (x + 1)*chunk_size - 1;
			int by_max_from_cy = (y + 1)*chunk_size - 1;

			int bx_min = bx_min_from_cx > bx1 ? bx_min_from_cx : bx1; //max(bx1, bx_min_from_cx)
			int bx_max = bx_max_from_cx < bx2 ? bx_max_from_cx : bx2; //min(bx2, bx_max_from_cx)
			int by_min = by_min_from_cy > by1 ? by_min_from_cy : by1;
			int by_max = by_max_from_cy < by2 ? by_max_from_cy : by2;

			//复制数据
			for (int t = bx_min; t <= bx_max; t++)
			{
				memcpy_s(
					raw_data + (t - bx_min_from_cx) * chunk_size + (by_min - by_min_from_cy),
					by_max - by_min + 1,
					data[t - bx1] + (by_min - by1),
					by_max - by_min + 1
				);
			}

			//modify flag
			auto i1 = _loaded_data.begin();
			auto i2 = _last_access.begin();
			auto i3 = _data_changed.begin();
			for (; i1 != _loaded_data.end(); i1++, i2++, i3++)
			{
				if (*i1 == tag_data)
				{
					*i2 = time(nullptr);
					*i3 = true;
				}
			}
		}
	}
}

bool ChunkLoader::_chunk_generated(int chunk_x, int chunk_y) const
{
	//memory search
	for (auto i = _loaded_pos.begin(); i != _loaded_pos.end(); i++)
	{
		if (i->first == chunk_x && i->second == chunk_y)
			return true;
	}

	//file search
	char filename[512];
	int index = sprintf(filename, DEFAULT_SAVEDATA_PATH);
	filename[index - 1] = 0;
	if (access(filename, 6) == -1)
		mkdir(filename);
	filename[index - 1] = '\\';

	sprintf(filename + index, DEFAULT_SAVEDATA_FMT, chunk_x, chunk_y);

	if (access(filename, IO_MODE_EXIST) == -1)
		return false;

	return true;
}

Tag * ChunkLoader::_get_chunk_data(int chunk_x, int chunk_y)
{
	//memory search
	auto i1 = _loaded_pos.begin();
	auto i2 = _loaded_data.begin();
	for (; i1 != _loaded_pos.end(); i1++, i2++)
	{
		if (i1->first == chunk_x && i1->second == chunk_y)
			return *i2;
	}

	//file search
	Tag* file_data = _load_chunk_from_file(chunk_x, chunk_y);
	bool is_new = false;
	if (!file_data)
	{
		//not found, generate new data
		file_data = _generate_chunk_data(chunk_x, chunk_y);
		is_new = true;
	}

	//added to memory
	_add_to_memory(chunk_x, chunk_y, file_data, is_new);

	return file_data;
}

Tag * ChunkLoader::_generate_chunk_data(int chunk_x, int chunk_y)
{
	if (_callback_func)
	{
		Tag* tag = (*_callback_func)(_parent, chunk_x, chunk_y);
		return tag;
	}
	return nullptr;
}

Tag * ChunkLoader::_load_chunk_from_file(int chunk_x, int chunk_y)
{
	char filename[512];
	int index = sprintf(filename, DEFAULT_SAVEDATA_PATH);
	filename[index - 1] = 0;
	if (access(filename, IO_MODE_EXIST) == -1)
		mkdir(filename);
	filename[index - 1] = '\\';

	sprintf(filename + index, DEFAULT_SAVEDATA_FMT, chunk_x, chunk_y);

	if (access(filename, IO_MODE_READ) == -1) return nullptr; //file not exist

	ifstream ifs(filename, ios::binary | ios::in);
	//decompress mode
	ifs.seekg(0, ios::end);
	streamoff file_length = ifs.tellg();
	ifs.seekg(0, ios::beg);
	auto src_data = new char[file_length];
	ifs.read(src_data, file_length);
	ifs.close();
	uLong dst_length = 0x200000; //2mb
	int dst_increment = 0x100000; //1mb increment
	int code;
	auto dst_data = new char[dst_length];
	do
	{
		code = uncompress((Byte*)dst_data, (uLong*)&dst_length, (Byte*)src_data, file_length);
		if (code == Z_BUF_ERROR)
		{
			delete[] dst_data;
			dst_length += dst_increment;
			dst_data = new char[dst_length];
		}
		else if (code == Z_DATA_ERROR || code == Z_MEM_ERROR) //out of memory or data invalid
		{
			delete[] src_data;
			delete[] dst_data;
			return nullptr;
		}
	} while (code != Z_OK);

	stringstream ss;
	ss.write(dst_data, dst_length);

	delete[] src_data;
	delete[] dst_data;

	TagList* tag_list = (TagList*)Tag::ReadTagFromStream(ss);

	TagString* tag_version = (TagString*)(tag_list->GetData(string("version")));
	TagByteArray* tag_data = (TagByteArray*)(tag_list->GetData(string("chunkdata")));

	string* str_version = nullptr;
	tag_version->GetData((void*&)str_version);

	//这里可以插入存档版本的升级代码

	delete str_version;
	delete tag_version;
	delete tag_list;

	return tag_data;
}

void ChunkLoader::_write_chunk_to_file(int chunk_x, int chunk_y, Tag * data)
{
	char filename[512];
	int index = sprintf(filename, DEFAULT_SAVEDATA_PATH);
	filename[index - 1] = 0;
	if (access(filename, IO_MODE_EXIST) == -1)
		mkdir(filename);
	filename[index - 1] = '\\';

	sprintf(filename + index, DEFAULT_SAVEDATA_FMT, chunk_x, chunk_y);

	TagList* tag_list = new TagList("svd");
	TagString* tag_version = new TagString("version");
	string* str_version = new string(DATA_VERSION);
	tag_version->SetData((const void*&)str_version);
	TagByteArray* tag_data = (TagByteArray*)data;
	tag_data->SetName(string("chunkdata"));
	tag_list->AddData(tag_version);
	tag_list->AddData(tag_data);

	stringstream ss;
	Tag::WriteTagFromStream(ss, *tag_list);
	streamoff src_length = ss.tellp();
	auto src_data = new char[src_length];
	ss.read(src_data, src_length);

	auto dst_data = new char[src_length];
	uLong dst_length = (uLong)src_length;

	int code = compress((Byte*)dst_data, (uLongf*)&dst_length, (Byte*)src_data, src_length);
	delete[] src_data;
	if (code != Z_OK)
	{
		delete[] dst_data;
		return;
	}

	ofstream ofs(filename, ios::binary | ios::out);
	ofs.write(dst_data, dst_length);
	ofs.close();
	delete tag_version;
	delete str_version;
	delete tag_list;
	delete[] dst_data;
}

void ChunkLoader::_add_to_memory(int chunk_x, int chunk_y, Tag * data, bool data_changed)
{
	if (!data) return;
	if (_loaded_pos.size() >= DEFAULT_MAX_CHUNK_CACHE)
	{
		//iterators
		auto i1 = _loaded_pos.begin();
		auto i3 = _last_access.begin();

		pair<int, int> min_access_pos = *i1;
		time_t min_access_time = *i3;

		for (; i1 != _loaded_pos.end(); i1++, i3++)
		{
			if (*i3 < min_access_time)
			{
				min_access_pos = *i1;
				min_access_time = *i3;
			}
		}
		_unload_data(min_access_pos.first, min_access_pos.second);
	}

	_loaded_pos.push_back(pair<int, int>(chunk_x, chunk_y));
	_loaded_data.push_back(data);
	_data_changed.push_back(data_changed);
	_last_access.push_back(time(nullptr));
}

void ChunkLoader::_unload_data(int chunk_x, int chunk_y)
{
	auto i1 = _loaded_pos.begin();
	auto i2 = _loaded_data.begin();
	auto i3 = _last_access.begin();
	auto i4 = _data_changed.begin();

	for (; i1 != _loaded_pos.end(); i1++, i2++, i3++, i4++)
	{
		if (i1->first == chunk_x && i1->second == chunk_y)
			break;
	}

	if (i1 == _loaded_pos.end()) return;

	if (*i4)
	{
		_write_chunk_to_file(i1->first, i1->second, *i2);
	}
	delete *i2;
	_loaded_pos.erase(i1);
	_loaded_data.erase(i2);
	_last_access.erase(i3);
	_data_changed.erase(i4);
}
