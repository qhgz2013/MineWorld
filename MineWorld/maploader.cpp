#include "maploader.h"
#include <string>
#include <fstream>
#include <qpainter.h>
#include <qfuture.h>
#include <QtConcurrent\qtconcurrentmap.h>
#include <Windows.h>
#include <list>
using namespace std;

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
		p.setFont(QFont("Segoe UI Black", 10));
		p.setPen(Qt::black);
		QTextOption opt(Qt::AlignVCenter | Qt::AlignHCenter);
		p.drawText(QRect(0, 0, size, size), QStringLiteral("？"), opt);
		p.setPen(Qt::lightGray);
	}
	//clicked
	else if (code.code & 0x20)
	{
		//mine
		if (code.code & 0x10)
		{
			p.drawImage(QRect(1, 1, size - 1, size - 1), *code.sender->_mine_icon);
			p.setPen(Qt::darkGray);
			p.drawLine(0, 0, size - 1, 0);
			p.drawLine(0, 0, 0, size - 1);
		}
		//common block
		else if ((code.code & 0x0f) > 0 && (code.code & 0x0f) < 9)
		{
			p.setFont(QFont("Segoe UI Black", 11));
			const QColor color[] = {
				QColor(0,0,0),
				QColor(0,0,255),
				QColor(0,129,0),
				QColor(255,0,0),
				QColor(0,0,129),
				QColor(129,0,0),
				QColor(0,129,129),
				QColor(0,0,0),
				QColor(129,129,129)
			};
			p.setPen(color[code.code & 0x0f]);
			QTextOption opt(Qt::AlignVCenter | Qt::AlignHCenter);
			char buf[5];
			sprintf_s(buf, "%d", code.code & 0x0f);
			p.drawText(QRect(1, 1, size - 1, size - 1), buf, opt);
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
		p.drawImage(QRect(2, 2, size - 3, size - 3), *code.sender->_flag_icon);
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
		if (pos >= GEN_MINE_POSSIBILITY)
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

	Tag* cur_tag = nullptr;
	TagString* tag_version = nullptr;
	TagDouble* tag_px = nullptr;
	TagDouble* tag_py = nullptr;
	TagDouble* tag_block_size = nullptr;
	TagInt* tag_possibility = nullptr;
	TagULong* tag_cleared_block = nullptr;
	TagULong* tag_flag_made = nullptr;
	TagULong* tag_unknown_made = nullptr;
	TagULong* tag_mine_clicked = nullptr;
	TagDouble* tag_time_played = nullptr;
#define _chk_tag(x, name, type) cur_tag = (tag_list->GetData(string(name))); if (cur_tag) x = (type*)cur_tag
	//cur_tag = (tag_list->GetData(string("version")));
	//if (cur_tag) tag_version = (TagString*)cur_tag;
	//TagDouble* px = (TagDouble*)(tag_list->GetData(string("position-x")));
	//TagDouble* py = (TagDouble*)(tag_list->GetData(string("position-y")));
	//TagDouble* block_size = (TagDouble*)(tag_list->GetData(string("block-size")));
	//TagInt* possibility = (TagInt*)(tag_list->GetData(string("possibility")));
	_chk_tag(tag_version, "version", TagString);
	_chk_tag(tag_px, "position-x", TagDouble);
	_chk_tag(tag_py, "position-y", TagDouble);
	_chk_tag(tag_block_size, "block-size", TagDouble);
	_chk_tag(tag_possibility, "possibility", TagInt);
	_chk_tag(tag_cleared_block, "cleared-block", TagULong);
	_chk_tag(tag_flag_made, "flag-made", TagULong);
	_chk_tag(tag_unknown_made, "unknown-made", TagULong);
	_chk_tag(tag_mine_clicked, "mine-clicked", TagULong);
	_chk_tag(tag_time_played, "time-played", TagDouble);
#undef _chk_tag

	string* str_version = nullptr;
	if (tag_version) tag_version->GetData((void*&)str_version);

	//这里可以插入存档版本的升级代码

	if (str_version) debug_delete str_version;
	if (tag_version) debug_delete tag_version;

	double* x = nullptr;
	double* y = nullptr;
	if (tag_px) tag_px->GetData((void*&)x);
	if (tag_py) tag_py->GetData((void*&)y);
	if (!x) x = debug_new double(0);
	if (!y) y = debug_new double(0);
	_location = QPointF(*x, *y);

	debug_delete x;
	debug_delete y;
	if (tag_px) debug_delete tag_px;
	if (tag_py) debug_delete tag_py;

	int* possibility = nullptr;
	if (tag_possibility) tag_possibility->GetData((void*&)possibility);
	if (possibility) _possibility = *possibility;
	else _possibility = GEN_MINE_POSSIBILITY;

	if (possibility) debug_delete possibility;
	if (tag_possibility) debug_delete tag_possibility;

	double* block_size = nullptr;
	if (tag_block_size) tag_block_size->GetData((void*&)block_size);
	if (block_size) _block_size = *block_size;
	if (block_size) debug_delete block_size;
	if (tag_block_size) debug_delete tag_block_size;

	uint64_t* cleared_block = nullptr;
	if (tag_cleared_block) tag_cleared_block->GetData((void*&)cleared_block);
	if (cleared_block) _cleared_block = *cleared_block;
	if (cleared_block) debug_delete cleared_block;
	if (tag_cleared_block) debug_delete tag_cleared_block;

	uint64_t* flag_made = nullptr;
	if (tag_flag_made) tag_flag_made->GetData((void*&)flag_made);
	if (flag_made) _flag_made = *flag_made;
	if (flag_made) debug_delete flag_made;
	if (tag_flag_made) debug_delete tag_flag_made;

	uint64_t* unknown_made = nullptr;
	if (tag_unknown_made) tag_unknown_made->GetData((void*&)unknown_made);
	if (unknown_made) _unknown_made = *unknown_made;
	if (unknown_made) debug_delete unknown_made;
	if (tag_unknown_made) debug_delete tag_unknown_made;

	uint64_t* mine_clicked = nullptr;
	if (tag_mine_clicked) tag_mine_clicked->GetData((void*&)mine_clicked);
	if (mine_clicked) _mine_clicked = *mine_clicked;
	if (mine_clicked) debug_delete mine_clicked;
	if (tag_mine_clicked) debug_delete tag_mine_clicked;

	double* time_played = nullptr;
	if (tag_time_played) tag_time_played->GetData((void*&)time_played);
	if (time_played) _time_played = *time_played;
	if (time_played) debug_delete time_played;
	if (tag_time_played) debug_delete tag_time_played;

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
	TagInt* possibility = debug_new TagInt(string("possibility"), _possibility);
	TagULong* cleared_block = debug_new TagULong(string("cleared-block"), _cleared_block);
	TagULong* flag_made = debug_new TagULong(string("flag-made"), _flag_made);
	TagULong* unknown_made = debug_new TagULong(string("unknown-made"), _unknown_made);
	TagULong* mine_clicked = debug_new TagULong(string("mine-clicked"), _mine_clicked);
	TagDouble* time_played = debug_new TagDouble(string("time-played"), _time_played);

	tag_list->AddData(tag_version);
	tag_list->AddData(px);
	tag_list->AddData(py);
	tag_list->AddData(block_size);
	tag_list->AddData(possibility);
	tag_list->AddData(cleared_block);
	tag_list->AddData(flag_made);
	tag_list->AddData(unknown_made);
	tag_list->AddData(mine_clicked);
	tag_list->AddData(time_played);

	debug_delete tag_version;
	debug_delete px;
	debug_delete py;
	debug_delete block_size;
	debug_delete possibility;
	debug_delete cleared_block;
	debug_delete flag_made;
	debug_delete unknown_made;
	debug_delete mine_clicked;
	debug_delete time_played;

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
				if (x - 1 > 0 && y > 0 && y < size - 1) data[x - 1][y]++;
				if (x - 1 > 0 && y + 1 < size - 1)data[x - 1][y + 1]++;
				if (x > 0 && x < size - 1 && y - 1 > 0) data[x][y - 1]++;
				if (x > 0 && x < size - 1 && y + 1 < size - 1) data[x][y + 1]++;
				if (x + 1 < size - 1 && y - 1 > 0)data[x + 1][y - 1]++;
				if (x + 1 < size - 1 && y > 0 && y < size - 1) data[x + 1][y]++;
				if (x + 1 < size - 1 && y + 1 < size - 1)data[x + 1][y + 1]++;
				if (x > 0 && x < size - 1 && y > 0 && y < size - 1) data[x][y]++; //test only
			}
		}
	}
	//validating code
	for (int x = 1; x < size - 1; x++)
	{
		for (int y = 1; y < size - 1; y++)
		{
			char expected_data = data[x][y] & 0x0f;
			char actual_data = 0;
			if (data[x - 1][y - 1] & 0x10) actual_data++;
			if (data[x - 1][y] & 0x10) actual_data++;
			if (data[x - 1][y + 1] & 0x10) actual_data++;
			if (data[x][y - 1] & 0x10) actual_data++;
			if (data[x][y] & 0x10) actual_data++;
			if (data[x][y + 1] & 0x10) actual_data++;
			if (data[x + 1][y - 1] & 0x10) actual_data++;
			if (data[x + 1][y] & 0x10) actual_data++;
			if (data[x + 1][y + 1] & 0x10) actual_data++;

			if (actual_data != expected_data)
			{
				char buf[256];
				sprintf_s(buf, "Generator error: validate failed at block (%d,%d)\r\n", x, y);
				OutputDebugStringA(buf);
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

void MapLoader::_load_map(char **& data, QPointF& pt)
{
	QPointF pf1(pt), pf2(pt.x() + _width / _block_size, pt.y() + _height / _block_size);
	QPoint p1((int)floor(pf1.x()), (int)floor(pf1.y())), p2((int)floor(pf2.x()), (int)floor(pf2.y()));
	auto c1 = _cl->GetChunkPos(p1.x(), p1.y());
	auto c2 = _cl->GetChunkPos(p2.x(), p2.y());
	//char buf[140];
	//sprintf_s(buf, "load block (%d,%d) to (%d,%d) / origin:(%f,%f) to (%f,%f)\r\n", p1.x(), p1.y(), p2.x(), p2.y(), pf1.x(), pf1.y(), pf2.x(), pf2.y());
	//OutputDebugStringA(buf);
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

QPointF MapLoader::_translate_pos(QPointF& block, QPointF& left_top)
{
	double x = (block.x() - left_top.x())*_block_size;
	double y = (block.y() - left_top.y())*_block_size;
	return QPointF(x, y);
}

void MapLoader::_draw_block(QPoint block, char data, QPainter & p)
{
	QPointF pos = _translate_pos(QPointF(block), _cache_location);
	QImage* img = _thumbnail_cache[data & 0x7f];
	p.drawImage(QRect(floor(pos.x()), floor(pos.y()), _block_size, _block_size), *img);
}

MapLoader::MapLoader(int possibility)
{
	_cl = debug_new ChunkLoader(_chunk_gen_cb, this);
	_location = QPointF(0, 0);
	_block_size = 16;
	_thumbnail_cache = nullptr;
	_mine_icon = debug_new QImage(":/MineWorld/icon.png");
	_flag_icon = debug_new QImage(":/MineWorld/icon2.png");
	_width = 0;
	_height = 0;
	_cache_map = nullptr;
	_cache_location = _location;
	_cache_width = 0;
	_cache_height = 0;
	_possibility = possibility;
	_cleared_block = 0;
	_flag_made = 0;
	_unknown_made = 0;
	_mine_clicked = 0;
	_time_played = 0;
	_ctor_time = fGetCurrentTimestamp();
	_load_config();
}

MapLoader::~MapLoader()
{
	_time_played += fGetCurrentTimestamp() - _ctor_time;
	_save_config();
	if (_cl) debug_delete _cl;
	_cl = nullptr;
	if (_mine_icon) debug_delete _mine_icon;
	_mine_icon = nullptr;
	if (_flag_icon) debug_delete _flag_icon;
	_flag_icon = nullptr;
	if (_thumbnail_cache)
	{
		for (int i = 0; i < 256; i++)
			debug_delete _thumbnail_cache[i];
		debug_delete[] _thumbnail_cache;
	}
	_thumbnail_cache = nullptr;
	if (_cache_map) debug_delete _cache_map;
	_cache_map = nullptr;
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

	//这里改成先在cache_image绘图，再复制到窗体
	char** data = nullptr;
#pragma region cache resize
	if (_cache_width != _width || _cache_height != _height)
	{
		int x1, x2, y1, y2, dx, dy;
		if (_cache_map)
		{
			_load_map(data, _cache_location);
			x1 = floor(_cache_location.x());
			x2 = floor(_cache_location.x() + _width / _block_size);
			y1 = floor(_cache_location.y());
			y2 = floor(_cache_location.y() + _height / _block_size);
			dx = x2 - x1;
			dy = y2 - y1;
			QImage* temp = debug_new QImage(_width, _height, QImage::Format::Format_ARGB32);
			QPainter* temp2 = debug_new QPainter(temp);
			int w = min(_width, _cache_width), h = min(_height, _cache_height);
			temp2->drawImage(QRect(0, 0, w, h), *_cache_map, QRect(0, 0, w, h));
			//part redraw
			if (_width > _cache_width || _height > _cache_height)
			{
				//uses _cache_location.x() and y() instead of x1, y1
				int xc = floor(_cache_location.x() + _cache_width / _block_size);
				int yc = floor(_cache_location.y() + _cache_height / _block_size);

				for (int x = xc; x <= x2; x++)
				{
					for (int y = y1; y <= y2; y++)
					{
						QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
						QPointF pos = _translate_pos(QPointF(x, y), _cache_location);
						temp2->drawImage(QRect(floor(pos.x()), floor(pos.y()), (int)_block_size, (int)_block_size), *img);
					}
				}
				for (int x = x1; x < xc; x++)
				{
					for (int y = yc; y <= y2; y++)
					{
						QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
						QPointF pos = _translate_pos(QPointF(x, y), _cache_location);
						temp2->drawImage(QRect(floor(pos.x()), floor(pos.y()), (int)_block_size, (int)_block_size), *img);
					}
				}
			}
			_cache_width = _width;
			_cache_height = _height;
			debug_delete temp2;
			debug_delete _cache_map;
			_cache_map = temp;
		}
		else
		{
			_load_map(data, _location);
			x1 = floor(_location.x());
			x2 = floor(_location.x() + _width / _block_size);
			y1 = floor(_location.y());
			y2 = floor(_location.y() + _height / _block_size);
			dx = x2 - x1;
			dy = y2 - y1;
			_cache_map = debug_new QImage(_width, _height, QImage::Format::Format_ARGB32);
			QPainter* temp = debug_new QPainter(_cache_map);
			_cache_width = _width;
			_cache_height = _height;
			_cache_location = _location;
			//full redraw
			for (int x = x1; x <= x2; x++)
			{
				for (int y = y1; y <= y2; y++)
				{
					QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
					QPointF pos = _translate_pos(QPointF(x, y), _cache_location);
					temp->drawImage(QRect(floor(pos.x()), floor(pos.y()), (int)_block_size, (int)_block_size), *img);
				}
			}
			debug_delete temp;
		}
		for (int i = 0; i <= dx; i++)
			debug_delete[] data[i];
		debug_delete[] data;
		data = nullptr;
	}
#pragma endregion

#pragma region cache translate
	if (_location != _cache_location)
	{
		int x1 = floor(_location.x());
		int x2 = floor(_location.x() + _width / _block_size);
		int y1 = floor(_location.y());
		int y2 = floor(_location.y() + _height / _block_size);
		int dx = x2 - x1;
		int dy = y2 - y1;
		_load_map(data, _location);
		QRect src, dst;
		if (_location.x() >= _cache_location.x())
		{
			int dx = floor((_location.x() - _cache_location.x()) * _block_size);
			src.setX(dx);
			src.setWidth(_width - dx);
			dst.setX(0);
			dst.setWidth(_width - dx);
		}
		else
		{
			int dx = floor((_cache_location.x() - _location.x()) * _block_size);
			src.setX(0);
			src.setWidth(_width - dx);
			dst.setX(dx);
			dst.setWidth(_width - dx);
		}
		if (_location.y() >= _cache_location.y())
		{
			int dy = floor((_location.y() - _cache_location.y())*_block_size);
			src.setY(dy);
			src.setHeight(_height - dy);
			dst.setY(0);
			dst.setHeight(_height - dy);
		}
		else
		{
			int dy = floor((_cache_location.y() - _location.y())*_block_size);
			src.setY(0);
			src.setHeight(_height - dy);
			dst.setY(dy);
			dst.setHeight(_height - dy);
		}

		QImage* temp = debug_new QImage(_width, _height, QImage::Format::Format_ARGB32);
		QPainter* temp2 = debug_new QPainter(temp);
		temp2->drawImage(dst, *_cache_map, src);

		int xc1 = floor(_location.x() + dst.x() / _block_size);
		int xc2 = floor(_location.x() + (dst.x() + dst.width()) / _block_size);
		int yc1 = floor(_location.y() + dst.y() / _block_size);
		int yc2 = floor(_location.y() + (dst.y() + dst.height()) / _block_size);

		for (int x = x1; x <= x2; x++)
		{
			for (int y = y1; y <= yc1; y++)
			{
				QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
				QPointF pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(floor(pos.x()), floor(pos.y()), (int)_block_size, (int)_block_size), *img);
			}
			for (int y = yc2; y <= y2; y++)
			{
				QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
				QPointF pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(floor(pos.x()), floor(pos.y()), (int)_block_size, (int)_block_size), *img);
			}
		}

		for (int y = yc1 + 1; y < y2; y++)
		{
			for (int x = x1; x <= xc1; x++)
			{
				QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
				QPointF pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(floor(pos.x()), floor(pos.y()), (int)_block_size, (int)_block_size), *img);
			}
			for (int x = xc2; x <= x2; x++)
			{
				QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
				QPointF pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(floor(pos.x()), floor(pos.y()), (int)_block_size, (int)_block_size), *img);
			}
		}

		_cache_location = _location;
		debug_delete temp2;
		debug_delete _cache_map;
		_cache_map = temp;
		for (int i = 0; i <= dx; i++)
			debug_delete[] data[i];
		debug_delete[] data;
		data = nullptr;
	}
#pragma endregion
	//merging map
	p.drawImage(QRect(0, 0, _width, _height), *_cache_map);
}

