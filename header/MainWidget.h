#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_MainWidget.h"
#include "proxyserver.h"
#include <QString>
#include <QTimer>
#include <QTableWidget>
#include <QTableWidgetItem>

#define UPDATE_INTERVAL 1000        /* 更新GUI首页任务表格的时间间隔 */

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
    ProxyServer* proxy;
    //QTimer* timer;

    static void proxyThread(ProxyServer* p);

public slots:
    //void updateTaskTable();         /* 定时更新GUI首页的任务表格 */
};
