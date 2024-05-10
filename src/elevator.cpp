#include "elevator.h"
#include "qobject.h"

Elevator::Elevator(QWidget *parent, int floor, int id)
  : QWidget(parent)
  , total_floor(floor)
  , current_floor(1)
  , state(WAITING)
  , waiting_target(0)
  , id(id)
{
  setupUi();
  setAttribute(Qt::WA_DeleteOnClose);
}

Elevator::~Elevator()
{
  for (int i = 0; i < total_floor; i++)
    delete buttons[i];
  delete[] buttons;
  delete close_button;
  delete lcdNumber;
  delete stateLabel;
  delete board_line;
}

void Elevator::onStateChange(TaskState st)
{
  // TODO: 应该把多线程逻辑放在这里
  if (st == START and state == WAITING) {
    state = RUNNING;
    stateLabel->setStyleSheet("background: red;");
  }
  else if (st == START and state == RUNNING) {
    waiting_target++;
  }
  else if (st == END and state == RUNNING) {
    if (waiting_target == 0) {
      state = WAITING;
      stateLabel->setStyleSheet("background: grey;");
    }
    else {
      waiting_target--;
    }
  }
  else if (st == END and state == WAITING) {
    throw "Expected branch";
  }
}

void Elevator::onNewTarget(int floor)
{
  if (floor != current_floor) {
    std::thread t1(&Elevator::sleep, this, floor);
    t1.detach();
  }
}

void Elevator::sleep(int floor)
{
  emit changeState(START);

  QThread::sleep(std::abs(floor - current_floor));

  current_floor = floor;
  emit newFloor(floor);
  emit changeState(END);
}

void Elevator::setupUi()
{
  if (objectName().isEmpty())
    setObjectName("Elevator" + std::to_string(id));
  this->resize(135, (total_floor / 2 + 4) * 30 - 5);

  buttons = new MyButton* [total_floor];
  for (int i = 0; i < total_floor; i++) {
    buttons[i] = new MyButton(this, i + 1);
    buttons[i]->setObjectName("Button" + std::to_string(i));
    buttons[i]->setGeometry(QRect(i % 2 * 65 + 5, (i + 2) / 2 * 30 + 5, 60, 25));
    buttons[i]->setText(QCoreApplication::translate(objectName().toStdString().c_str(), std::to_string(i + 1).c_str(), nullptr));
  }

  board_line = new QFrame(this);
  board_line->setObjectName("boardLine");
  board_line->setGeometry(QRect(133, 5, 3, (total_floor / 2 + 3) * 30 - 5));
  board_line->setFrameShadow(QFrame::Plain);
  board_line->setLineWidth(5);
  board_line->setFrameShape(QFrame::VLine);

  close_button = new QPushButton(this);
  close_button->setObjectName("CloseButton");
  close_button->setGeometry(QRect(5, (total_floor + 3) / 2 * 30 + 5, 60, 25));
  close_button->setText("关闭");

  stateLabel = new QLabel(this);
  stateLabel->setObjectName("stateLabel");
  stateLabel->setGeometry(QRect(5, 5, 125, 25));
  stateLabel->setStyleSheet("background: grey;");

  lcdNumber = new QLCDNumber(this);
  lcdNumber->setObjectName("Layer");
  lcdNumber->setGeometry(QRect(45, 5, 45, 25));
  lcdNumber->setSegmentStyle(QLCDNumber::Flat);
  lcdNumber->setStyleSheet("background: white");
  lcdNumber->display(current_floor);

  void(QLCDNumber::*display)(int) = &QLCDNumber::display;
  for (int i = 0; i < total_floor; i++) {
    QObject::connect(buttons[i], &MyButton::newTarget, this, &Elevator::onNewTarget);
  }
  QObject::connect(this, &Elevator::changeState, this, &Elevator::onStateChange);
  QObject::connect(this, &Elevator::newFloor, lcdNumber, display);
  QMetaObject::connectSlotsByName(this);
}