QImage MapLoader::renderMiniMap(int w, int h)
{
	QImage img(w, h, QImage::Format::Format_ARGB32);
	QPainter p(&img);

	const QPen pen_mine(Qt::red);
	const QPen pen_flag(Qt::green);
	const QPen pen_unknown(Qt::blue);
	const QPen pen_unclicked(Qt::black);
	const QPen pen_rect(Qt::gray);
	const QPen pen_clicked(Qt::white);

	QPointF offset_f((w - _width / _block_size) / 2, (h - _height / _block_size) / 2);
	QPointF loc_f = _location - offset_f;

	QRect win_rect(floor(offset_f.x()), floor(offset_f.y()), floor(_width / _block_size) + 1, floor(_height / _block_size) + 1);
	QPoint loc(floor(loc_f.x()), floor(loc_f.y()));

	char** data = nullptr;
	_cl->GetBlockData(loc.x(), loc.y(), loc.x() + w - 1, loc.y() + h - 1, data);

	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			char cur_data = data[x][y];
			if ((cur_data & 0x60) == 0x60)
				p.setPen(pen_unknown);
			else if ((cur_data & 0x40))
				p.setPen(pen_flag);
			else if ((cur_data & 0x30) == 0x30)
				p.setPen(pen_mine);
			else if (cur_data & 0x20)
				p.setPen(pen_clicked);
			else
				p.setPen(pen_unclicked);

			p.drawPoint(x, y);
		}
	}

	p.setPen(pen_rect);
	p.drawRect(win_rect);

	for (int i = 0; i < w; i++)
		delete data[i];
	delete data;
	return img;
}

