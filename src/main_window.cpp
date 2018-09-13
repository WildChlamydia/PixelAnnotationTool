#include "main_window.h"
#include "utils.h"
#include <QException>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QJsonDocument>
#include <QPainter>
#include <QJsonObject>
#include <QJsonArray>
#include <QColorDialog>
#include <QTextStream>
#include <QSettings>

//#include "pixel_annotation_tool_version.h"

#include "about_dialog.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
{
	setupUi(this);

	list_label->setSpacing(1);
    image_canvas = NULL;
	save_action = new QAction(tr("&Save current image"), this);
    save_action->setIcon(QIcon(":/save.png"));
    close_tab_action = new QAction(tr("&Close current tab"), this);
	undo_action = new QAction(tr("&Undo"), this);
	redo_action = new QAction(tr("&Redo"), this);
	undo_action->setShortcuts(QKeySequence::Undo);
	redo_action->setShortcuts(QKeySequence::Redo);
	save_action->setShortcut(Qt::CTRL+Qt::Key_S);
    close_tab_action->setShortcut(Qt::CTRL + Qt::Key_W);
	undo_action->setEnabled(false);
	redo_action->setEnabled(false);

	menuFile->addAction(save_action);
    menuFile->addSeparator();
    menuFile->addAction(this->actionQuit);
    menuEdit->addAction(close_tab_action);
	menuEdit->addAction(undo_action);
	menuEdit->addAction(redo_action);

    this->checkbox_border_ws->setVisible(false);

	tabWidget->clear();
    
	connect(button_watershed      , SIGNAL(released())                        , this, SLOT(runWatershed()  ));
	connect(actionOpen_config_file, SIGNAL(triggered())                       , this, SLOT(loadConfigFile()));
	connect(actionSave_config_file, SIGNAL(triggered())                       , this, SLOT(saveConfigFile()));
    connect(close_tab_action      , SIGNAL(triggered())                       , this, SLOT(closeCurrentTab()));
	connect(tabWidget             , SIGNAL(tabCloseRequested(int))            , this, SLOT(closeTab(int)   ));
	connect(tabWidget             , SIGNAL(currentChanged(int))               , this, SLOT(updateConnect(int)));
    connect(tree_widget_img       , SIGNAL(itemClicked(QTreeWidgetItem *,int)), this, SLOT(treeWidgetClicked()));
    
    labels = defaultLabels();

	loadConfigLabels();

    connect(list_label, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(changeLabel(QListWidgetItem*, QListWidgetItem*)));

    list_label->setEnabled(false);
    openDirectory();

    connect(&_timer_autosave,
            &QTimer::timeout,
            this,
            &MainWindow::autosave);

    _timer_autosave.start(AUTOSAVE_TIME_SECONDS * 1000);
    list_label->installEventFilter(this);
    this->dockWidget_3->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
}

void MainWindow::closeCurrentTab() {
    int index = tabWidget->currentIndex();
    if (index >= 0)
        closeTab(index);
}

void MainWindow::closeTab(int index) {
    ImageCanvas * ic = getImageCanvas(index);
    if (ic == nullptr)
        throw std::logic_error("error index");

    if (ic->isNotSaved()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Current image is not saved",
            "You will close the current image, Would you like saved image before ?", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            ic->saveMask();
        }
    }

    auto wid = tabWidget->widget(index);
    wid->disconnect();
    tabWidget->removeTab(index);
    allDisconnnect(ic);
    delete ic;
    ic = nullptr;
    image_canvas = nullptr;

    if (tabWidget->count() == 0) {
        list_label->setEnabled(false);
    } else {
        auto i = std::min(index, tabWidget->count() - 1);
        image_canvas = getImageCanvas(i);
        tabWidget->widget(i)->setFocus(Qt::ActiveWindowFocusReason);
    }
}

