#include "controller.h"
#include <string>

Controller::Controller(QWidget *parent, int floor, int total_elevator)
  : QWidget(parent)
  , total_floor(floor)
  , total_elevator(total_elevator)
{
  setupUi();
  setAttribute(Qt::WA_DeleteOnClose);
}

Controller::~Controller()
{
  // Elevator 对象由 deleteLater 回收
  delete[] elevators;
  for (int i = 0; i < 2 * total_floor; i++)
    delete buttons[i];
  delete[] buttons;
  for (int i = 0; i < total_floor; i++)
    delete labels[i];
  delete[] labels;
}

void Controller::newTarget(int floor, MyButton::Direction dir)
{
  elevators[1]->onNewTarget(floor);
}

void Controller::setupUi()
{
  if (objectName().isEmpty())
    setObjectName("MainController");
  resize(140 * (total_elevator + 1) - 5 + 40, (total_floor / 2 + 4) * 30 - 5);

  elevators = new Elevator* [total_elevator];
  for (int i = 0; i < total_elevator; i++) {
    elevators[i] = new Elevator(this, total_floor, i);
    elevators[i]->setGeometry(QRect(135 * i + 5, 5, 135, (total_floor / 2 + 4) * 30 - 5));
  }

  buttons = new MyButton* [2 * total_floor];
  int base_x = 135 * total_elevator + 10;
  for (int i = 0; i < 2 * total_floor; i++) {
    buttons[i] = new MyButton(this, i / 2 + 1, i % 2 ? MyButton::UP : MyButton::DOWN);
    buttons[i]->setObjectName("Button" + std::to_string(i / 2) + (i % 2 ? "UP" : "DOWN"));
    buttons[i]->setGeometry(QRect(base_x + i % 4 * (30 + 20) - (i / 2 % 2) * 10, i / 4 * (25 + 10) + 5, 30, 25));
    buttons[i]->setText(i % 2 ? "/\\" : "\\/");
  }

  labels = new QLabel* [total_floor];
  for (int i = 0; i < total_floor; i++) {
    labels[i] = new QLabel(this);
    labels[i]->setObjectName("FloorLabel" + std::to_string(i + 1));
    labels[i]->setText(std::to_string(i + 1).c_str());
    labels[i]->setAlignment(Qt::AlignCenter);
    labels[i]->setGeometry(QRect(base_x + 30 + i % 2 * 90, i / 2 * (25 + 10) + 5, 20, 25));
    labels[i]->setStyleSheet("background-color: #cccccc;");
  }

  for (int i = 0; i < total_elevator; i++) {
    QObject::connect(this, &QWidget::destroyed, elevators[i], &QWidget::deleteLater);
  }
  for (int i = 0; i < total_floor * 2; i++) {
    QObject::connect(buttons[i], &MyButton::newTarget, this, &Controller::newTarget);
  }
}

