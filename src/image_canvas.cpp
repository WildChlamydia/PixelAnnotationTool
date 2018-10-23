
#include "image_canvas.h"
#include "main_window.h"

#include <QtDebug>
#include <QtWidgets>

ImageCanvas::ImageCanvas(MainWindow *ui) :
    QLabel() ,
	_ui(ui),
	_alpha(0.5),
	_pen_size(30) {

    _scroll_parent = new QScrollArea(ui);
    setParent(_scroll_parent);
	resize(800,600);
	_scale = 1.0;
	_initPixmap();
	setScaledContents(true);
	setMouseTracking(true);
	_button_is_pressed = false;

    _undo_list.clear();
    _undo_index = 0;
    //_undo = false;

    _scroll_parent->setBackgroundRole(QPalette::Dark);
    _scroll_parent->setWidget(this);
    ui->spinbox_scale->setSingleStep(ZOOM_STEP);
    ui->spinbox_scale->setMinimum(ZOOM_MIN);
    ui->spinbox_scale->setMaximum(ZOOM_MAX);
    _ui->spinbox_pen_size->setMaximum(200);

    _scroll_parent->installEventFilter(this);
}

ImageCanvas::~ImageCanvas() {
    _scroll_parent->disconnect();
    _scroll_parent->deleteLater();
    _scroll_parent = nullptr;
}

void ImageCanvas::_initPixmap() {
	QPixmap newPixmap = QPixmap(width(), height());
	newPixmap.fill(Qt::white);
	QPainter painter(&newPixmap);
	const QPixmap * p = pixmap();
    if (p != NULL) {
		painter.drawPixmap(0, 0, *pixmap());
    }
	painter.end();
	setPixmap(newPixmap);
}

void ImageCanvas::loadImage(const QString &filename) {
	if (!_image.isNull() )
		saveMask();

	_img_file = filename;
	QFileInfo file(_img_file);
	if (!file.exists()) return;

 	_image = mat2QImage(cv::imread(_img_file.toLocal8Bit().toStdString()));
	
	_mask_file = file.dir().absolutePath()+ "/" + file.baseName() + "_mask.png";
    //_watershed_file = file.dir().absolutePath()+ "/" + file.baseName() + "_watershed_mask.png";

    //_watershed = ImageMask(_image.size());
	_undo_list.clear();
	_undo_index = 0;
	if (QFile(_mask_file).exists()) {
        _mask = ImageMask(_mask_file, _ui->id_labels);
        //_ui->runWatershed(this);// button_watershed->released());
		_ui->checkbox_manuel_mask->setChecked(true);
	} else {
		clearMask();
	}
    addUndo();

    _ui->undo_action->setEnabled(true);
	_ui->redo_action->setEnabled(false);

	setPixmap(QPixmap::fromImage(_image));
    resize(_scale * _image.size());
    _is_saved = true;
}

void ImageCanvas::saveMask() {
	if (isFullZero(_mask.id))
		return;

	_mask.id.save(_mask_file);
    if (!_mask.id.isNull()) {
        QImage watershed = _mask.id;
        if (!_ui->checkbox_border_ws->isChecked()) {
            watershed = removeBorder(_mask.id, _ui->id_labels);
        }

        //watershed.save(_watershed_file);
		QFileInfo file(_img_file);
		QString color_file = file.dir().absolutePath() + "/" + file.baseName() + "_color_mask.png";
        QImage colored = idToColor(watershed, _ui->id_labels);

        cv::Mat temp = qImage2Mat(colored);
        cv::imwrite(color_file.toLocal8Bit().toStdString(), temp);
	}

    _ui->setStarAtNameOfTab(false);
    _is_saved = true;
}

void ImageCanvas::scaleChanged(double scale) {

    if (scale == 0.0) {
        return;
    }

    auto size = _scale * _image.size();
    if (!size.width() || !size.height()) {
        return;
    }

    _scale  = scale;
    resize(size);
    //repaint();
}

