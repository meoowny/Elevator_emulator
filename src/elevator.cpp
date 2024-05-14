#include "elevator.h"

using namespace std::chrono_literals;

/**********************
 * 构造函数与析构函数 *
 **********************/

Elevator::Elevator(QWidget *parent, int floor, int id)
  : QWidget(parent)
  , total_floor(floor)
  , current_floor(1)
  , state(WAITING)
  , waiting_target(0)
  , id(id)
  , door_semaphore(Semaphore(1))
  , direction(ElevatorButton::TARGET)
{
  setupUi();
  setAttribute(Qt::WA_DeleteOnClose);
  door_mutex.lock();
}

Elevator::~Elevator()
{
  for (int i = 0; i < total_floor; i++)
    delete buttons[i];
  delete open_door_button;
  delete alarm_button;
  delete lcdNumber;
  delete stateLabel;
  delete board_line;
}

/**********
 * 槽函数 *
 **********/

void Elevator::onNewTarget(int floor)
{
  if (state == BROKEN)
    return;
  if (floor == current_floor and state == OPENED) {
    emit arrive(floor);
    return;
  }
  buttons[floor - 1]->lightupOnly();
  // 若此时电梯在运行，则跳过，会在到达任一目标楼层后在调度中被执行
  if (work_mutex.try_lock()) {
    std::thread(&Elevator::moveTo, this, floor)
      .detach();
  }
}

void Elevator::on_OpenDoor_clicked()
{
  if (state == BROKEN or state != WAITING)
    return;
  std::thread(&Elevator::openDoor, this)
    .detach();
}

void Elevator::on_CloseDoor_clicked()
{
  if (state == BROKEN or state != OPENED)
    return;
  door_mutex.unlock();
}

void Elevator::on_AlarmButton_clicked()
{
  if (state == BROKEN) {
    state = WAITING;
    stateLabel->setStyleSheet("background: grey;");
  }
  else {
    state = BROKEN;
    stateLabel->setStyleSheet("background: red;");
  }
  // 故障时清除当前电梯的所有目标楼层的按键显示
  // 外部按键需要重新按
  for (int i = 0; i < total_floor; i++) {
    if (buttons[i]->isWaiting()) {
      arrive(i + 1);
    }
  }
}

