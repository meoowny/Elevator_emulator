#include "controller.h"
#include <algorithm>
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

void Controller::newTarget(int floor, ElevatorButton::Direction dir)
{
  // 一个简单的多电梯调度实现
  int target_elevator = -1;
  int min_distance = total_floor + 2;
  int candidate_elevator = -1;
  int candidate_min_distance = total_floor + 2;
  for (int i = 0; i < total_elevator; i++) {
    // 电梯损坏则跳过，目标楼层已处于等待状态则结束查找
    if (elevators[i]->getState() == Elevator::BROKEN)
      continue;
    else if (elevators[i]->inWaitingList(floor))
      return;

    int distance = elevators[i]->getCurrentFloor() - floor;
    ElevatorButton::Direction demandDirection;
    if (distance > 0) {
      demandDirection = ElevatorButton::DOWN;
    }
    else {
      demandDirection = ElevatorButton::UP;
      distance = -distance;
    }

    // 同向或空闲
    if ((elevators[i]->getDirections() == demandDirection
      or elevators[i]->getDirections() == ElevatorButton::TARGET)
      and distance < min_distance) {
      target_elevator = i;
      min_distance = distance;
    }
    // 反向
    else if (elevators[i]->getDirections() != demandDirection
      and elevators[i]->getDirections() != ElevatorButton::TARGET
      and distance < candidate_min_distance) {
      candidate_elevator = i;
      candidate_min_distance = distance;
    }
  }

  // 首选同向或空闲的电梯
  if (target_elevator != -1) {
    elevators[target_elevator]->onNewTarget(floor);
  }
  else if (candidate_elevator != -1) {
    elevators[candidate_elevator]->onNewTarget(floor);
  }
  else {
    throw "Unexpected branch.";
  }
}

void Controller::setupUi()
{
  int base_x = 135 * total_elevator + 15;
  int base_y = (total_floor + 1) / 2 * (25 + 10) - 25;

  if (objectName().isEmpty())
    setObjectName("MainController");
  resize(135 * total_elevator + 5 + 80 * 2 + 10 + 20, std::max(((total_floor + 1) / 2 + 4) * 30 - 5, base_y + 25 + 10));
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/tju.png"), QSize(), QIcon::Normal, QIcon::Off);
  setWindowIcon(icon);

  elevators = new Elevator* [total_elevator];
  for (int i = 0; i < total_elevator; i++) {
    elevators[i] = new Elevator(this, total_floor, i);
    elevators[i]->setGeometry(QRect(135 * i + 5, 5, 135, (total_floor / 2 + 4) * 30 - 5));
  }

  buttons = new ElevatorButton* [2 * total_floor];
  for (int i = 0; i < 2 * total_floor; i++) {
    buttons[i] = new ElevatorButton(this, i / 2 + 1, i % 2 ? ElevatorButton::UP : ElevatorButton::DOWN);
    buttons[i]->setObjectName("Button" + std::to_string(i / 2) + (i % 2 ? "UP" : "DOWN"));
    buttons[i]->setGeometry(QRect(base_x + i % 4 * (30 + 20) - (i / 2 % 2) * 10, base_y - (i / 4 * (25 + 10)), 30, 25));
    buttons[i]->setText(i % 2 ? "/\\" : "\\/");
    // 禁用顶楼和一楼的部分按键
    if (i == 0 or i == 2 * total_floor - 1)
      buttons[i]->setEnabled(false);
  }

  labels = new QLabel* [total_floor];
  for (int i = 0; i < total_floor; i++) {
    labels[i] = new QLabel(this);
    labels[i]->setObjectName("FloorLabel" + std::to_string(i + 1));
    labels[i]->setText(std::to_string(i + 1).c_str());
    labels[i]->setAlignment(Qt::AlignCenter);
    labels[i]->setGeometry(QRect(base_x + 30 + i % 2 * 90, base_y - (i / 2 * (25 + 10)), 20, 25));
    labels[i]->setStyleSheet("background-color: #cccccc;");
  }

  for (int i = 0; i < total_elevator; i++) {
    QObject::connect(this, &QWidget::destroyed, elevators[i], &QWidget::deleteLater);
  }
  for (int i = 0; i < total_floor * 2; i++) {
    QObject::connect(buttons[i], &ElevatorButton::newTarget, this, &Controller::newTarget);
    for (int j = 0; j < total_elevator; j++) {
      QObject::connect(elevators[j], &Elevator::arrive, buttons[i], &ElevatorButton::onArrive);
    }
  }
}