void MapLoader::clickBlock(QPoint block)
{
	list<QPoint> bfs_queue;
	list<QPoint> checked_block;
	//c++11 lambda expr [capture](param){body}
	auto exist_checked_block = [&](QPoint& x) 
	{
		for (auto lambda_iter = checked_block.begin(); lambda_iter != checked_block.end(); lambda_iter++)
			if (*lambda_iter == x) return true;
		for (auto lambda_iter = bfs_queue.begin(); lambda_iter != bfs_queue.end(); lambda_iter++)
			if (*lambda_iter == x) return true;
		return false; 
	};

	double cur_time = fGetCurrentTimestamp();

	QPainter p(_cache_map);
	char blockdata = _cl->GetBlockData(block.x(), block.y());
	if ((blockdata & 0x60) == 0x60)
	{
		//被标记为??，应该是不能点的
	}
	else if (blockdata & 0x40)
	{
		//被标记为旗
	}
	else if (blockdata & 0x20)
	{
		//该方块已经被点击 and 不是雷
		if ((blockdata & 0x10) == 0x00)
		{
			//在周围的未点击方块个数等于该位置的数字时清空周围8个方块
			int known_block_count = 0; //已标记的雷的个数
			int expect_block_count = blockdata & 0x0f;
			char code[8];
			QPoint nearby[8];
			_get_nearby_block(block, nearby);
			for (int i = 0; i < 8; i++)
			{
				code[i] = _cl->GetBlockData(nearby[i].x(), nearby[i].y());
				if ((code[i] & 0x30) == 0x30 || (code[i] & 0x60) == 0x40)
					known_block_count++;
			}
			if (known_block_count == expect_block_count)
			{
				//符合清空的要求
				for (int i = 0; i < 8; i++)
				{
					if ((code[i] & 0x60) == 0x60 || (code[i] & 0x60) == 0x00)
					{
						//清空??标记,即设置为X01X XXXX
						code[i] = (code[i] & 0x9f) | 0x20;
						_cl->SetBlockData(nearby[i].x(), nearby[i].y(), code[i]);
						_draw_block(nearby[i], code[i], p);
						bfs_queue.push_back(nearby[i]); //添加到空方块搜索队列

						//stat
						if ((code[i] & 0x60) == 0x60)
							_unknown_made--;
						else if ((code[i] & 0x10))
							_mine_clicked++;
					}
				}
			}
		}
	}
	else if (blockdata & 0x10)
	{
		//当前为雷，被点击
		blockdata |= 0x20;
		_cl->SetBlockData(block.x(), block.y(), blockdata);
		_draw_block(block, blockdata, p);
		_mine_clicked++;
	}
	else
	{
		//当前为一般方块（非雷），被点击
		bfs_queue.push_back(block);
	}

	//bfs empty block search
	while (bfs_queue.size())
	{
		QPoint pos = bfs_queue.front();
		bfs_queue.pop_front();

		//char buf[50];
		//sprintf_s(buf, "bfs(%d,%d)\r\n", pos.x(), pos.y());
		//OutputDebugStringA(buf);

		char data = _cl->GetBlockData(pos.x(), pos.y());
		if (!(data & 0x1f))
		{
			//当前为空方块，继续向四周搜索
			QPoint nearby[8];
			_get_nearby_block(pos, nearby);
			for (int i = 0; i < 8; i++)
			{
				char nearby_data = _cl->GetBlockData(nearby[i].x(), nearby[i].y());
				if (!(nearby_data & 0x20)) //继续添加未点击的方块进搜索队列
					if (!exist_checked_block(nearby[i]))
					{
						checked_block.push_back(pos);
						bfs_queue.push_back(nearby[i]);
					}
			}
		}

		data |= 0x20;
		_cl->SetBlockData(pos.x(), pos.y(), data);
		_draw_block(pos, data, p);
		_cleared_block++;
	}
}

