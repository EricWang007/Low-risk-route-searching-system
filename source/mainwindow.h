#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //ÖØÐ´paintEventÊÂ¼þ£¬»­±³¾°Í¼
    void paintEvent(QPaintEvent *);
    void chooseCity1();
    void chooseCity2();
    void chooseCity3();
    void chooseCity4();
    void chooseCity5();
    void chooseCity6();
    void chooseCity7();
    void chooseCity8();
    void chooseCity9();
    void chooseCity10();
    void startDfs();
    void connectBut();
    void disconnectBut();
    int linex1, liney1, linex2, liney2;
    int vehicleX, vehicleY;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

