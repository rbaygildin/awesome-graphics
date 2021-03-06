#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "figures/tetrahedron.h"

#define PI 3.1459


double sliderToRotate(int tick) {
    return (75.0 - tick) / 75.0 * 3.1459;
}

double sliderToScale(int tick) {
    return (tick * 1.0 + 1.0) / 20.0;
}

double d2r(double degrees) {
    return degrees * PI / 180.0;
}

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->menuBar->setNativeMenuBar(false);
    color = QColor(0, 0, 0, 255);

    //Scene
    this->scene = new QGraphicsScene(ui->graphicsView);
    ui->graphicsView->setScene(scene);

    //Disable scrolling
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->showFullScreen();

//    QRect content = ui->graphicsView->contentsRect();
//    ui->graphicsView->setSceneRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height());

    //Create actions
    createActions();

    //Install event filter
    ui->graphicsView->scene()->installEventFilter(this);
}

void MainWindow::createActions() {
    //About
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(save()));
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(open()));

    //Context menu
    ui->graphicsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->graphicsView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showGraphicsViewMenu(QPoint)));
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showListWidgetMenu(QPoint)));

    //Rotation
    //Rotate x
    connect(ui->xRotateInput, SIGNAL(valueChanged(int)), this, SLOT(rotateX()));
    //Rotate y
    connect(ui->yRotateInput, SIGNAL(valueChanged(int)), this, SLOT(rotateY()));
    //Rotate z
    connect(ui->zRotateInput, SIGNAL(valueChanged(int)), this, SLOT(rotateZ()));

    //Remove lines
    connect(ui->removeLinesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(redraw()));

    //Scale
    //Scale x
    connect(ui->xScaleInput, SIGNAL(valueChanged(int)), this, SLOT(scaleX()));
    //Scale y
    connect(ui->yScaleInput, SIGNAL(valueChanged(int)), this, SLOT(scaleY()));
    //Scale y
    connect(ui->zScaleInput, SIGNAL(valueChanged(int)), this, SLOT(scaleZ()));

    //Move
    //Move x
    connect(ui->dxInp, SIGNAL(valueChanged(int)), this, SLOT(moveX()));
    //Move y
    connect(ui->dyInp, SIGNAL(valueChanged(int)), this, SLOT(moveY()));
    //Move z
    connect(ui->dzInp, SIGNAL(valueChanged(int)), this, SLOT(moveZ()));

    //FOV
    connect(ui->fovInp, SIGNAL(valueChanged(int)), this, SLOT(redraw()));

    //Select figure
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem * )), this, SLOT(select(QListWidgetItem * )));
}

void MainWindow::about() {
    QMessageBox msgBox;
    msgBox.setText(
            "<p>Разработка: Ярных Роман, студент БПИ141, 2017</p>"
                    "<p>1. Реализация проецирования 3D фигур на плоскость</p>"
                    "<p>2. Базовые преобразования (вращение, масштабирование, перемещение)</p>"
                    "<p>3. Удаление невидимых граней и линий</p>"
    );
    msgBox.exec();
}

void MainWindow::showGraphicsViewMenu(QPoint pos) {
    auto *menu = new QMenu(this);
    QAction *drawCubeAction = new QAction("Add cube", this);
    QAction *drawPyramidAction = new QAction("Add pyramid", this);
    QAction *drawOctahedronAction = new QAction("Add octahedron", this);
    QAction *drawIcosahedronAction = new QAction("Add icosahedron", this);
    QAction *drawTetrahedronAction = new QAction("Add tetrahedron", this);
    QAction *drawAction = new QAction("Redraw", this);
    QAction *clearAction = new QAction("Clear", this);
    QAction *restoreAction = new QAction("Restore", this);

    //Connect
    connect(drawCubeAction, &QAction::triggered, this, [this, pos]() {
        addCube(pos);
    });
    connect(drawPyramidAction, &QAction::triggered, this, [this, pos]() {
        addPyramid(pos);
    });
    connect(drawOctahedronAction, &QAction::triggered, this, [this, pos]() {
        addOctahedron(pos);
    });
    connect(drawIcosahedronAction, &QAction::triggered, this, [this, pos]() {
        addIcosahedron(pos);
    });
    connect(drawTetrahedronAction, &QAction::triggered, this, [this, pos]() {
        addTetrahedron(pos);
    });

    connect(drawAction, SIGNAL(triggered(bool)), this, SLOT(redraw()));
    connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(clear()));
    connect(restoreAction, SIGNAL(triggered(bool)), this, SLOT(restore()));

    //Add items
    menu->addAction(drawCubeAction);
    menu->addAction(drawPyramidAction);
    menu->addAction(drawOctahedronAction);
    menu->addAction(drawIcosahedronAction);
    menu->addAction(drawTetrahedronAction);
    menu->addAction(drawAction);
    menu->addAction(clearAction);
    menu->addAction(restoreAction);
    menu->popup(ui->graphicsView->viewport()->mapToGlobal(pos));
}