void MainWindow::loadConfigLabels() {
	list_label->clear();
    QMapIterator<QString, LabelInfo> it(labels);
    auto values = labels.values();

    qSort(values.begin(), values.end(), [](auto first, auto second) {
        return first.id < second.id;
    });

    QStringList list;
    for (const auto& v : values) {
        list.push_back(labels.key(v));
    }

    for (const auto& key : list) {
        const LabelInfo & label = labels[key];
		QListWidgetItem * item = new QListWidgetItem(list_label);
        LabelWidget * label_widget = new LabelWidget(label, this);

        // WARNING: Magic label item height constant
        label_widget->setFixedHeight(32);
		
		item->setSizeHint(label_widget->sizeHint());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		list_label->addItem(item);
		list_label->setItemWidget(item, label_widget);
        labels[key].item = item;

        label_widget->installEventFilter(this);
	}
	id_labels = getId2Label(labels);
}

void MainWindow::changeColor(QListWidgetItem *item) {

    if (!item) return;

	LabelWidget * widget = static_cast<LabelWidget*>(list_label->itemWidget(item));
    const LabelInfo &label = labels[widget->text()];
    image_canvas->replaceCurrentLabel(label);
}

void MainWindow::changeLabel(QListWidgetItem* current, QListWidgetItem* previous) {
	if (current == NULL && previous == NULL)
		return;

	LabelWidget * label;
	if (previous == NULL) {
        for (short i = 0, total = list_label->count(); i < total; ++i) {
            LabelWidget* label = static_cast<LabelWidget*>(list_label->itemWidget(list_label->item(i)));
			label->setSelected(false);
		}
	} else {
		label = static_cast<LabelWidget*>(list_label->itemWidget(previous));
		label->setSelected(false);
	}

	if (current == NULL) current = previous;

	label = static_cast<LabelWidget*>(list_label->itemWidget(current));
	label->setSelected(true);

	QString str;
	QString key = label->text();
	QTextStream sstr(&str);
	sstr <<"label=["<< key <<"] id=[" << labels[key].id << "] categorie=[" << labels[key].categorie << "] color=[" << labels[key].color.name() << "]" ;
    //statusBar()->showMessage(str);
	image_canvas->setId(labels[key].id);
}

void MainWindow::runWatershed(ImageCanvas * ic) {
    auto w = watershed(ic->getImage(), ic->getMask().id);
    ic->setWatershedMask(w);
    //checkbox_watershed_mask->setCheckState(Qt::CheckState::Checked);
	ic->update();
}

void MainWindow::runWatershed() {
    ImageCanvas * ic = image_canvas;
    if( ic != NULL)
        runWatershed(ic);
}

