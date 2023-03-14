#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_MainWidget.h"
#include "proxyserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidgetClass; };
QT_END_NAMESPACE

class MainWidget : public QMainWindow
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
private:
    Ui::MainWidgetClass *ui;
    std::vector<std::thread> workThreads;
    static void proxyThread();
};
