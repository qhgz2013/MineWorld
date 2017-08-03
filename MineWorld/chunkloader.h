#pragma once
#include <list>
#include <ctime>
#include "mineworld-const.h"
#include "nbt.h"

typedef Tag* (*ChunkGenCallback)(int chunk_x, int chunk_y);
extern const char* DATA_VERSION;

//2维char数组区块加载/缓存
class ChunkLoader
{
public:
	//参数：区块生成时执行的回调函数
	ChunkLoader(ChunkGenCallback cbf) : _callback_func(cbf) {}
	~ChunkLoader();
	//获取整个区块数据，由data返回（指针会自动创建，需要逐个删除），注意：data的数组长度不能小于2^DEFAULT_CHUNK_SIZE 否则会内存报错
	void GetChunkData(int cx, int cy, char* data[]);
	//获取指定方块数据
	char GetBlockData(int bx, int by);
	//设置整个区块数据（注意：data的数组长度不能小于2^DEFAULT_CHUNK_SIZE 否则会内存报错）
	void SetChunkData(int cx, int cy, const char* data[]);
	//设置指定方块数据
	void SetBlockData(int bx, int by, char data);

private:
	//加载的坐标
	std::list<std::pair<int, int>> _loaded_pos;
	//加载的区块数据
	std::list<Tag*> _loaded_data;
	//区块是否被修改
	std::list<bool> _data_changed;
	//区块最后访问时间，用于调度而已
	std::list<time_t> _last_access;
	//区块生成时执行的回调函数
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
	void _write_chunk_to_file(int chunk_x, int chunk_y, Tag* data);
	//加载数据到内存中
	void _add_to_memory(int chunk_x, int chunk_y, Tag* data, bool data_changed = false);
	//从内存中移除数据
	void _unload_data(int chunk_x, int chunk_y);
};