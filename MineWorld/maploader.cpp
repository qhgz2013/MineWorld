#include "maploader.h"
#include <string>
#include <fstream>
#include <qpainter.h>
#include <qlist.h>
#include <qfuture.h>
#include <QtConcurrent\qtconcurrentmap.h>
#include "util.h"
using namespace std;

double MapLoader::_animation_duration = 0.3;

QImage MapLoader::_render_thumbnail(structA code)
{
	int size = (int)code.sender->_block_size;
	QImage img(size, size, QImage::Format::Format_ARGB32);
	QPainter p(&img);
	p.fillRect(QRect(0, 0, size, size), QColor(245, 245, 245));
	//unknown marker
	if ((code.code & 0x60) == 0x60)
	{
		QBrush normal(QColor(99, 120, 213));
		QBrush highlight(QColor(200, 210, 249));
		QBrush shadow(QColor(64, 72, 104));
		p.fillRect(0, 0, size, size, normal);
		if (size > 4)
		{
			QPainterPath path1, path2;
			path1.moveTo(0, 0);
			path1.lineTo(size - 1, 0);
			path1.lineTo(size - 2, 1);
			path1.lineTo(1, 1);
			path1.lineTo(1, size - 2);
			path1.lineTo(0, size - 1);
			path1.lineTo(0, 0);
			path2.moveTo(size - 1, 0);
			path2.lineTo(size - 1, size - 1);
			path2.lineTo(0, size - 1);
			path2.lineTo(1, size - 2);
			path2.lineTo(size - 2, size - 2);
			path2.lineTo(size - 2, 1);
			path2.lineTo(size - 1, 0);
			p.fillPath(path2, shadow);
			p.fillPath(path1, highlight);
		}
		p.setFont(QFont("SimSun", 9));
		p.setPen(Qt::black);
		QTextOption opt(Qt::AlignVCenter | Qt::AlignHCenter);
		char buf[5] = "?";
		p.drawText(QRect(0, 0, size, size), buf, opt);
		p.setPen(Qt::lightGray);
	}
	//clicked
	else if (code.code & 0x20)
	{
		//mine
		if (code.code & 0x10)
		{
			p.drawImage(QRect(0, 0, size, size), *code.sender->_mine_icon);
			p.setPen(Qt::darkGray);
			p.drawLine(0, 0, size - 1, 0);
			p.drawLine(0, 0, 0, size - 1);
		}
		//common block
		else if ((code.code & 0x0f) > 0 && (code.code & 0x0f) < 9)
		{
			p.setFont(QFont("SimSun", 9));
			p.setPen(Qt::black);
			QTextOption opt(Qt::AlignVCenter | Qt::AlignHCenter);
			char buf[5];
			sprintf_s(buf, "%d", code.code & 0x0f);
			p.drawText(QRect(0, 0, size, size), buf, opt);
			p.setPen(Qt::lightGray);
			p.drawLine(0, 0, size - 1, 0);
			p.drawLine(0, 0, 0, size - 1);
		}
		//empty block
		else if ((code.code & 0x0f) == 0)
		{
			p.setPen(Qt::lightGray);
			p.drawLine(0, 0, size - 1, 0);
			p.drawLine(0, 0, 0, size - 1);
		}
	}
	//flag marker
	else if (code.code & 0x40)
	{
		QBrush normal(QColor(99, 120, 213));
		QBrush highlight(QColor(200, 210, 249));
		QBrush shadow(QColor(64, 72, 104));
		p.fillRect(0, 0, size, size, normal);
		if (size > 4)
		{
			QPainterPath path1, path2;
			path1.moveTo(0, 0);
			path1.lineTo(size - 1, 0);
			path1.lineTo(size - 2, 1);
			path1.lineTo(1, 1);
			path1.lineTo(1, size - 2);
			path1.lineTo(0, size - 1);
			path1.lineTo(0, 0);
			path2.moveTo(size - 1, 0);
			path2.lineTo(size - 1, size - 1);
			path2.lineTo(0, size - 1);
			path2.lineTo(1, size - 2);
			path2.lineTo(size - 2, size - 2);
			path2.lineTo(size - 2, 1);
			path2.lineTo(size - 1, 0);
			p.fillPath(path2, shadow);
			p.fillPath(path1, highlight);
		}
		p.setFont(QFont("SimSun", 9));
		p.setPen(Qt::black);
		QTextOption opt(Qt::AlignVCenter | Qt::AlignHCenter);
		char buf[5] = "><";
		p.drawText(QRect(0, 0, size, size), buf, opt);
		p.setPen(Qt::lightGray);
	}
	//unclicked (including mine
	else
	{
		QBrush normal(QColor(99, 120, 213));
		QBrush highlight(QColor(200, 210, 249));
		QBrush shadow(QColor(64, 72, 104));
		p.fillRect(0, 0, size, size, normal);
		if (size > 4)
		{
			QPainterPath path1, path2;
			path1.moveTo(0, 0);
			path1.lineTo(size - 1, 0);
			path1.lineTo(size - 2, 1);
			path1.lineTo(1, 1);
			path1.lineTo(1, size - 2);
			path1.lineTo(0, size - 1);
			path1.lineTo(0, 0);
			path2.moveTo(size - 1, 0);
			path2.lineTo(size - 1, size - 1);
			path2.lineTo(0, size - 1);
			path2.lineTo(1, size - 2);
			path2.lineTo(size - 2, size - 2);
			path2.lineTo(size - 2, 1);
			path2.lineTo(size - 1, 0);
			p.fillPath(path2, shadow);
			p.fillPath(path1, highlight);
		}
	}
	return img;
}