void ImageCanvas::alphaChanged(double alpha) {
	_alpha = alpha;
	repaint();
}

void ImageCanvas::paintEvent(QPaintEvent *event) {

    QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);
    QRect rect = painter.viewport();
    QSize size = _scale * _image.size();

    if (size != _image.size()) {
        rect.size().scale(size, Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(pixmap()->rect());
    }
    QPoint topleft = QPoint(0, 0);

    painter.drawImage(topleft, _image);
	painter.setOpacity(_alpha);
    if (!_mask.id.isNull() && _ui->checkbox_manuel_mask->isChecked()) {
        painter.drawImage(topleft, _mask.color);
	}
		
//	if (!_watershed.id.isNull() && _ui->checkbox_watershed_mask->isChecked()) {
//        painter.drawImage(topleft, _watershed.color);
//	}

    if (_local_mouse_pos.x() > 10 && _local_mouse_pos.y() > 10 &&
        _local_mouse_pos.x() <= QLabel::size().width() - 10 &&
        _local_mouse_pos.y() <= QLabel::size().height() - 10) {
		painter.setBrush(QBrush(_color.color));
        painter.setPen(QPen(QBrush(_color.color), 1.0));
        if (!_carry_activated) {
            painter.drawEllipse(_local_mouse_pos.x() / _scale - _pen_size / 2,
                                _local_mouse_pos.y() / _scale - _pen_size / 2, _pen_size, _pen_size);
        }
		painter.end();
    }

}

void ImageCanvas::mouseMoveEvent(QMouseEvent * e) {

    auto pos = this->mapToGlobal(e->pos());
    auto shift = _global_mouse_pos - pos;
    _global_mouse_pos = pos;
    _local_mouse_pos = e->pos();

    if (_button_is_pressed) {
		_drawFillCircle(e);
    }
    if (_carry_activated){
        _scroll_parent->verticalScrollBar()->setValue(_scroll_parent->verticalScrollBar()->value()
                                                      + shift.y() * CARRY_SPEED);
        _scroll_parent->horizontalScrollBar()->setValue(_scroll_parent->horizontalScrollBar()->value()
                                                        + shift.x() * CARRY_SPEED);
    }

	update();
}

void ImageCanvas::setSizePen(int pen_size) {
	_pen_size = pen_size;
}


void ImageCanvas::mouseReleaseEvent(QMouseEvent * e) {

	if(e->button() == Qt::LeftButton) {
        if (_carry_activated) {
            _carry_activated = false;
            qApp->setOverrideCursor(Qt::ArrowCursor);
            repaint();
            return;
        }
        if (_button_is_pressed)
            addUndo();

		_button_is_pressed = false;

//		if (_undo) {
//			QMutableListIterator<ImageMask> it(_undo_list);
//			int i = 0;
//			while (it.hasNext()) {
//				it.next();
//				if (i++ >= _undo_index)
//					it.remove();
//			}
//			_undo = false;
//			_ui->redo_action->setEnabled(false);
//        }


        _ui->setStarAtNameOfTab(true);
		_ui->undo_action->setEnabled(true);
	}

	if (e->button() == Qt::RightButton) { // selection of label
        QColor color = _mask.id.pixel(_local_mouse_pos / _scale);
		const LabelInfo * label = _ui->id_labels[color.red()];

//		if (!_watershed.id.isNull() && _ui->checkbox_watershed_mask->isChecked()) {
//            QColor color = QColor(_watershed.id.pixel(_local_mouse_pos / _scale));
//			QMap<int, const LabelInfo*>::const_iterator it = _ui->id_labels.find(color.red());
//			if (it != _ui->id_labels.end()) {
//				label = it.value();
//			}
//		}
		if(label->item != NULL)
			emit(_ui->list_label->currentItemChanged(label->item, NULL));

		refresh();
	}
}

