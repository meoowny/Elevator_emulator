#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QWidget>
#include <QApplication>
#include <QLCDNumber>
#include <QLabel>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include "MyButton.h"
#include "Semaphore.h"

class Elevator : public QWidget
{
  Q_OBJECT;

public:
  explicit Elevator(QWidget *parent = 0, int floor = 20, int id = 0);
  ~Elevator();

  enum State { RUNNING, WAITING, OPENED, BROKEN, SPARE };
  enum TaskState { START, END, OPEN, CLOSE };

  signals:
  void opened();
  void finished(int floor);
  void changeState(TaskState state);
  void newFloor(int floor);

public slots:
  void onNewTarget(int floor);
  void on_OpenDoor_clicked()
  {
    // TODO: 优先级调整
    std::thread t(&Elevator::openDoor, this);
    t.detach();
  }
  void onStateChange(TaskState st);

private:
  MyButton **buttons;
  QPushButton *open_door_button;
  QPushButton *close_door_button;
  QLCDNumber *lcdNumber;
  QLabel *stateLabel;
  QFrame *board_line;

  int id;
  State state;
  const int total_floor;
  int current_floor;
  int waiting_target;
  // std::vector<int> target_floors;

  Semaphore work_semaphore;
  Semaphore door_semaphore;

  void setupUi();

  void moveTo(int n);
  void openDoor();
  void step(bool isUp);
  void waitPassenger();
};

#endif // ELEVATOR_H