Tag * MapLoader::_chunk_gen_cb(void* sender, int cx, int cy)
{
	int size = 1 << DEFAULT_CHUNK_SIZE;
	size *= size;
	char* data = debug_new char[size];
	//char structure:
	//
	// Generated Mask Data
	// | Flag at this block      -+
	// | | This block is clicked -+-> 1 1 represents that this is marked as unknown
	// | | | This block is mine
	// v v v v
	// 0 0 0 0  |  0 0 0 0  <- Mask data (4 bit)
	// flags    |  data
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
	QPointF pf1(_location), pf2((_location.x() + _width) / _block_size, (_location.y() + _height) / _block_size);
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
	_thumbnail_cache = nullptr;
	_mine_icon = debug_new QImage(":/MineWorld/icon.png");
	_width = 0;
	_height = 0;
	_load_config();
}

MapLoader::~MapLoader()
{
	if (_cl) debug_delete _cl;
	_cl = nullptr;
	if (_mine_icon) debug_delete _mine_icon;
	_mine_icon = nullptr;
	if (_thumbnail_cache)
	{
		for (int i = 0; i < 256; i++)
			debug_delete _thumbnail_cache[i];
		debug_delete[] _thumbnail_cache;
	}
	_thumbnail_cache = nullptr;
}

void MapLoader::renderMap(QPainter& p, QWidget* form)
{
	if (!_thumbnail_cache)
	{
		QList<structA> ls;
		for (int i = 0; i < 256; i++)
			ls.append(structA{ this, (char)i });
		QFuture<QImage> task = QtConcurrent::mapped(ls, _render_thumbnail);
		task.waitForFinished();

		_thumbnail_cache = debug_new QImage*[256];
		auto i1 = ls.begin();
		auto i2 = task.begin();
		for (; i1 != ls.end(); i1++, i2++)
			_thumbnail_cache[(unsigned char)i1->code] = debug_new QImage(*i2);
	}

	//merging map

	int x1 = floor(_location.x());
	int x2 = ceil((_location.x() + _width) / _block_size);
	int y1 = floor(_location.y());
	int y2 = ceil((_location.y() + _height) / _block_size);
	int dx = x2 - x1;
	int dy = y2 - y1;
	char** data = nullptr;
	_load_map(data);

	for (int x = 0; x <= dx; x++)
	{
		for (int y = 0; y <= dy; y++)
		{
			QImage* img = _thumbnail_cache[(data[x][y] & 0x7f)];
			if (!img)
			{
				int test = 0;
			}
			QPoint pos = _translate_pos(QPointF(x, y));
			p.drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
		}
	}
	for (int i = 0; i <= dx; i++)
		debug_delete[] data[i];
	debug_delete[] data;

}

void MapLoader::renderAnimation(QPainter& p, QWidget* form)
{
	auto i1 = _start_time.begin();
	auto i2 = _affect_block.begin();
	auto i3 = _type.begin();
	int size = (int)_block_size;
	
	//todo: using form->update() to render animation
}

void MapLoader::enterBlock(QPoint block)
{
	if (block.x() >= _location.x() + _width / _block_size || block.y() >= _location.y() + _height / _block_size) return;
	auto i1 = _start_time.begin();
	auto i2 = _affect_block.begin();
	auto i3 = _type.begin();
	for (; i2 != _affect_block.end(); i1++, i2++, i3++)
	{
		if (*i2 == block)
		{
			_start_time.erase(i1);
			_affect_block.erase(i2);
			_type.erase(i3);
		}
	}
	double curtime = fGetCurrentTimestamp();
	_start_time.push_back(curtime);
	_affect_block.push_back(block);
	_type.push_back(animationType::Enter);
}

void MapLoader::leaveBlock(QPoint block)
{
	if (block.x() >= _location.x() + _width / _block_size || block.y() >= _location.y() + _height / _block_size) return;
	auto i1 = _start_time.begin();
	auto i2 = _affect_block.begin();
	auto i3 = _type.begin();
	for (; i2 != _affect_block.end(); i1++, i2++, i3++)
	{
		if (*i2 == block)
		{
			_start_time.erase(i1);
			_affect_block.erase(i2);
			_type.erase(i3);
		}
	}
	double curtime = fGetCurrentTimestamp();
	_start_time.push_back(curtime);
	_affect_block.push_back(block);
	_type.push_back(animationType::Leave);
}

void MapLoader::clickBlock(QPoint block)
{
	if (block.x() >= _location.x() + _width / _block_size || block.y() >= _location.y() + _height / _block_size) return;
	auto i1 = _start_time.begin();
	auto i2 = _affect_block.begin();
	auto i3 = _type.begin();
	for (; i2 != _affect_block.end(); i1++, i2++, i3++)
	{
		if (*i2 == block)
		{
			_start_time.erase(i1);
			_affect_block.erase(i2);
			_type.erase(i3);
		}
	}
	double curtime = fGetCurrentTimestamp();
	_start_time.push_back(curtime);
	_affect_block.push_back(block);
	_type.push_back(animationType::Click);

	//todo: bfs recursive click
}