void ImageCanvas::mousePressEvent(QMouseEvent * e) {
	setFocus();
    if (e->button() == Qt::LeftButton) {
        if (Qt::ShiftModifier == e->modifiers()) {
            _carry_activated = true;
            qApp->setOverrideCursor(Qt::PointingHandCursor);
            repaint();
        } else {
            _button_is_pressed = true;
            _drawFillCircle(e);
        }
	}
}

void ImageCanvas::_drawFillCircle(QMouseEvent * e) {
	if (_pen_size > 0) {
		int x = e->x() / _scale - _pen_size / 2;
		int y = e->y() / _scale - _pen_size / 2;
        _mask.drawFillCircle(x, y, _pen_size, _color);
	} else {
        int x = (e->x() + 0.5) / _scale ;
        int y = (e->y() + 0.5) / _scale ;
        _mask.drawPixel(x, y, _color);
	}
	update();
}

double ImageCanvas::getScale() const
{
    return _scale;
}

void ImageCanvas::replaceCurrentLabel(const LabelInfo &dst)
{
    cv::Mat temp_color_mat = qImage2Mat(_mask.color);
    cv::Mat temp_id_mat = qImage2Mat(_mask.id);

    addUndo();

    cv::Mat mask;
    auto src_color = cv::Scalar(_color.color.blue(), _color.color.green(), _color.color.red());
    auto dst_color = cv::Scalar(dst.color.blue(), dst.color.green(), dst.color.red());
    cv::inRange(temp_color_mat, src_color, src_color, mask);
    temp_color_mat.setTo(dst_color, mask);

    auto src_id = cv::Scalar(_color.id.blue(), _color.id.green(), _color.id.red());
    auto dst_id = cv::Scalar(dst.id, dst.id, dst.id);
    cv::inRange(temp_id_mat, src_id, src_id, mask);
    temp_id_mat.setTo(dst_id, mask);

    _mask.color = mat2QImage(temp_color_mat);
    _mask.id = mat2QImage(temp_id_mat);

    update();
}

void ImageCanvas::clearMask() {
    _mask = ImageMask(_image.size());
    //_watershed = ImageMask(_image.size());
    _undo_list.clear();
	_undo_index = 0;
    addUndo();
	repaint();
	
}

void ImageCanvas::scaleCanvas(int delta)
{

    double step = ZOOM_STEP;
    if (_ui->spinbox_scale->value() < ZOOM_TRESH){
        step = SMALL_ZOOM_STEP;
    }
    if (fabs(_ui->spinbox_scale->value() - ZOOM_TRESH) < FLOAT_DELTA) {
        if (delta < 0) {
            step = SMALL_ZOOM_STEP;
        }
        if (delta > 0) {
            step = ZOOM_STEP;
        }
    }

    double value = _ui->spinbox_scale->value() + delta * step;
    value = std::min<double>(_ui->spinbox_scale->maximum(), value);
    value = std::max<double>(_ui->spinbox_scale->minimum(), value);
    if (_ui->spinbox_scale->value() > ZOOM_TRESH && value < ZOOM_TRESH
            || _ui->spinbox_scale->value() < ZOOM_TRESH && value > ZOOM_TRESH) {
        value = ZOOM_TRESH;
    }

    double v_w = _scroll_parent->viewport()->width() - _scroll_parent->frameWidth();
    double v_h = _scroll_parent->viewport()->height() - _scroll_parent->frameWidth();

    if (delta < 0 && (value * _image.size().width() <= v_w
            &&
        value * _image.size().height() <= v_h)) {

        if (_scale * _image.size().width() <= v_w && _scale * _image.size().height() <= v_h) {
            return;
        }

        value = std::min((v_h + _scroll_parent->verticalScrollBar()->width()) / _image.size().height(),
                         (v_w + _scroll_parent->horizontalScrollBar()->height()) / _image.size().width());

    }

    _ui->spinbox_scale->setValue(value);
    scaleChanged(value);

    repaint();
}

