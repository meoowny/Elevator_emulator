#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QWidget>
#include <QApplication>
#include <QLCDNumber>
#include <QLabel>
#include <thread>
#include <string>
#include "elevatorbutton.h"
#include "Semaphore.h"

class Elevator : public QWidget
{
  Q_OBJECT;

public:
  // 用于标识电梯状态，电梯开门时为 OPENED，报修时为 BROKEN，关门空闲时 为 WAITING，关门在楼层间移动时为 WAITING
  enum State { RUNNING, WAITING, OPENED, BROKEN };
  enum TaskState { START, END, OPEN, CLOSE };

  explicit Elevator(QWidget *parent = 0, int floor = 20, int id = 0);
  ~Elevator();

  int getCurrentFloor() const { return current_floor; }
  ElevatorButton::Direction getDirections() const { return direction; }
  State getState() const { return state; }

  signals:
  // 该信号槽用于线程间沟通
  void changeState(TaskState state);
  void arrive(int floor);

public slots:
  void onNewTarget(int floor);
  void on_OpenDoor_clicked();
  void on_CloseDoor_clicked();
  void on_AlarmButton_clicked();
  void onStateChange(TaskState st);

private:
  const int id;
  const int total_floor;
  std::atomic<State> state;
  std::atomic<int> current_floor;
  std::atomic<int> waiting_target;
  std::atomic<ElevatorButton::Direction> direction;

  Semaphore work_semaphore;
  Semaphore door_semaphore;
  std::timed_mutex door_mutex; // 用于开门关门
                               // 初始化时锁上，使用 try_lock_for
                               // 需要关门时解锁 door_mutex 即可

  void setupUi();

  void moveTo(int n);
  void openDoor();
  void step(bool isUp);
  void waitPassenger();

  std::vector<ElevatorButton*> buttons;
  QPushButton *open_door_button;
  QPushButton *close_door_button;
  QPushButton *alarm_button;
  QLCDNumber *lcdNumber;
  QLabel *stateLabel;
  QFrame *board_line;
};

#endif // ELEVATOR_H
