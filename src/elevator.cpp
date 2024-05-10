#include "elevator.h"

using namespace std::chrono_literals;

Elevator::Elevator(QWidget *parent, int floor, int id)
  : QWidget(parent)
  , total_floor(floor)
  , current_floor(1)
  , state(SPARE)
  , waiting_target(0)
  , id(id)
  , work_semaphore(Semaphore(1))
  , door_semaphore(Semaphore(1))
{
  setupUi();
  setAttribute(Qt::WA_DeleteOnClose);
}

Elevator::~Elevator()
{
  for (int i = 0; i < total_floor; i++)
    delete buttons[i];
  delete[] buttons;
  delete open_door_button;
  delete lcdNumber;
  delete stateLabel;
  delete board_line;
}

void Elevator::onStateChange(TaskState st)
{
  if (st == START and state == SPARE) {
    state = RUNNING;
    stateLabel->setStyleSheet("background: red;");
  }
  else if (st == START and state == RUNNING) {
    waiting_target++;
  }
  else if (st == END and state == RUNNING) {
    if (waiting_target == 0) {
      state = SPARE;
      stateLabel->setStyleSheet("background: grey;");
    }
    else {
      waiting_target--;
    }
  }
  else if (st == OPEN and state == SPARE) {
    state = WAITING;
    stateLabel->setStyleSheet("background: green;");
  }
  else if (st == CLOSE and state == WAITING) {
    state = SPARE;
    stateLabel->setStyleSheet("background: grey;");
  }
  else {
    throw "Expected branch";
  }
}

void Elevator::onNewTarget(int floor)
{
  std::thread t1(&Elevator::moveTo, this, floor);
  t1.detach();
}

void Elevator::step(bool isUp)
{
  std::this_thread::sleep_for(1s);
  if (isUp)
    current_floor++;
  else
    current_floor--;
  emit newFloor(current_floor);
}

void Elevator::waitPassenger()
{
  std::this_thread::sleep_for(2s);
}

void Elevator::openDoor()
{
  door_semaphore.wait();
  emit changeState(OPEN);
  waitPassenger();
  emit changeState(CLOSE);
  door_semaphore.signal();
}

void Elevator::moveTo(int floor)
{
  // WIP: 初步解决，但开门键的响应存在延迟
  work_semaphore.wait();
  // work_count++;
  // // if (work_count == 1)
    door_semaphore.wait();
  // work_semaphore.signal();

  emit changeState(START);
  bool isUp = floor > current_floor;
  for (int i = current_floor; isUp and i < floor or not isUp and i > floor;) {
    step(isUp);
    i += isUp ? 1 : -1;
  }
  emit changeState(END);

  // work_semaphore.wait();
  // work_count--;
  // // if (work_count == 0)
    door_semaphore.signal();
  work_semaphore.signal();

  openDoor();
  // emit changeState(OPEN);
  // waitPassenger();
  // emit changeState(CLOSE);
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

  open_door_button = new QPushButton(this);
  open_door_button->setObjectName("OpenDoor");
  open_door_button->setGeometry(QRect(5, (total_floor + 3) / 2 * 30 + 5, 60, 25));
  open_door_button->setText("\xE5\xBC\x80\xE9\x97\xA8"); // 开门

  close_door_button = new QPushButton(this);
  close_door_button->setObjectName("CloseDoor");
  close_door_button->setGeometry(QRect(5 + 65, (total_floor + 3) / 2 * 30 + 5, 60, 25));
  close_door_button->setText("关门"); // 关门

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
