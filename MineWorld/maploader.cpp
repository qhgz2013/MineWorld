#include "maploader.h"
#include <string>
#include <fstream>
#include <qpainter.h>
#include <qlist.h>
#include <qfuture.h>
#include <QtConcurrent\qtconcurrentmap.h>
#include "util.h"
#include <Windows.h>
#include <queue>
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
				if (x - 1 > 0 && y > 0) data[x - 1][y]++;
				if (x - 1 > 0 && y + 1 < size - 1)data[x - 1][y + 1]++;
				if (x > 0 && x < size - 1 && y - 1 > 0) data[x][y - 1]++;
				if (x > 0 && x < size - 1 && y + 1 < size - 1) data[x][y + 1]++;
				if (x + 1 < size - 1 && y - 1 > 0)data[x + 1][y - 1]++;
				if (x + 1 < size - 1 && y > 0 && y < size - 1) data[x + 1][y]++;
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

void MapLoader::_load_map(char **& data, QPointF& pt)
{
	QPointF pf1(pt), pf2(pt.x() + _width / _block_size, pt.y() + _height / _block_size);
	QPoint p1((int)floor(pf1.x()), (int)floor(pf1.y())), p2((int)floor(pf2.x()), (int)floor(pf2.y()));
	auto c1 = _cl->GetChunkPos(p1.x(), p1.y());
	auto c2 = _cl->GetChunkPos(p2.x(), p2.y());
	char buf[140];
	sprintf_s(buf, "load block (%d,%d) to (%d,%d) / origin:(%f,%f) to (%f,%f)\r\n", p1.x(), p1.y(), p2.x(), p2.y(), pf1.x(), pf1.y(), pf2.x(), pf2.y());
	OutputDebugStringA(buf);
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

QPoint MapLoader::_translate_pos(QPointF& block, QPointF& left_top)
{
	double x = (block.x() - left_top.x())*_block_size;
	double y = (block.y() - left_top.y())*_block_size;
	return QPoint((int)x, (int)y);
}

MapLoader::MapLoader()
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
	_load_config();
}

