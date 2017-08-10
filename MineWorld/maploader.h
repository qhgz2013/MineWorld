#pragma once
#include "chunkloader.h"
#include <qpoint.h>
#include <io.h>
#include <direct.h>
#include <qwidget.h>
#include <qobject.h>

//��Ϊ������Ⱦ���м���
class MapLoader : public QObject
{
	Q_OBJECT
private:
	//�������
	ChunkLoader* _cl;
	//��ǰ����
	QPointF _location;

	//����Ĵ�С������)
	double _block_size;
	//��Ļ��С
	double _width, _height;

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