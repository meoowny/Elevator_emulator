#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QWidget>
#include <QApplication>
#include <QLCDNumber>
#include <QLabel>
#include <thread>
#include <atomic>
#include <string>
#include "MyButton.h"
#include "Semaphore.h"

class Elevator : public QWidget
{
  Q_OBJECT;

public:
  explicit Elevator(QWidget *parent = 0, int floor = 20, int id = 0);
  ~Elevator();

  enum State { RUNNING, WAITING, OPENED, BROKEN };
  enum TaskState { START, END, OPEN, CLOSE, ALARM };

  signals:
  void opened();
  void finished(int floor);
  void changeState(TaskState state);
  void newFloor(int floor);

public slots:
  void onNewTarget(int floor);
  void on_OpenDoor_clicked();
  void on_CloseDoor_clicked();
  void on_AlarmButton_clicked();
  void onStateChange(TaskState st);

private:
  const int id;
  std::atomic<State> state;
  const int total_floor;
  std::atomic<int> current_floor;
  std::atomic<int> waiting_target;
  std::atomic<MyButton::Direction> direction;

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

  MyButton **buttons;
  QPushButton *open_door_button;
  QPushButton *close_door_button;
  QPushButton *alarm_button;
  QLCDNumber *lcdNumber;
  QLabel *stateLabel;
  QFrame *board_line;
};

#endif // ELEVATOR_H
