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
#include <QtConcurrent/QtConcurrent>

#include "about_dialog.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setupUi(this);

    list_label->setSpacing(1);
    image_canvas = nullptr;
    save_action = new QAction(tr("&Save current image"), this);
    save_action->setIcon(QIcon(":/save.png"));
    close_tab_action = new QAction(tr("&Close current tab"), this);
    undo_action = new QAction(tr("&Undo"), this);
    redo_action = new QAction(tr("&Redo"), this);
    undo_action->setShortcuts(QKeySequence::Undo);
    redo_action->setShortcuts(QKeySequence::Redo);

    next_image_action = new QAction(tr("Next image"), this);
    next_image_action->setShortcuts({QKeySequence("E")});

    prev_image_action = new QAction(tr("Previous image"), this);
    prev_image_action->setShortcuts({QKeySequence("Q")});

    save_action->setShortcut(Qt::CTRL + Qt::Key_S);
    close_tab_action->setShortcut(Qt::CTRL + Qt::Key_W);
    undo_action->setEnabled(false);
    redo_action->setEnabled(false);

    menuFile->addAction(save_action);
    menuFile->addSeparator();
    menuFile->addAction(this->actionQuit);
    menuFile->addSeparator();
    menuFile->addAction(actionOpen_config_file);
    menuFile->addAction(actionSave_config_file);

    menuEdit->addAction(close_tab_action);
    menuEdit->addAction(undo_action);
    menuEdit->addAction(redo_action);
    menuEdit->addSeparator();
    menuEdit->addAction(next_image_action);
    menuEdit->addAction(prev_image_action);

    this->checkbox_border_ws->setVisible(false);

    tabWidget->clear();

    connect(button_watershed      , SIGNAL(released())                        , this, SLOT(runWatershed()  ));

    connect(next_image_action      , SIGNAL(triggered())                        , this, SLOT(pickNextImage()  ));
    connect(prev_image_action      , SIGNAL(triggered())                        , this, SLOT(pickPrevImage()  ));

    connect(actionOpen_config_file, SIGNAL(triggered())                       , this, SLOT(loadConfigFile()));
    connect(actionSave_config_file, SIGNAL(triggered())                       , this, SLOT(saveConfigFile()));
    connect(close_tab_action      , SIGNAL(triggered())                       , this, SLOT(closeCurrentTab()));
    connect(tabWidget             , SIGNAL(tabCloseRequested(int))            , this, SLOT(closeTab(int)   ));
    connect(tabWidget             , SIGNAL(currentChanged(int))               , this, SLOT(updateConnect(int)));
    connect(tree_widget_img       , SIGNAL(itemClicked(QTreeWidgetItem *,int)), this, SLOT(treeWidgetClicked()));

    labels = defaultLabels();

    loadConfigLabels();

    connect(list_label, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(changeLabel(QListWidgetItem*, QListWidgetItem*)));

    list_label->setEnabled(false);
    openDirectory();

    connect(&_timer_autosave,
            &QTimer::timeout,
            this,
            &MainWindow::autosave);

    _timer_autosave.start(AUTOSAVE_TIME_SECONDS * 1000);
    list_label->installEventFilter(this);
    this->dockWidget_3->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);


    QSettings settings("Home", "PixelAnnotation");
    QString file = settings.value("last_json").toString();

    if (file.isEmpty() || !QFile::exists(file)) {
        file = QDir::currentPath() + "/default.json";
    }
    loadJSON(file);


    _last_network_path = settings.value("last_segmentator_path").toString();

    // Configuring status label
    labelStatusBar->setFixedWidth(260);
    labelStatusBar->setFixedHeight(60);
    labelStatusBar->setWordWrap(true);
    //

    loadNetwork(_last_network_path);
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

void MainWindow::pickNextImage() {
    const auto tuple = getCurrentItemIndecies();

    int current_parent_index = std::get<0>(tuple);
    int current_child_index = std::get<1>(tuple);

    if ((current_child_index + 1) >= tree_widget_img->topLevelItem(current_parent_index)->childCount()) {
        if ((current_parent_index + 1) >= tree_widget_img->topLevelItemCount()) {
            QMessageBox::information(this, "No more images", "There is no next image! Well done");
            return;
        } else {
            if (!tree_widget_img->topLevelItem(current_parent_index + 1)->childCount()) {
                QMessageBox::information(this, "No more images", "There is no next image! Well done");
                return;
            }
            current_parent_index += 1;
            current_child_index = 0;
        }
    } else {
        current_child_index += 1;
    }

    closeCurrentTab();
    auto item = tree_widget_img->topLevelItem(current_parent_index)->child(current_child_index);
    tree_widget_img->setCurrentItem(item);
}

