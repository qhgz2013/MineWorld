#pragma once
#include "chunkloader.h"
#include <qpoint.h>
#include <io.h>
#include <direct.h>
#include <qwidget.h>
#include <qobject.h>
#include <list>
//��Ϊ������Ⱦ���м���
class MapLoader : public QObject
{
	Q_OBJECT
private:
	struct structA { MapLoader* sender; char code; };
	//�������
	ChunkLoader* _cl;
	//���׵�ͼƬ ���滻
	QImage* _mine_icon;
	//��ǰ����
	QPointF _location;

	//����Ĵ�С������)
	double _block_size;
	//��Ļ��С
	double _width, _height;

	QImage** _thumbnail_cache;
	static QImage _render_thumbnail(structA code);

	//functions
	static Tag* _chunk_gen_cb(void* sender, int cx, int cy);
	void _load_config();
	void _save_config();

	//�����׵�λ����������
	bool _is_gen_mask(int cx, int cy);
	void _gen_mask(int cx, int cy);
	//���ص�ǰ�ķ�������
	void _load_map(char**& data);
	//ת��Ϊ��Ļ����
	QPoint _translate_pos(QPointF block);

	//����˳��
	static double _animation_duration;
	std::list<double> _start_time;
	std::list<QPoint> _affect_block;
	enum class animationType
	{
		Enter, Leave, Click
	};
	std::list<animationType> _type;
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
	inline QPointF blockAt(QPoint mouse_pos) const { return QPointF(_location.x() + mouse_pos.x() / _block_size, _location.y() + mouse_pos.y() / _block_size); }
	void renderMap(QPainter& p, QWidget* form);
	void renderAnimation(QPainter& p, QWidget* form);
	//��Ӧ���¼�

	void enterBlock(QPoint block);
	void leaveBlock(QPoint block);
	void clickBlock(QPoint block);

};