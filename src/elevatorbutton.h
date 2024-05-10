#ifndef MY_BUTTON_H
#define MY_BUTTON_H

#include <QPushButton>

class ElevatorButton : public QPushButton
{
  Q_OBJECT;

public:
  enum Direction { UP, DOWN, TARGET };

  explicit ElevatorButton(QWidget *parent = 0, int l = 1, Direction d = TARGET)
    : QPushButton(parent)
    , floor(l)
    , dir(d)
    , waiting(false)
  {
    connect(this, &QPushButton::clicked, this, &ElevatorButton::onClick);
    setStyleSheet(QString("\
      QPushButton {\
        background-color: lightblue;\
      }\
      QPushButton:hover {\
        background-color: #66ccff;\
      } "));
  }

  bool getStatus() const { return waiting; }

  signals:
  void newTarget(int floor, Direction dir = TARGET);

public slots:
  void onClick()
  {
    if (not waiting) {
      lightupOnly();
      emit newTarget(floor);
    }
  }

  void onArrive(int f)
  {
    if (f != floor)
      return;
    waiting = false;
    setStyleSheet("background-color: lightblue;");
  }

  void lightupOnly()
  {
    waiting = true;
    setStyleSheet("background-color: #4ad;");
  }

private:
  int floor;
  Direction dir;
  bool waiting;
};

#endif // MY_BUTTON_H