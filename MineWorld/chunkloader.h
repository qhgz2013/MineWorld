#pragma once
#include <list>
#include <ctime>
#include "mineworld-const.h"
#include "nbt.h"

typedef Tag* (*ChunkGenCallback)(void* sender, int chunk_x, int chunk_y);
extern const char* DATA_VERSION;
extern const int IO_MODE_EXIST;
extern const int IO_MODE_WRITE;
extern const int IO_MODE_READ;
extern const int IO_MODE_READWRITE;

//2άchar�����������/����
class ChunkLoader
{
	//char** data: data[x][y]
public:
	//��������������ʱִ�еĻص�����
	ChunkLoader(ChunkGenCallback cbf, void* parent = nullptr) : _callback_func(cbf), _parent(parent) {}
	~ChunkLoader();
	//��ȡ�����������ݣ���data���أ�ָ����Զ���������Ҫ���ɾ����
	void GetChunkData(int cx, int cy, char**& data);
	//��ȡָ����������
	char GetBlockData(int bx, int by);
	//���������������ݣ�ע�⣺data�����鳤�Ȳ���С��2^DEFAULT_CHUNK_SIZE ������ڴ汨��
	void SetChunkData(int cx, int cy, const char**& data);
	//����ָ����������
	void SetBlockData(int bx, int by, char data);
	//��ȡ������Ϊbx1��bx2��������Ϊby1��by2�ķ�������
	void GetBlockData(int bx1, int by1, int bx2, int by2, char**& data);
	//���ú�����Ϊbx1��bx2��������Ϊby1��by2�ķ�������
	void SetBlockData(int bx1, int by1, int bx2, int by2, const char**& data);

	//��ȡָ�������µ�����λ��
	static inline std::pair<int, int> GetChunkPos(int block_x, int block_y)
	{
		std::pair<int, int> ret_val = { block_x >> DEFAULT_CHUNK_SIZE, block_y >> DEFAULT_CHUNK_SIZE };
		return ret_val;
	}
private:
	//���ص�����
	std::list<std::pair<int, int>> _loaded_pos;
	//���ص���������
	std::list<Tag*> _loaded_data;
	//�����Ƿ��޸�
	std::list<bool> _data_changed;
	//����������ʱ�䣬���ڵ��ȶ���
	std::list<time_t> _last_access;
	//��������ʱִ�еĻص�����
	ChunkGenCallback _callback_func;
	void* _parent;

	//��ȡָ�������Ƿ񴴽�
	bool _chunk_generated(int chunk_x, int chunk_y) const;
	//��ȡ�������ݣ���ӵ����棩
	Tag* _get_chunk_data(int chunk_x, int chunk_y);
	//������������
	Tag* _generate_chunk_data(int chunk_x, int chunk_y);
	//���ļ��ж�ȡ����
	Tag* _load_chunk_from_file(int chunk_x, int chunk_y);
	//���ļ���д������
	void _write_chunk_to_file(int chunk_x, int chunk_y, Tag* data);
	//�������ݵ��ڴ���
	void _add_to_memory(int chunk_x, int chunk_y, Tag* data, bool data_changed = false);
	//���ڴ����Ƴ�����
	void _unload_data(int chunk_x, int chunk_y);
};