MapLoader::~MapLoader()
{
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
						QPoint pos = _translate_pos(QPointF(x, y), _cache_location);
						temp2->drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
					}
				}
				for (int x = x1; x < xc; x++)
				{
					for (int y = yc; y <= y2; y++)
					{
						QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
						QPoint pos = _translate_pos(QPointF(x, y), _cache_location);
						temp2->drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
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
					QPoint pos = _translate_pos(QPointF(x, y), _cache_location);
					temp->drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
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
			int dx = (int)((_location.x() - _cache_location.x()) * _block_size);
			src.setX(dx);
			src.setWidth(_width - dx);
			dst.setX(0);
			dst.setWidth(_width - dx);
		}
		else
		{
			int dx = (int)((_cache_location.x() - _location.x()) * _block_size);
			src.setX(0);
			src.setWidth(_width - dx);
			dst.setX(dx);
			dst.setWidth(_width - dx);
		}
		if (_location.y() >= _cache_location.y())
		{
			int dy = (int)((_location.y() - _cache_location.y())*_block_size);
			src.setY(dy);
			src.setHeight(_height - dy);
			dst.setY(0);
			dst.setHeight(_height - dy);
		}
		else
		{
			int dy = (int)((_cache_location.y() - _location.y())*_block_size);
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
				QPoint pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
			}
			for (int y = yc2; y <= y2; y++)
			{
				QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
				QPoint pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
			}
		}

		for (int y = yc1 + 1; y < y2; y++)
		{
			for (int x = x1; x <= xc1; x++)
			{
				QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
				QPoint pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
			}
			for (int x = xc2; x <= x2; x++)
			{
				QImage* img = _thumbnail_cache[(data[x - x1][y - y1] & 0x7f)];
				QPoint pos = _translate_pos(QPointF(x, y), _location);
				temp2->drawImage(QRect(pos.x(), pos.y(), (int)_block_size, (int)_block_size), *img);
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

void MapLoader::renderAnimation(QPainter& p, QWidget* form)
{
	//auto i1 = _start_time.begin();
	//auto i2 = _affect_block.begin();
	//auto i3 = _type.begin();
	//int size = (int)_block_size;

	//double curtime = fGetCurrentTimestamp();

	//while (i2 != _affect_block.end())
	//{
	//	//timed out, remove
	//	if (*i1 + _animation_duration >= curtime)
	//	{
	//		_start_time.erase(i1);
	//		_affect_block.erase(i2);
	//		_type.erase(i3);
	//		i1 = _start_time.begin();
	//		i2 = _affect_block.begin();
	//		i3 = _type.begin();
	//	}

	//	QPoint pos = _translate_pos(QPointF(*i2), _location);
	//	double stat = (curtime - *i1) / _animation_duration;
	//	if (stat < 0.0) stat = 0.0;
	//	else if (stat > 1.0) stat = 1.0;
	//	if (pos.x() < _width || pos.y() < _height)
	//	{
	//		switch (*i3)
	//		{
	//		case animationType::Click:

	//		case animationType::Enter:
	//		case animationType::Leave:
	//		default:
	//			//type error
	//			throw 1; //throw a int exception
	//			break;
	//		}
	//	}

	//	i1++, i2++, i3++;
	//}
}

void MapLoader::enterBlock(QPoint block)
{
	//if (block.x() >= _location.x() + _width / _block_size || block.y() >= _location.y() + _height / _block_size) return;
	//auto i1 = _start_time.begin();
	//auto i2 = _affect_block.begin();
	//auto i3 = _type.begin();
	//for (; i2 != _affect_block.end(); i1++, i2++, i3++)
	//{
	//	if (*i2 == block)
	//	{
	//		_start_time.erase(i1);
	//		_affect_block.erase(i2);
	//		_type.erase(i3);
	//	}
	//}
	//double curtime = fGetCurrentTimestamp();
	//_start_time.push_back(curtime);
	//_affect_block.push_back(block);
	//_type.push_back(animationType::Enter);
}

void MapLoader::leaveBlock(QPoint block)
{
	//if (block.x() >= _location.x() + _width / _block_size || block.y() >= _location.y() + _height / _block_size) return;
	//auto i1 = _start_time.begin();
	//auto i2 = _affect_block.begin();
	//auto i3 = _type.begin();
	//for (; i2 != _affect_block.end(); i1++, i2++, i3++)
	//{
	//	if (*i2 == block)
	//	{
	//		_start_time.erase(i1);
	//		_affect_block.erase(i2);
	//		_type.erase(i3);
	//	}
	//}
	//double curtime = fGetCurrentTimestamp();
	//_start_time.push_back(curtime);
	//_affect_block.push_back(block);
	//_type.push_back(animationType::Leave);
}

void MapLoader::clickBlock(QPoint block)
{
	//if (block.x() >= _location.x() + _width / _block_size || block.y() >= _location.y() + _height / _block_size) return;
	//auto i1 = _start_time.begin();
	//auto i2 = _affect_block.begin();
	//auto i3 = _type.begin();
	//for (; i2 != _affect_block.end(); i1++, i2++, i3++)
	//{
	//	if (*i2 == block)
	//	{
	//		_start_time.erase(i1);
	//		_affect_block.erase(i2);
	//		_type.erase(i3);
	//	}
	//}
	//double curtime = fGetCurrentTimestamp();
	//_start_time.push_back(curtime);
	//_affect_block.push_back(block);
	//_type.push_back(animationType::Click);

	queue<QPoint> bfs_queue;

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
						bfs_queue.push(nearby[i]); //添加到空方块搜索队列
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
	}
	else
	{
		//当前为一般方块（非雷），被点击
		//blockdata |= 0x20;
		//_cl->SetBlockData(block.x(), block.y(), blockdata);
		bfs_queue.push(block);
	}

	//bfs empty block search
	while (bfs_queue.size())
	{
		QPoint pos = bfs_queue.front();
		bfs_queue.pop();

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
					bfs_queue.push(nearby[i]);
			}
		}

		data |= 0x20;
		_cl->SetBlockData(pos.x(), pos.y(), data);
		_draw_block(pos, data, p);
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
	char blockdata = _cl->GetBlockData(block.x(), block.y());
	QPainter p(_cache_map);
	if ((blockdata & 0x60) == 0x60)
	{
		//被标记为??，取消标记
		blockdata &= 0x9f;
		_cl->SetBlockData(block.x(), block.y(), blockdata);
		_draw_block(block, blockdata, p);

	}
	else if (blockdata & 0x40)
	{
		//被标记为旗
		blockdata = (blockdata & 0x9f) | 0x60;
		_cl->SetBlockData(block.x(), block.y(), blockdata);
		_draw_block(block, blockdata, p);
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
	}
}

void MapLoader::_draw_block(QPoint block, char data, QPainter& p)
{
	QPoint pos = _translate_pos(QPointF(block), _location);
	QImage* img = _thumbnail_cache[data & 0x7f];
	p.drawImage(QRect(pos.x(), pos.y(), _block_size, _block_size), *img);
}