void Elevator::onStateChange(TaskState st)
{
  if (st == START and state == WAITING) {
    state = RUNNING;
    stateLabel->setStyleSheet("background: orange;");
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
  else if (st == OPEN and state == WAITING) {
    state = OPENED;
    stateLabel->setStyleSheet("background: green;");
  }
  else if (st == CLOSE and state == OPENED) {
    state = WAITING;
    stateLabel->setStyleSheet("background: grey;");
  }
  else if (state == BROKEN) {
  }
  else {
    throw "Unexpected branch";
  }
}

/******************************
 * 私有函数，用于实现电梯逻辑 *
 ******************************/

// 电梯移动模拟函数，用于新建线程
void Elevator::moveTo(int floor)
{
  door_semaphore.wait();

  // 逐楼层移动，中途遇到报修则释放信号量并终止
  emit changeState(START);
  bool isUp = floor > current_floor;
  direction = isUp ? ElevatorButton::UP : ElevatorButton::DOWN;
  for (int i = current_floor; isUp and i < floor or not isUp and i > floor;) {
    if (state == BROKEN) {
      door_semaphore.signal();
      work_mutex.unlock();
      return;
    }
    step(isUp);
    i += isUp ? 1 : -1;

    // 若当前层有在等电梯，则直接开门并将当前层作为目标层
    if (buttons[i - 1]->isWaiting()) {
      floor = i;
      break;
    }
  }
  // direction = ElevatorButton::TARGET;
  emit changeState(END);

  // 移动到目标楼层后向按钮发送状态更新的信号
  emit arrive(floor);

  // 到达目标楼层后开门一段时间等待乘客
  emit changeState(OPEN);
  waitPassenger();
  emit changeState(CLOSE);

  door_semaphore.signal();
  work_mutex.unlock();

  nextFloor();
}

// 当前任务结束后，寻找下一个目标楼层
void Elevator::nextFloor()
{
  // 单电梯调度逻辑，首先选择当前方向的目标楼层，确保不会出现饥饿问题
  for (int up = current_floor, down = current_floor; up <= total_floor or down > 0; up++, down--) {
    // if (buttons[i]->isWaiting()) {
    //   onNewTarget(i + 1);
    //   return;
    // }
    if (direction == ElevatorButton::DOWN
      and down > 0
      and buttons[down - 1]->isWaiting()) {
      onNewTarget(down);
      return;
    }
    else if (direction == ElevatorButton::UP
      and up <= total_floor
      and buttons[up - 1]->isWaiting()) {
      onNewTarget(up);
      return;
    }
  }
  for (int up = current_floor, down = current_floor; up <= total_floor or down > 0; up++, down--) {
    if (down > 0
      and buttons[down - 1]->isWaiting()) {
      onNewTarget(down);
      return;
    }
    else if (up <= total_floor
      and buttons[up - 1]->isWaiting()) {
      onNewTarget(up);
      return;
    }
  }
  // 无未处理楼层时，置电梯运行方向为 TARGET，即无方向
  direction = ElevatorButton::TARGET;
}

// 电梯开门模拟函数，用于新建线程
void Elevator::openDoor()
{
  door_semaphore.wait();
  emit changeState(OPEN);
  waitPassenger();
  emit changeState(CLOSE);
  door_semaphore.signal();

  nextFloor();
}

// 用于模拟电梯的单层移动
void Elevator::step(bool isUp)
{
  // 休眠 0.7 秒模拟电梯在楼层间的移动
  std::this_thread::sleep_for(0.7s);
  if (isUp)
    current_floor++;
  else
    current_floor--;
  lcdNumber->display(current_floor);
}

// 用于模拟开门等待乘客
void Elevator::waitPassenger()
{
  // 若中途收到关门指令，则结束阻塞状态，否则阻塞两秒作为等待模拟
  if (door_mutex.try_lock_for(2s))
    return;
}

/***********************
 * UI 设置与信号槽绑定 *
 ***********************/

void Elevator::setupUi()
{
  if (objectName().isEmpty())
    setObjectName("Elevator" + std::to_string(id));
  this->resize(135, ((total_floor + 1) / 2 + 5) * 30 - 5);

  // TODO: 奇数楼层显示 bug
  int base_y = (total_floor + 3) / 2 * 30 + 5;
  buttons.resize(total_floor);
  for (int i = 0; i < total_floor; i++) {
    buttons[i] = new ElevatorButton(this, i + 1);
    buttons[i]->setObjectName("Button" + std::to_string(i));
    buttons[i]->setGeometry(QRect(i % 2 * 65 + 5, base_y - ((i + 2) / 2 * 30), 60, 25));
    buttons[i]->setText(QCoreApplication::translate(
      objectName().toStdString().c_str(),
      std::to_string(i + 1).c_str(),
      nullptr));
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
  close_door_button->setText("关门");

  alarm_button = new QPushButton(this);
  alarm_button->setObjectName("AlarmButton");
  alarm_button->setGeometry(QRect(5, (total_floor + 5) / 2 * 30 + 5, 125, 25));
  alarm_button->setText("报修");
  alarm_button->setStyleSheet("background: #fcc;");

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
    QObject::connect(buttons[i], &ElevatorButton::newTarget, this, &Elevator::onNewTarget);
    QObject::connect(this, &Elevator::arrive, buttons[i], &ElevatorButton::onArrive);
  }
  QObject::connect(this, &Elevator::changeState, this, &Elevator::onStateChange);
  QMetaObject::connectSlotsByName(this);
}