void MainWindow::redraw() {
    drawAxes();
    ui->graphicsView->setSceneRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height());
    sort(figures.begin(), figures.end(), [](const IFigure *a, const IFigure *b) -> bool {
        return const_cast<IFigure *>(a)->zIndex() < const_cast<IFigure *>(b)->zIndex();
    });
    if (ui->ortRadioBtn->isChecked()) {
        for (auto &figure : figures) {
            figure->paint(ui->removeLinesCheckBox->isChecked());
        }
    } else {
        for (auto &figure : figures) {
            figure->paint(ui->removeLinesCheckBox->isChecked(), -depth, 0.01, 100,
                          ui->graphicsView->height() * 1.0 / ui->graphicsView->width() * 1.0);
        }
    }
}

void MainWindow::save() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save figures"), "",
                                                    tr("Figures (*.json);;All files (*)"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, QString("Could not save a file"), file.errorString());
    }
    QJsonObject obj;
    QJsonArray array;
    for (auto &figure : figures) {
        array.append(figure->toJson());
    }
    obj.insert("figures", array);
    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
}

void MainWindow::open() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open figures"), "",
                                                    tr("Figures (*.json);;All files (*)"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QString("Could not to open a file"), file.errorString());
    }
    QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    clear();
    for (auto &&item : obj["figures"].toArray()) {
        QJsonObject figure = item.toObject();
        if (figure["type"] == "CUBE") {
            auto cube = new Cube(figure["edge"].toDouble(), scene);
            QJsonArray trArray = figure["transformations"].toArray();
            for (int i = 0; i < trArray.size(); i++)
                cube->transform(i, trArray.at(i).toDouble());
            figures.emplace_back(cube);
            QListWidgetItem *widgetItem = new QListWidgetItem("Cube");
            widgetItem->setData(Qt::UserRole, qVariantFromValue((void *) cube));
            ui->listWidget->addItem(widgetItem);
        } else if (figure["type"] == "PYRAMID") {
            auto pyramid = new Pyramid(figure["edge"].toDouble(), scene);
            QJsonArray trArray = figure["transformations"].toArray();
            for (int i = 0; i < trArray.size(); i++)
                pyramid->transform(i, trArray.at(i).toDouble());
            figures.emplace_back(pyramid);
            QListWidgetItem *widgetItem = new QListWidgetItem("Pyramid");
            widgetItem->setData(Qt::UserRole, qVariantFromValue((void *) pyramid));
            ui->listWidget->addItem(widgetItem);
        } else if (figure["type"] == "OCTAHEDRON") {
            auto octahedron = new Octahedron(figure["edge"].toDouble(), scene);
            QJsonArray trArray = figure["transformations"].toArray();
            for (int i = 0; i < trArray.size(); i++)
                octahedron->transform(i, trArray.at(i).toDouble());
            figures.emplace_back(octahedron);
            QListWidgetItem *widgetItem = new QListWidgetItem("Octahedron");
            widgetItem->setData(Qt::UserRole, qVariantFromValue((void *) octahedron));
            ui->listWidget->addItem(widgetItem);
        } else if (figure["type"] == "ICOSAHEDRON") {
            auto icosahedron = new Icosahedron(figure["edge"].toDouble(), scene);
            QJsonArray trArray = figure["transformations"].toArray();
            for (int i = 0; i < trArray.size(); i++)
                icosahedron->transform(i, trArray.at(i).toDouble());
            figures.emplace_back(icosahedron);
            QListWidgetItem *widgetItem = new QListWidgetItem("Icosahedron");
            widgetItem->setData(Qt::UserRole, qVariantFromValue((void *) icosahedron));
            ui->listWidget->addItem(widgetItem);
        } else if (figure["type"] == "TETRAHEDRON") {
            QJsonArray vArray = figure["vertex"].toArray();
            vector<PointF3d> vertex;
            for (int i = 0; i < 4; i++) {
                vertex.emplace_back(
                        PointF3d(
                                vArray.at(i * 4 + 0).toDouble(),
                                vArray.at(i * 4 + 1).toDouble(),
                                vArray.at(i * 4 + 2).toDouble()
                        )
                );
            }
            auto tetrahedron = new Tetrahedron(scene, vertex[0], vertex[1], vertex[2], vertex[3]);
            QJsonArray trArray = figure["transformations"].toArray();
            for (int i = 0; i < trArray.size(); i++)
                tetrahedron->transform(i, trArray.at(i).toDouble());
            figures.emplace_back(tetrahedron);
            QListWidgetItem *widgetItem = new QListWidgetItem("Tetrahedron");
            widgetItem->setData(Qt::UserRole, qVariantFromValue((void *) tetrahedron));
            ui->listWidget->addItem(widgetItem);
        }
    }
    redraw();
}

