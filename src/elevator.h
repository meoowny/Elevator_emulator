#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <QWidget>
#include <QApplication>
#include <QLCDNumber>
#include <QLabel>
#include <thread>
#include <string>
#include <vector>
#include "MyButton.h"

class Elevator : public QWidget
{
  Q_OBJECT;

public:
  explicit Elevator(QWidget *parent = 0, int floor = 20, int id = 0);
  ~Elevator();

  enum State { RUNNING, WAITING, OPENED, BROKEN };
  enum TaskState { START, END };

  signals:
  void opened();
  void finished(int floor);
  void changeState(TaskState state);
  void newFloor(int floor);

public slots:
  void onNewTarget(int floor);
  void on_CloseButton_clicked()
  {
    close();
  }
  void onStateChange(TaskState st);

private:
  MyButton **buttons;
  QPushButton *close_button;
  QLCDNumber *lcdNumber;
  QLabel *stateLabel;
  QFrame *board_line;

  int id;
  State state;
  const int total_floor;
  int current_floor;
  int waiting_target;
  // std::vector<int> target_floors;

  void setupUi();

  void sleep(int n);
};

#endif // ELEVATOR_H