bool ImageCanvas::eventFilter(QObject *target, QEvent *event)
{
    if (_scroll_parent == target || qApp == target) {
        if (event->type() == QEvent::KeyPress) {
            auto ev = (QKeyEvent*)(event);
            if (Qt::ShiftModifier == ev->modifiers()) {
                _scroll_parent->verticalScrollBar()->setEnabled(false);
            }
        }
        if (event->type() == QEvent::KeyRelease) {
            auto ev = (QKeyEvent*)(event);
            if (Qt::ShiftModifier == ev->modifiers()) {
                _scroll_parent->verticalScrollBar()->setEnabled(true);
            }
        }
    }
    return false;
}

void ImageCanvas::wheelEvent(QWheelEvent *event) {

    int delta = event->delta() > 0 ? 1 : -1;
    if (Qt::ShiftModifier == event->modifiers()) {

        int value = 0;
        if (_ui->spinbox_pen_size->value() == 1) {
            value = delta * _ui->spinbox_pen_size->singleStep();
        } else {
            value = _ui->spinbox_pen_size->value() + delta * _ui->spinbox_pen_size->singleStep();
        }
        if (value <= 0) {
            value = 1;
        }
        _ui->spinbox_pen_size->setValue(value);
        emit(_ui->spinbox_pen_size->valueChanged(value));
        setSizePen(value);
        repaint();

    } else if (Qt::ControlModifier == event->modifiers()) {

        _scroll_parent->verticalScrollBar()->setEnabled(false);
        scaleCanvas(delta);
        event->ignore();

    } else {

        _scroll_parent->verticalScrollBar()->setEnabled(true);

    }

}

void ImageCanvas::keyPressEvent(QKeyEvent * event) {
	if (event->key() == Qt::Key_Space) {
		emit(_ui->button_watershed->released());
	}
}

void ImageCanvas::setWatershedMask(QImage watershed) {
    watershed = removeBorder(watershed, _ui->id_labels);

    _mask.id = watershed;

    idToColor(_mask.id, _ui->id_labels, &_mask.color);

    addUndo();

}

void ImageCanvas::setMask(const ImageMask & mask) {
	_mask = mask;
}

void ImageCanvas::setId(int id) {
	_color.id = QColor(id, id, id);
	_color.color = _ui->id_labels[id]->color;
}

void ImageCanvas::refresh() {
//	if (!_watershed.id.isNull() && _ui->checkbox_watershed_mask->isChecked() ) {
//		emit(_ui->button_watershed->released());
//	}
	update();
}


void ImageCanvas::undo() {

    //_undo = true;
    _undo_index--;
    if (_undo_index == 1) {
        _mask = _undo_list.at(0);
		_ui->undo_action->setEnabled(false);
		refresh();
	} else if (_undo_index > 1) {
		_mask = _undo_list.at(_undo_index - 1);
		refresh();
	} else {
		_undo_index = 0;
        _mask = _undo_list.at(0);
		_ui->undo_action->setEnabled(false);
	}
	_ui->redo_action->setEnabled(true);
}

void ImageCanvas::redo() {
// NOTE: Undo debug
//    auto index = 0;
//    for (auto t: _undo_list) {
//        auto tt = t.color;
//        auto m = qImage2Mat(tt);
//        cv::imwrite(QString("/home/undead/undo/%1.png").arg(index++).toStdString(), m);
//    }
	_undo_index++;
	if (_undo_index < _undo_list.size()) {
		_mask = _undo_list.at(_undo_index - 1);
		refresh();
	}  else if (_undo_index == _undo_list.size()) {
		_mask = _undo_list.at(_undo_index - 1);
		_ui->redo_action->setEnabled(false);
		refresh();
	} else {
		_undo_index = _undo_list.size();
		_ui->redo_action->setEnabled(false);
	}
	_ui->undo_action->setEnabled(true);
}
