#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QWidget>
#include "elevator.h"

class Controller : public QWidget
{
  Q_OBJECT;

public:
  explicit Controller(QWidget *parent = 0, int floor = 20, int total_elevator = 5);
  ~Controller();

  signals:
  void none();

public slots:

private:
  int total_floor;
  int total_elevator;
  Elevator **elevators;
  MyButton **buttons;
  QLabel **labels;

  void setupUi();
};

#endif // CONTROLLER_H
