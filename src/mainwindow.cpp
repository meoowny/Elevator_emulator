#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "controller.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  QObject::connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onRun);
}

void MainWindow::onRun()
{
  int floor = ui->floorSpinBox->value();
  int elevator = ui->elevatorSpinBox->value();
  Controller *c = new Controller(nullptr, floor, elevator);
  c->show();
  this->close();
}

MainWindow::~MainWindow()
{
  delete ui;
}
