#include "maploader.h"
#include <string>
#include <fstream>
#include <qpainter.h>

using namespace std;
Tag * MapLoader::_chunk_gen_cb(void* sender, int cx, int cy)
{
	int size = 1 << DEFAULT_CHUNK_SIZE;
	size *= size;
	char* data = debug_new char[size];
	for (int i = 0; i < size; i++)
	{
		int pos = rand() % 100;
		if (pos >= DEFAULT_GEN_MINE_POSSIBILITY)
			data[i] = 0x00;
		else
			data[i] = 0x10;
	}

	Tag* ret = debug_new TagByteArray(string("chunkdata"), data, size);
	debug_delete[] data;
	return ret;
}

void MapLoader::_load_config()
{
	char filename[512];
	int index = sprintf(filename, DEFAULT_SAVEDATA_PATH);
	filename[index - 1] = 0;
	if (access(filename, IO_MODE_EXIST) == -1)
		mkdir(filename);
	filename[index - 1] = '\\';

	sprintf(filename + index, DEFAULT_SAVEDATA_SETTING_FMT);

	if (access(filename, IO_MODE_READ) == -1) return; //file not exist

	ifstream ifs(filename, ios::binary | ios::in);
	TagList* tag_list = (TagList*)Tag::ReadTagFromStream(ifs);

	TagString* tag_version = (TagString*)(tag_list->GetData(string("version")));
	TagDouble* px = (TagDouble*)(tag_list->GetData(string("position-x")));
	TagDouble* py = (TagDouble*)(tag_list->GetData(string("position-y")));
	TagInt* block_size = (TagInt*)(tag_list->GetData(string("block-size")));
	string* str_version = nullptr;
	tag_version->GetData((void*&)str_version);

	//这里可以插入存档版本的升级代码

	debug_delete str_version;
	debug_delete tag_version;

	double* x;
	double* y;
	px->GetData((void*&)x);
	py->GetData((void*&)y);
	_location = QPointF(*x, *y);

	debug_delete x;
	debug_delete y;
	debug_delete px;
	debug_delete py;

	double* bs;
	block_size->GetData((void*&)bs);
	_block_size = *bs;
	debug_delete bs;
	debug_delete block_size;

	debug_delete tag_list;

	ifs.close();
}

void MapLoader::_save_config()
{
	char filename[512];
	int index = sprintf(filename, DEFAULT_SAVEDATA_PATH);
	filename[index - 1] = 0;
	if (access(filename, IO_MODE_EXIST) == -1)
		mkdir(filename);
	filename[index - 1] = '\\';

	sprintf(filename + index, DEFAULT_SAVEDATA_SETTING_FMT);

	ofstream ofs(filename, ios::binary | ios::out);
	TagList* tag_list = debug_new TagList("settings");

	TagString* tag_version = debug_new TagString(string("version"), string("1.0.0"));
	TagDouble* px = debug_new TagDouble(string("position-x"), _location.x());
	TagDouble* py = debug_new TagDouble(string("position-y"), _location.y());
	TagDouble* block_size = debug_new TagDouble(string("block-size"), _block_size);

	tag_list->AddData(tag_version);
	tag_list->AddData(px);
	tag_list->AddData(py);
	tag_list->AddData(block_size);

	debug_delete tag_version;
	debug_delete px;
	debug_delete py;
	debug_delete block_size;

	Tag::WriteTagFromStream(ofs, *tag_list);
	debug_delete tag_list;

	ofs.close();
}

bool MapLoader::_is_gen_mask(int cx, int cy)
{
	return (_cl->GetBlockData(cx << DEFAULT_CHUNK_SIZE, cy << DEFAULT_CHUNK_SIZE) & 0x80) != 0;
}