void MainWindow::addCube(QPoint pos) {
    double r = ui->rInput->text().toDouble();
    auto cube = new Cube(r, this->scene);
    QListWidgetItem *item = new QListWidgetItem("Cube");
    item->setData(Qt::UserRole, qVariantFromValue((void *) cube));
    ui->listWidget->addItem(item);
    figures.emplace_back(cube);
    redraw();
}

void MainWindow::addPyramid(QPoint pos) {
    double r = ui->rInput->text().toDouble();
    auto cube = new Pyramid(r, this->scene);
    QListWidgetItem *item = new QListWidgetItem("Pyramid");
    item->setData(Qt::UserRole, qVariantFromValue((void *) cube));
    ui->listWidget->addItem(item);
    figures.emplace_back(cube);
    redraw();
}

void MainWindow::addOctahedron(QPoint pos) {
    double r = ui->rInput->text().toDouble();
    auto cube = new Octahedron(r, this->scene);
    QListWidgetItem *item = new QListWidgetItem("Octahedron");
    item->setData(Qt::UserRole, qVariantFromValue((void *) cube));
    ui->listWidget->addItem(item);
    figures.emplace_back(cube);
    redraw();
}

void MainWindow::addIcosahedron(QPoint pos) {
    double r = ui->rInput->text().toDouble();
    auto cube = new Icosahedron(r, this->scene);
    QListWidgetItem *item = new QListWidgetItem("Icosahedron");
    item->setData(Qt::UserRole, qVariantFromValue((void *) cube));
    ui->listWidget->addItem(item);
    figures.emplace_back(cube);
    redraw();
}

void MainWindow::addTetrahedron(QPoint point) {
    auto *dialog = new TetrahedronInput(this);
    dialog->exec();
    auto tetrahedron = new Tetrahedron(scene, dialog->v1(), dialog->v2(), dialog->v3(), dialog->v4());
    QListWidgetItem *item = new QListWidgetItem("Tetrahedron");
    item->setData(Qt::UserRole, qVariantFromValue((void *) tetrahedron));
    ui->listWidget->addItem(item);
    figures.emplace_back(tetrahedron);
    redraw();
}

void MainWindow::drawAxes() {
    QPen pen(Qt::blue);
    this->scene->clear();
    double h = ui->graphicsView->height();
    double w = ui->graphicsView->width();
    this->scene->addLine(0, h / 2, w, h / 2,
                         pen);
    this->scene->addLine(w / 2, 0, w / 2, h,
                         pen);
}

