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
constexpr int    MAX_UNDO_SIZE = 50;

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
    bool isNotSaved() const { return !_is_saved; }

    double getScale() const;

    void replaceCurrentLabel(const LabelInfo& dst);

    int getPenSize() const;

    int getId() const;

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

    inline void addUndo() {

        if (_undo_list.size() >= MAX_UNDO_SIZE) {
            _undo_list.pop_front();
            --_undo_index;
        }

        if (_undo_index != _undo_list.size()) {
            _undo_list = _undo_list.mid(0, _undo_index);
        }

        _undo_list.push_back(_mask);
        ++_undo_index;
        _is_saved = false;

        auto index = 0;
        for (auto t: _undo_list) {
            auto tt = t.color;
            auto m = qImage2Mat(tt);
            cv::imwrite(QString("/home/undead/undo/%1.png").arg(index++).toStdString(), m);
        }
    }

	MainWindow *_ui;
	
	void _initPixmap();
	void _drawFillCircle(QMouseEvent * e);

    bool _is_saved = true;

    int _id = 0;
	QScrollArea     *_scroll_parent    ;
	double           _scale            ;
	double           _alpha            ;
	QImage           _image            ;
	ImageMask        _mask             ;
	QList<ImageMask> _undo_list        ;
    //bool             _undo             ;
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
