#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_MainWidget.h"
#include "MyListWidget.h"
#include "proxyserver.h"
#include <QTimer>
#include <QStringList>

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
    QTimer* timer;              /* 定时器，用于更新GUI首页的任务表格 */
    ProxyServer* proxy;
    std::vector<std::thread> workThreads;

    // 设置菜单栏各按钮的连接关系
    void setMenuBarBotton();
    static void proxyThread(ProxyServer* p);
    QString compTraffic(unsigned long long bytes);

public slots:
    void updateTaskTable();     /* 自定义槽函数，定时更新GUI首页的任务表格 */
};