void MainWindow::pickPrevImage()
{
    const auto tuple = getCurrentItemIndecies();

    int current_parent_index = std::get<0>(tuple);
    int current_child_index = std::get<1>(tuple);

    if ((current_child_index - 1) < 0) {
        if ((current_parent_index - 1) < 0) {
            QMessageBox::information(this, "No more images", "There is no previous image! Well done");
            return;
        } else {
            if (!tree_widget_img->topLevelItem(current_parent_index + 1)->childCount()) {
                QMessageBox::information(this, "No more images", "There is no previous image! Well done");
                return;
            }
            current_parent_index -= 1;
            current_child_index = 0;
        }
    } else {
        current_child_index -= 1;
    }

    closeCurrentTab();
    auto item = tree_widget_img->topLevelItem(current_parent_index)->child(current_child_index);
    tree_widget_img->setCurrentItem(item);
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
    auto * old_canvas = image_canvas;
    if (iFile.isEmpty() || iDir.isEmpty())
        return;

    allDisconnnect(image_canvas);

    int index = getImageCanvas(iFile, image_canvas);

    updateConnect(image_canvas);
    tabWidget->setCurrentIndex(index);

    if (old_canvas) {
        image_canvas->setSizePen(old_canvas->getPenSize());
        image_canvas->setId(old_canvas->getId());
    }

}

void MainWindow::setStatus(const QString &text)
{
    labelStatusBar->setText(text);
}