void MapLoader::_get_nearby_block(QPoint & block, QPoint nearby[])
{
	nearby[0] = QPoint(block.x() - 1, block.y() - 1);
	nearby[1] = QPoint(block.x() - 1, block.y());
	nearby[2] = QPoint(block.x() - 1, block.y() + 1);
	nearby[3] = QPoint(block.x(), block.y() - 1);
	nearby[4] = QPoint(block.x(), block.y() + 1);
	nearby[5] = QPoint(block.x() + 1, block.y() - 1);
	nearby[6] = QPoint(block.x() + 1, block.y());
	nearby[7] = QPoint(block.x() + 1, block.y() + 1);
}

void MapLoader::rightClickBlock(QPoint block)
{
	double cur_time = fGetCurrentTimestamp();

	char blockdata = _cl->GetBlockData(block.x(), block.y());
	QPainter p(_cache_map);
	if ((blockdata & 0x60) == 0x60)
	{
		//被标记为??，取消标记
		blockdata &= 0x9f;
		_cl->SetBlockData(block.x(), block.y(), blockdata);
		_draw_block(block, blockdata, p);
		_unknown_made--;
	}
	else if (blockdata & 0x40)
	{
		//被标记为旗时，再次右键标记为？
		blockdata = (blockdata & 0x9f) | 0x60;
		_cl->SetBlockData(block.x(), block.y(), blockdata);
		_draw_block(block, blockdata, p);
		_flag_made--;
		_unknown_made++;
	}
	else if (blockdata & 0x20)
	{
		//该方块已经被点击，满足条件时清空周围8个方块
		int unknown_block_count = 0; //未点击的个数方块
		int expect_block_count = blockdata & 0x0f;
		char code[8];
		QPoint nearby[8];
		_get_nearby_block(block, nearby);
		for (int i = 0; i < 8; i++)
		{
			code[i] = _cl->GetBlockData(nearby[i].x(), nearby[i].y());
			if ((code[i] & 0x60) == 0x60 || (code[i] & 0x20) == 0x00)
				unknown_block_count++;
			else if ((code[i] & 0x30) == 0x30)
				expect_block_count--; //已经点过的雷
		}
		if (unknown_block_count == expect_block_count)
		{
			//符合清空的要求
			for (int i = 0; i < 8; i++)
			{
				if ((code[i] & 0x60) == 0x60 || (code[i] & 0x60) == 0x00)
				{
					//清空??标记,即设置为X10X XXXX
					code[i] = (code[i] & 0x9f) | 0x40;
					_cl->SetBlockData(nearby[i].x(), nearby[i].y(), code[i]);
					_draw_block(nearby[i], code[i], p);
					if ((code[i] & 0x60) == 0x60)
						_unknown_made--;
					_flag_made++;
				}
			}
		}
	}
	else
	{
		//当前为雷或一般方块，被右键
		blockdata |= 0x40;
		_cl->SetBlockData(block.x(), block.y(), blockdata);
		_draw_block(block, blockdata, p);
		_flag_made++;
	}
}
