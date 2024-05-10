#ifndef MY_BUTTON_H
#define MY_BUTTON_H

#include <QPushButton>
#include <QThread>
#include <thread>

class MyButton : public QPushButton
{
  Q_OBJECT;

public:
  enum Direction { UP, DOWN, TARGET };

  explicit MyButton(QWidget *parent = 0, int l = 1, Direction d = TARGET)
    : QPushButton(parent)
    , floor(l)
    , dir(d)
  {
    connect(this, &QPushButton::clicked, this, &MyButton::onClick);
    setStyleSheet(QString("\
      QPushButton {\
        background-color: lightblue;\
      }\
      QPushButton:hover {\
        background-color: #66ccff;\
      } "));
  }

  signals:
  void newTarget(int floor);

public slots:
  void onClick()
  {
    emit newTarget(floor);
  }

private:
  int floor;
  Direction dir;
};

#endif // MY_BUTTON_H