MainWindow::~MainWindow() {
    for (auto &figure : figures)
        delete figure;
    delete ui;
}

void MainWindow::clear() {
    for (auto &figure : figures)
        delete figure;
    figures.clear();
    scene->clear();
    ui->listWidget->clear();
    selectedFigure = nullptr;
}


bool MainWindow::eventFilter(QObject *target, QEvent *event) {
    if(event->type() == QEvent::KeyPress){
        auto ev = dynamic_cast<QKeyEvent*>(event);
        if(ev->key() == Qt::Key_U) {
            depth += 20;
            redraw();
        }
        else if(ev->key() == Qt::Key_D) {
            depth = depth - 20;
            depth = depth < 0 ? 0 : depth;
            redraw();
        }
    }
    else if(target == ui->graphicsView->scene()){
        if(event->type() == QEvent::GraphicsSceneMousePress){
            auto ev = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            mousePoint = ev->scenePos();
        }
        else if(event->type() == QEvent::GraphicsSceneMouseRelease){
            auto ev = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            auto currentPoint = ev->scenePos();
            double rX = abs(currentPoint.x() - mousePoint.x());
            double rY = abs(currentPoint.y() - mousePoint.y());
            if(selectedFigure != nullptr){
                selectedFigure->transform(IFigure::RotateX, rX);
                selectedFigure->transform(IFigure::RotateY, rY);
                redraw();
            }
        }
    }
    return QMainWindow::eventFilter(target, event);
}

void MainWindow::restore() {
    for (auto &figure : figures)
        figure->restore();
    redraw();
}

void MainWindow::select(QListWidgetItem *item) {
    selectedFigure = (IFigure *) item->data(Qt::UserRole).value<void *>();
}

void MainWindow::rotateX() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::RotateX, sliderToRotate(ui->xRotateInput->value()));
        redraw();
    }
}

void MainWindow::rotateY() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::RotateY, sliderToRotate(ui->yRotateInput->value()));
        redraw();
    }
}

void MainWindow::rotateZ() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::RotateZ, sliderToRotate(ui->zRotateInput->value()));
        redraw();
    }
}

void MainWindow::scaleX() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::ScaleX, sliderToScale(ui->xScaleInput->value()));
        redraw();
    }
}

void MainWindow::scaleY() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::ScaleY, sliderToScale(ui->yScaleInput->value()));
        redraw();
    }
}

void MainWindow::scaleZ() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::ScaleZ, sliderToScale(ui->zScaleInput->value()));
        redraw();
    }
}

void MainWindow::moveX() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::TranslateX, (ui->dxInp->value() - 50) * 10);
        redraw();
    }
}

void MainWindow::moveY() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::TranslateY, (ui->dyInp->value() - 50) * 10);
        redraw();
    }
}

void MainWindow::moveZ() {
    if (selectedFigure != nullptr) {
        selectedFigure->transform(IFigure::TranslateZ, (ui->dzInp->value() - 50) * 10);
        redraw();
    }
}

void MainWindow::showListWidgetMenu(QPoint pos) {
    QPoint globalPos = ui->listWidget->mapToGlobal(pos);
    QMenu menu;
    menu.addAction("Delete", this, [this]() {
//        for (int i = 0; i < this->ui->listWidget->selectedItems().size(); i++) {
//            QListWidgetItem *item = this->ui->listWidget->takeItem(this->ui->listWidget->currentRow());
//            delete (IFigure*) item->data(Qt::UserRole).value<void *>();
//            delete item;
//        }
//        redraw();
    });
    menu.addAction("Restore", this, [this]() {
        for (int i = 0; i < this->ui->listWidget->selectedItems().size(); i++) {
            QListWidgetItem *item = ui->listWidget->item(i);
            ((IFigure *) item->data(Qt::UserRole).value<void *>())->restore();
        }
        redraw();
    });
    menu.exec(globalPos);
}
