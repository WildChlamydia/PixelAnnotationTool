#ifndef IMAGE_CANVAS_H
#define IMAGE_CANVAS_H

#include "utils.h"
#include "image_mask.h"

#include <QLabel>
#include <QPen>
#include <QScrollArea>
#include <QCursor>

class MainWindow;

constexpr double ZOOM_STEP = 0.5;
constexpr double SMALL_ZOOM_STEP = 0.1;
constexpr double ZOOM_MIN = 0.1;
constexpr double ZOOM_MAX = 10.0;
constexpr double FLOAT_DELTA = 0.000001;
constexpr double ZOOM_TRESH = 1.0;
constexpr double CARRY_SPEED = 1.2;

class ImageCanvas : public QLabel {
	Q_OBJECT

public:

	ImageCanvas(MainWindow *ui);
    ~ImageCanvas();

	void setId(int id);
	void setMask(const ImageMask & mask);

	ImageMask getMask() const { return _mask; }
	QImage getImage() const { return _image; }

	void setWatershedMask(QImage watershed);
	void refresh();
	void updateMaskColor(const Id2Labels & labels) { _mask.updateColor(labels); }
	void loadImage(const QString &file);
	QScrollArea * getScrollParent() const { return _scroll_parent; }
    bool isNotSaved() const { return _undo_list.size() > 1; }

    double getScale() const;

    void replaceCurrentLabel(const LabelInfo& dst);

protected:
    void mouseMoveEvent(QMouseEvent * event) override;
	void mousePressEvent(QMouseEvent * event) override;
	void keyPressEvent(QKeyEvent * event) override;
	void wheelEvent(QWheelEvent * event) override;
	void mouseReleaseEvent(QMouseEvent * event) override;
	void paintEvent(QPaintEvent *event) override;

public slots :
	void scaleChanged(double);
	void alphaChanged(double);
	void setSizePen(int);
	void clearMask();
	void saveMask();
	void undo();
	void redo();
    void scaleCanvas(int delta);
	
private:
    bool eventFilter(QObject *target, QEvent *event) override;

	MainWindow *_ui;
	
	void _initPixmap();
	void _drawFillCircle(QMouseEvent * e);

	QScrollArea     *_scroll_parent    ;
	double           _scale            ;
	double           _alpha            ;
	QImage           _image            ;
	ImageMask        _mask             ;
	ImageMask        _watershed        ;
	QList<ImageMask> _undo_list        ;
	bool             _undo             ;
	int              _undo_index       ;
    QPoint           _local_mouse_pos        ;
	QString          _img_file         ;
	QString          _mask_file        ;
	QString          _watershed_file   ;
	ColorMask        _color            ;
	int              _pen_size         ;
    bool             _button_is_pressed;
    bool             _carry_activated = false;
    QPoint           _global_mouse_pos = QPoint(0, 0);
};



#endif //IMAGE_CANVAS_H
