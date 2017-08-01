#pragma once
#include <list>
#include "mineworld-const.h"
#include "nbt.h"

typedef Tag* (*ChunkGenCallback)(int chunk_x, int chunk_y);

//2维char数组区块加载/缓存
class ChunkLoader
{
public:
	ChunkLoader();
	~ChunkLoader();
	void GetChunkData(int cx, int cy, char* data[]) const;
	char GetBlockData(int bx, int by) const;
	void SetChunkData(int cx, int cy, const char* data[]);
	void SetBlockData(int bx, int by, char data);

private:
	//loaded chunk
	std::list<std::pair<int, int>> _loaded_pos;
	std::list<Tag*> _loaded_data;
	std::list<bool> _data_changed;
	ChunkGenCallback _callback_func;

	//获取指定坐标下的区块位置
	static inline std::pair<int, int> _get_chunk_pos(int block_x, int block_y)
	{
		std::pair<int, int> ret_val = { block_x >> DEFAULT_CHUNK_SIZE, block_y >> DEFAULT_CHUNK_SIZE };
		return ret_val;
	}
	//获取指定区块是否创建
	bool _chunk_generated(int chunk_x, int chunk_y) const;
	//获取区块数据（添加到缓存）
	Tag* _get_chunk_data(int chunk_x, int chunk_y);
	//生成区块数据
	Tag* _generate_chunk_data(int chunk_x, int chunk_y);
	//从文件中读取数据
	Tag* _load_chunk_from_file(int chunk_x, int chunk_y);
	//从文件中写入数据
	void _write_chunk_to_file(int chunk_x, int chunk_y);
	
};