void MainWindow::setStarAtNameOfTab(bool star) {
    if (tabWidget->count() > 0) {
        int index = tabWidget->currentIndex();
        QString name = tabWidget->tabText(index);
        if (star && !name.endsWith("*")) { //add star
            name += "*";
            tabWidget->setTabText(index, name);
        } else if (!star && name.endsWith("*")) { //remove star
            int pos = name.lastIndexOf('*');
            name = name.left(pos);
            tabWidget->setTabText(index, name);
        }
    }
}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if (qobject_cast<LabelWidget*>(target)) {
        if ((event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
                event->type() == QEvent::MouseButtonDblClick) &&
                ((QMouseEvent*)(event))->button() == Qt::RightButton) {
            return true;
        }
        return false;
    }
    if (target == list_label) {
        if (event->type() == QEvent::ContextMenu) {
            const auto& p = QCursor::pos();
            QListWidgetItem *item = nullptr;
            auto wid_pos = list_label->mapToGlobal(list_label->rect().topLeft());
            QRect wid_rect;
            for (short i = 0; i < list_label->count(); ++i){
                item = list_label->item(i);
                wid_rect.setTop(wid_pos.y() + (list_label->itemWidget(item)->height() + 2) * i);
                wid_rect.setLeft(wid_pos.x());
                wid_rect.setWidth(list_label->itemWidget(item)->width());
                wid_rect.setHeight(list_label->itemWidget(item)->height());
                if (wid_rect.contains(p)) {
                    changeColor(item);
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}

void MainWindow::autosave()
{
    if (!image_canvas)
        return;

    if (!image_canvas->isNotSaved())
        return;

    image_canvas->saveMask();
}

void MainWindow::updateConnect(ImageCanvas * ic) {
    if (ic == nullptr)
        return;

    connect(spinbox_scale, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ic, &ImageCanvas::scaleChanged);

    connect(spinbox_alpha, SIGNAL(valueChanged(double)), ic, SLOT(alphaChanged(double)));
    connect(spinbox_pen_size, SIGNAL(valueChanged(int)), ic, SLOT(setSizePen(int)));

	connect(checkbox_manuel_mask, SIGNAL(clicked()), ic, SLOT(update()));
	connect(actionClear, SIGNAL(triggered()), ic, SLOT(clearMask()));
	connect(undo_action, SIGNAL(triggered()), ic, SLOT(undo()));
	connect(redo_action, SIGNAL(triggered()), ic, SLOT(redo()));
	connect(save_action, SIGNAL(triggered()), ic, SLOT(saveMask()));
    connect(checkbox_border_ws, SIGNAL(clicked()), ic, SLOT(update()));
    
}

void MainWindow::allDisconnnect(ImageCanvas *ic) {
    if (ic == NULL) return;

    disconnect(spinbox_scale, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ic, &ImageCanvas::scaleChanged);

    disconnect(spinbox_alpha, SIGNAL(valueChanged(double)), ic, SLOT(alphaChanged(double)));
    disconnect(spinbox_pen_size, SIGNAL(valueChanged(int)), ic, SLOT(setSizePen(int)));

    disconnect(checkbox_manuel_mask, SIGNAL(clicked()), ic, SLOT(update()));
    disconnect(actionClear, SIGNAL(triggered()), ic, SLOT(clearMask()));
    disconnect(undo_action, SIGNAL(triggered()), ic, SLOT(undo()));
    disconnect(redo_action, SIGNAL(triggered()), ic, SLOT(redo()));
    disconnect(save_action, SIGNAL(triggered()), ic, SLOT(saveMask()));
    disconnect(checkbox_border_ws, SIGNAL(clicked()), ic, SLOT(update()));

}

ImageCanvas * MainWindow::newImageCanvas() {
    ImageCanvas * ic = new ImageCanvas( this);
	ic->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	ic->setScaledContents(true);
	updateConnect(ic);
	return ic;
}

void MainWindow::updateConnect(int index) {
    if (index < 0 || index >= tabWidget->count())
        return;
    allDisconnnect(image_canvas);
    image_canvas = getImageCanvas(index);
    if(image_canvas!= NULL)
        list_label->setEnabled(true);
    else 
        list_label->setEnabled(false);
	updateConnect(image_canvas);
}

ImageCanvas * MainWindow::getImageCanvas(int index) {
    QScrollArea * scroll_area = static_cast<QScrollArea *>(tabWidget->widget(index));
    ImageCanvas * ic = static_cast<ImageCanvas*>(scroll_area->widget());
    return ic;
}

int MainWindow::getImageCanvas(QString name, ImageCanvas * ic) {
    for (short i = 0; i < tabWidget->count(); ++i) {
		if (tabWidget->tabText(i).startsWith(name) ) {
            ic = getImageCanvas(i);
			return i;
		}
	}
	ic = newImageCanvas();
	QString iDir = currentDir();
	QString filepath(iDir + "/" + name);
	ic->loadImage(filepath);
    int index = tabWidget->addTab(ic->getScrollParent(), name);
    tabWidget->widget(index)->setFocus(Qt::ActiveWindowFocusReason);
	return index;
}

QString MainWindow::currentDir() const {
	QTreeWidgetItem *current = tree_widget_img->currentItem();
	if (!current || !current->parent())
		return "";

	return current->parent()->text(0);
}

QString MainWindow::currentFile() const {
	QTreeWidgetItem *current = tree_widget_img->currentItem();
	if (!current || !current->parent())
		return "";

	return current->text(0);
}



void MainWindow::treeWidgetClicked() {

    QString iFile = currentFile();
    QString iDir = currentDir();
    if (iFile.isEmpty() || iDir.isEmpty())
        return;
    allDisconnnect(image_canvas);
    int index = getImageCanvas(iFile, image_canvas);
    updateConnect(image_canvas);
    tabWidget->setCurrentIndex(index);

}

void MainWindow::on_tree_widget_img_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    treeWidgetClicked();
}

void MainWindow::openDirectory()
{
    QSettings settings("Home", "PixelAnnotation");
    if (curr_open_dir.isEmpty()) {
        curr_open_dir = settings.value("last_opened_dir").toString();
    }

    QTreeWidgetItem *currentTreeDir = new QTreeWidgetItem(tree_widget_img);
    tree_widget_img->setItemExpanded(currentTreeDir, true);
    currentTreeDir->setText(0, curr_open_dir);

    QDir current_dir(curr_open_dir);
    QStringList files = current_dir.entryList();
    static QStringList ext_img = { "png","jpg","bmp","pgm","jpeg" ,"jpe" ,"jp2" ,"pbm" ,"ppm" ,"tiff" ,"tif" };
    for (short i = 0, total_f = files.size(); i < total_f; ++i) {
        if (files[i].size() < 4)
            continue;
        QString ext = files[i].section(".", -1, -1);
        bool is_image = false;
        for (short e = 0, total = ext_img.size(); e < total; ++e) {
            if (ext.toLower() == ext_img[e]) {
                is_image = true;
                break;
            }
        }
        if (!is_image)
            continue;

        if( files[i].toLower().indexOf("_mask.png") > -1)
            continue;

        QTreeWidgetItem *currentFile = new QTreeWidgetItem(currentTreeDir);
        currentFile->setText(0, files[i]);
    }
}

void MainWindow::on_actionOpenDir_triggered() {
    //statusBar()->clearMessage();

    QSettings settings("Home", "PixelAnnotation");

    QString openedDir = QFileDialog::getExistingDirectory(this, "Choose a directory to be read in", curr_open_dir);
	if (openedDir.isEmpty())
		return;

	curr_open_dir = openedDir;
    settings.setValue("last_opened_dir", curr_open_dir);
	
    openDirectory();
//	setWindowTitle("PixelAnnotation - " + openedDir);
}


void MainWindow::saveConfigFile() {
	QString file = QFileDialog::getSaveFileName(this, tr("Save Config File"), QString(), tr("JSon file (*.json)"));
	QFile save_file(file);
	if (!save_file.open(QIODevice::WriteOnly)) {
		qWarning("Couldn't open save file.");
		return ;
	}
	QJsonObject object;
	labels.write(object);
	QJsonDocument saveDoc(object);
	save_file.write(saveDoc.toJson());
	save_file.close();
}

void MainWindow::loadConfigFile() {
	QString file = QFileDialog::getOpenFileName(this, tr("Open Config File"), QString(), tr("JSon file (*.json)"));
	QFile open_file(file);
	if (!open_file.open(QIODevice::ReadOnly)) {
		qWarning("Couldn't open save file.");
		return;
	}
	QJsonObject object;
	QByteArray saveData = open_file.readAll();
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

	labels.clear();
	labels.read(loadDoc.object());
	open_file.close();

	loadConfigLabels();
    update();

}

void MainWindow::on_actionAbout_triggered() {
	AboutDialog *d = new AboutDialog(this);
	d->setModal(true);
	d->show();
}
