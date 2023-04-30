#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_MainWidget.h"
#include "MyListWidget.h"
#include "proxyserver.h"
#include <QTimer>

#define UPDATE_INTERVAL 2000        /* 更新GUI首页任务表格的时间间隔 */

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
    QTimer* timer;                  /* 定时器，用于更新GUI首页的任务表格 */

    static void proxyThread(ProxyServer* p);
    void setMenuBarBotton();        /* 设置菜单栏各按钮的连接关系 */

public slots:
    void updateTaskTable();         /* 自定义槽函数，定时更新GUI首页的任务表格 */
};
