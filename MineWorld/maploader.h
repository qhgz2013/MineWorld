#pragma once
#include "chunkloader.h"
#include <qpoint.h>
#include <io.h>
#include <direct.h>
#include <qwidget.h>
#include <qobject.h>
#include <list>
#include "util.h"
//作为界面渲染的中间类
class MapLoader;
struct structA { MapLoader* sender; char code; };
class MapLoader : public QObject
{
	Q_OBJECT
private:
	//区块加载
	ChunkLoader* _cl;
	//地雷的图片 可替换
	QImage* _mine_icon;
	QImage* _flag_icon;
	//当前坐标
	QPointF _location;

	//方块的大小（像素)
	double _block_size;
	//屏幕大小
	int _width, _height;
	int _possibility;
	//图形缓存
	QImage** _thumbnail_cache;
	static QImage _render_thumbnail(structA code);
	//统计信息
	uint64_t _cleared_block;
	uint64_t _flag_made;
	uint64_t _unknown_made;
	uint64_t _mine_clicked;
	double _time_played;

	double _ctor_time;
	//functions
	static Tag* _chunk_gen_cb(void* sender, int cx, int cy);
	void _load_config();
	void _save_config();

	//根据雷的位置生成数据
	bool _is_gen_mask(int cx, int cy);
	void _gen_mask(int cx, int cy);
	//加载当前的方块数据
	void _load_map(char**& data, QPointF& pt);
	//转换为屏幕坐标
	QPointF _translate_pos(QPointF& block, QPointF& left_top);
	void _get_nearby_block(QPoint& block, QPoint nearby[]);


	QPointF _cache_location;
	int _cache_width, _cache_height;
	QImage* _cache_map;
	void _draw_block(QPoint block, char data, QPainter& p);
public:
	MapLoader(int possibility = GEN_MINE_POSSIBILITY);
	~MapLoader();

	inline double blockSize() const { return _block_size; }
	inline void setBlockSize(int size) { _block_size = size; }
	inline double width() const { return _width; }
	inline void setWidth(int width) { _location = QPointF(_location.x() + (_width - width) / 2.0 / _block_size, _location.y()); _width = width; }
	inline double height() const { return _height; }
	inline void setHeight(int height) { _location = QPointF(_location.x(), _location.y() + (_height - height) / 2.0 / _block_size); _height = height; }
	inline QPointF location() const { return _location; }
	inline void setLocation(QPointF location) { _location = location; }
	inline void setDeltaLocation(QPointF delta) { _location += delta; }
	inline QPointF blockAt(QPoint mouse_pos) const { return QPointF(_location.x() + mouse_pos.x() / _block_size, _location.y() + mouse_pos.y() / _block_size); }
	inline uint64_t clearedBlock() const { return _cleared_block; }
	inline uint64_t flagMade() const { return _flag_made; }
	inline uint64_t unknownMade() const { return _unknown_made; }
	inline uint64_t mineClicked() const { return _mine_clicked; }
	inline double timePlayed() const { return _time_played + (fGetCurrentTimestamp() - _ctor_time); }
	inline void clearStatistics() { _cleared_block = _flag_made = _unknown_made = _mine_clicked = 0; _time_played = 0; }
	inline int possibility() const { return _possibility; }
	inline void setPossibility(int possibility) { _possibility = possibility; }
	void renderMap(QPainter& p, QWidget* form);
	QImage renderMiniMap(int w, int h);

	//对应的事件
	void clickBlock(QPoint block);
	void rightClickBlock(QPoint block);
};