void MapLoader::_gen_mask(int cx, int cy)
{
	//加载上下左右的数据
	int size = (1 << DEFAULT_CHUNK_SIZE) + 2;
	char** data = nullptr;
	_cl->GetBlockData((cx << DEFAULT_CHUNK_SIZE) - 1, (cy << DEFAULT_CHUNK_SIZE) - 1, (cx + 1) << DEFAULT_CHUNK_SIZE, (cy + 1) << DEFAULT_CHUNK_SIZE, data);

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			if (data[x][y] & 0x10)
			{
				if (x - 1 > 0 && y - 1 > 0)data[x - 1][y - 1]++;
				if (x - 1 > 0) data[x - 1][y]++;
				if (x - 1 > 0 && y + 1 < size - 1)data[x - 1][y + 1]++;
				if (y - 1 > 0) data[x][y - 1]++;
				if (y + 1 < size - 1) data[x][y + 1]++;
				if (x + 1 < size - 1 && y - 1 > 0)data[x + 1][y - 1]++;
				if (x + 1 < size - 1) data[x + 1][y]++;
				if (x + 1 < size - 1 && y + 1 < size - 1)data[x + 1][y + 1]++;
			}
		}
	}

	for (int x = 1; x < size - 1; x++)
	{
		for (int y = 1; y < size - 1; y++)
		{
			data[x][y] |= 0x80;
		}
	}

	_cl->SetBlockData((cx << DEFAULT_CHUNK_SIZE) - 1, (cy << DEFAULT_CHUNK_SIZE) - 1, (cx + 1) << DEFAULT_CHUNK_SIZE, (cy + 1) << DEFAULT_CHUNK_SIZE, (const char**&)data);

	for (int i = 0; i < size; i++)
		debug_delete[] data[i];
	debug_delete[] data;
}

void MapLoader::_load_map(char **& data)
{
	QPointF pf1(_location), pf2(_location.x() + _width, _location.y() + _height);
	QPoint p1((int)floor(pf1.x()), (int)floor(pf1.y())), p2((int)ceil(pf2.x()), (int)ceil(pf2.y()));
	auto c1 = _cl->GetChunkPos(p1.x(), p1.y());
	auto c2 = _cl->GetChunkPos(p2.x(), p2.y());
	for (int x = c1.first; x <= c2.first; x++)
	{
		for (int y = c1.second; y <= c2.second; y++)
		{
			if (!_is_gen_mask(x, y))
				_gen_mask(x, y);
		}
	}
	_cl->GetBlockData(p1.x(), p1.y(), p2.x(), p2.y(), data);
}

QPoint MapLoader::_translate_pos(QPointF block)
{
	double x = (block.x() - _location.x())*_block_size;
	double y = (block.y() - _location.y())*_block_size;
	return QPoint((int)x, (int)y);
}

MapLoader::MapLoader()
{
	_cl = debug_new ChunkLoader(_chunk_gen_cb, this);
	_location = QPointF(0, 0);
	_block_size = 16;
	_load_config();
}

MapLoader::~MapLoader()
{
	if (_cl) debug_delete _cl;
	_cl = nullptr;
}

void MapLoader::RenderMap(QPainter& p, QWidget* parent)
{
	char** data = nullptr;
	_load_map(data);
	int x1 = floor(_location.x());
	int x2 = ceil(_location.x() + _width);
	int y1 = floor(_location.y());
	int y2 = ceil(_location.y() + _height);
	int dx = x2 - x1;
	int dy = y2 - y1;

	p.setPen(Qt::black);
	p.setFont(QFont("SimSun", 9));

	QTextOption opt(Qt::AlignHCenter | Qt::AlignVCenter);
	char buf[5];
	for (int x = 0; x <= dx; x++)
	{
		for (int y = 0; y <= dy; y++)
		{
			QPoint pos = _translate_pos(QPointF(x1 + x, y1 + y));
			p.drawRect(pos.x(), pos.y(), _block_size, _block_size);

			if (data[x][y] & 0x10)
			{
				strcpy_s(buf, "*");
			}
			else if (data[x][y] & 0x0f)
			{
				sprintf(buf, "%d", data[x][y] & 0x0f);
			}
			else
			{
				continue;
			}

			p.drawText(QRect(pos.x(), pos.y(), _block_size - 1, _block_size - 1), buf, opt);
		}
	}

	for (int i = 0; i <= dx; i++)
		debug_delete[] data[i];
	debug_delete[] data;
}