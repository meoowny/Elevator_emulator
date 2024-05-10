#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "controller.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
  QObject::connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onRun);
}

void MainWindow::onRun()
{
  int floor = ui->spinBox_2->value();
  int elevator = ui->spinBox->value();
  Controller *c = new Controller(nullptr, floor, elevator);
  c->show();
  this->close();
}

MainWindow::~MainWindow()
{
    delete ui;
}
