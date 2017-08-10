#pragma once
#include "chunkloader.h"
#include <qpoint.h>
#include <io.h>
#include <direct.h>
#include <qwidget.h>
#include <qobject.h>

//作为界面渲染的中间类
class MapLoader : public QObject
{
	Q_OBJECT
private:
	//区块加载
	ChunkLoader* _cl;
	//当前坐标
	QPointF _location;

	//方块的大小（像素)
	double _block_size;
	//屏幕大小
	double _width, _height;

	//functions
	static Tag* _chunk_gen_cb(void* sender, int cx, int cy);
	void _load_config();
	void _save_config();
	
	//根据雷的位置生成数据
	bool _is_gen_mask(int cx, int cy);
	void _gen_mask(int cx, int cy);
	//加载当前的方块数据
	void _load_map(char**& data);
	//转换为屏幕坐标
	QPoint _translate_pos(QPointF block);
public:
	MapLoader();
	~MapLoader();

	inline double blockSize() const { return _block_size; }
	inline void setBlockSize(int size) { _block_size = size; }
	inline double width() const { return _width; }
	inline void setWidth(double width) { _width = width; }
	inline double height() const { return _height; }
	inline void setHeight(double height) { _height = height; }
	inline QPointF location() const { return _location; }
	inline void setLocation(QPointF location) { _location = location; }
	inline void setDeltaLocation(QPointF delta) { _location += delta; }

public slots:
	void RenderMap(QPainter& p, QWidget* parent);
};