void MainWindow::loadNetwork(const QString &filename)
{
    if (!filename.isEmpty()) {
        QString *error_str = new QString();

        auto future = QtConcurrent::run(&loadSegmentator, filename, error_str);
        QFutureWatcher<TensorflowSegmentator*> *watcher = new QFutureWatcher<TensorflowSegmentator*>(this);

        _network_load_in_process = true;
        setStatus("Network loading...");


        connect(watcher, &QFutureWatcher<TensorflowSegmentator*>::finished, this,
                [this, error_str]() {

            _network_load_in_process = false;

            auto s = (QFutureWatcher<TensorflowSegmentator*>*)(sender());
            auto future = s->future();
            if (error_str->isEmpty()) {
                if (segmentator) {
                    delete segmentator;
                    segmentator = nullptr;
                }
                this->segmentator = future.result();
                setStatus("Network load finished successfuly");
                delete error_str;
            } else {
                setStatus("Network load failed: " + *error_str);
                delete error_str;
                return;
            }

            _last_network_path = QString::fromStdString(segmentator->getPath());

            QSettings settings("Home", "PixelAnnotation");
            settings.setValue("last_segmentator_path", _last_network_path);

            segmentator->warmUp();

            auto label_names = segmentator->getLabelNames();
            auto label_colors = segmentator->getLabelColours();

            if (!label_names.size() || !label_colors.size()) {
                return;
            }

            labels.clear();

            if (label_names.size() != label_colors.size()) {
                return;
            }

            for (short i = 0; i < label_names.size(); ++i) {
                QString name = QString::fromStdString(label_names[i]);
                auto cv_color = label_colors[i];
                // bgr -> rgb
                QColor color = QColor(cv_color[2], cv_color[1], cv_color[0]);

                LabelInfo new_info;
                new_info.id = i;
                new_info.name = name;
                new_info.color = color;
                new_info.categorie = name;
                new_info.id_categorie = i;

                labels[name] = new_info;
            }

            loadConfigLabels();

        });

        watcher->setFuture(future);
    }
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

std::tuple<int, int> MainWindow::getCurrentItemIndecies()
{
    const QString current_file = currentFile();
    const QString current_dir = currentDir();

    int current_parent_index = -1;
    int current_child_index = -1;

    // find parent index
    for (int index = 0; index < tree_widget_img->topLevelItemCount(); ++index) {
        auto current_item = tree_widget_img->topLevelItem(index);
        auto current_text = current_item->text(0);

        if (current_text == current_dir) {
            current_parent_index = index;

            // find child index
            for (int child_index = 0; child_index < current_item->childCount(); ++child_index) {
                auto child_item = current_item->child(child_index);
                auto child_text = child_item->text(0);
                if (child_text == current_file) {
                    current_child_index = child_index;
                    break;
                }
            }

            break;
        }
    }

    return std::tuple<int, int>(current_parent_index, current_child_index);
}

void MainWindow::loadJSON(const QString &file)
{
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

    QSettings settings("Home", "PixelAnnotation");
    settings.setValue("last_json", file);

    loadConfigLabels();
    update();
}

QString MainWindow::getLastNetworkPath() const
{
    return _last_network_path;
}

TensorflowSegmentator* loadSegmentator(const QString &filename, QString* error)
{
    if (filename.isEmpty()) return nullptr;

    TensorflowSegmentator* segmentator;

    segmentator = new TensorflowSegmentator;
    segmentator->setAllowSoftPlacement(false);
    segmentator->setCpuOnly(true);
    bool success = segmentator->load(filename.toStdString());

    if (!success) {
        *error = "Can not load this model file";
        delete segmentator;
        segmentator = nullptr;
        return nullptr;
    }

    if (segmentator->getInputWidth() == 0 || segmentator->getInputWidth() == 0) {
        segmentator->setInputWidth(800);
        segmentator->setInputHeight(800);
    }

    if (segmentator->getOutputNodeName().empty()) {
        *error = "Seems no output node name specified";
        delete segmentator;
        segmentator = nullptr;
        return nullptr;
    }


    return segmentator;

}

void MainWindow::on_actionOpenDir_triggered() {
    //statusBar()->clearMessage();

    QSettings settings("Home", "PixelAnnotation");

    QString openedDir = QFileDialog::getExistingDirectory(this, "Choose a directory to be read in", curr_open_dir);
    if (openedDir.isEmpty())
        return;

    if (curr_open_dir == openedDir) {
        QMessageBox::information(this, "Already opened", "This directory is alread opened!");
        return;
    }

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
    loadJSON(file);

}

void MainWindow::on_actionAbout_triggered() {
    AboutDialog *d = new AboutDialog(this);
    d->setModal(true);
    d->show();
}

void MainWindow::on_button_NeuralNetwork_clicked()
{
    if (_network_load_in_process) {
        QMessageBox::warning(this, "Error", "Sorry, network is loading, please wait!");
        return;
    }
    if (_network_inference_in_process) {
        QMessageBox::warning(this, "Error", "Sorry, network is inferencing, please wait!!");
        return;
    }

    if (!segmentator) {
        QMessageBox::critical(this, "Not network loaded", "Please, load network in Tools first");
        return;
    }

    if (!image_canvas) return;

    QImage canvas = image_canvas->getImage();
    cv::Mat mat_canvas = qImage2Mat(canvas);
    std::vector<cv::Mat> inf_vec = {mat_canvas};

    _network_inference_in_process = true;
    setStatus("Inferencing network...");

    auto future = QtConcurrent::run(this->segmentator,
                                    static_cast<std::string(TensorflowSegmentator::*)(const std::vector<cv::Mat>&)>(&TensorflowSegmentator::inference),
                                    inf_vec);
    QFutureWatcher<std::string> *watcher = new QFutureWatcher<std::string>(this);

    connect(watcher, &QFutureWatcher<TensorflowSegmentator*>::finished, this,
            [this, mat_canvas, canvas]() {


        auto s = (QFutureWatcher<std::string>*)(sender());
        auto future = s->future();
        auto err_str = future.result();
        _network_inference_in_process = false;

        if (err_str != "OK") {
            this->setStatus("Can't inference network: " + QString::fromStdString(err_str));
            return;
        }

        auto outputs = segmentator->getOutputIndices();
        if (!outputs.size()) {
            this->setStatus("Network resturn empty output, inference failed");
            return;
        }
        this->setStatus("Network inference finished successful");

        auto mt = segmentator->getOutputColored()[0];

        cv::Mat indices = outputs[0];
        cv::resize(indices, indices, mat_canvas.size(), 0, 0, cv::INTER_NEAREST);
        cv::cvtColor(indices, indices, cv::COLOR_GRAY2BGR);

        QImage qt_indices = mat2QImage(indices);

        ImageMask new_mask(canvas.size());
        new_mask.id = qt_indices;

        new_mask.updateColor(id_labels);
        image_canvas->setMask(new_mask);

        image_canvas->update();

        image_canvas->addUndo();
        setStarAtNameOfTab(true);


    });
    watcher->setFuture(future);
}

void MainWindow::on_actionLoad_network_pb_triggered()
{
    if (_network_load_in_process) {
        QMessageBox::warning(this, "Error", "Sorry, some network load already is in process, wait its finished!");
        return;
    }
    if (_network_inference_in_process) {
        QMessageBox::warning(this, "Error", "Sorry, network is inferencing, please wait!!");
        return;
    }
    QString filename = QFileDialog::getOpenFileName(this, "Choose network model", QDir::current().path(), "(*.pb)");
    if (filename.isEmpty()) return;

    loadNetwork(filename);
}
