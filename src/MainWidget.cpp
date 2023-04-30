#include "MainWidget.h"
#include <stdio.h>

MainWidget::MainWidget(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWidgetClass())
{
	ui->setupUi(this);
	setWindowTitle("代理服务器");
	move(250, 190);

	/* 创建代理和定时器并启动 */
	proxy = new ProxyServer();
	timer = new QTimer(this);
	workThreads.emplace_back(proxyThread, proxy);
	QObject::connect(timer, &QTimer::timeout, this, &MainWidget::updateTaskTable);
	timer->start(UPDATE_INTERVAL);

	setMenuBarBotton();
}

MainWidget::~MainWidget()
{
	for (int i = 0; i < workThreads.size(); i++)
		workThreads[i].join();
	if (timer->isActive()) timer->stop();
	delete ui;
	delete proxy;
	delete timer;
}

void MainWidget::proxyThread(ProxyServer* p)
{
	p->proxyStartUp();
}

/*
* 设置菜单栏各按钮的连接关系
*/
void MainWidget::setMenuBarBotton()
{
	QObject::connect(ui->action_ip, &QAction::triggered, [=]() {
		MyListWidget* lw = new MyListWidget(proxy->ruleManager, MODEL_IP);
		lw->setDetail();
		lw->show();
		});

	QObject::connect(ui->action_domain, &QAction::triggered, [=]() {
		MyListWidget* lw = new MyListWidget(proxy->ruleManager, MODEL_DOMAIN);
		lw->setDetail();
		lw->show();
		});

	QObject::connect(ui->action_type, &QAction::triggered, [=]() {
		MyListWidget* lw = new MyListWidget(proxy->ruleManager, MODEL_TYPE);
		lw->setDetail();
		lw->show();
		});
}

/*
* 定时更新GUI首页的任务表格
*/
void MainWidget::updateTaskTable()
{
	int taskCount = 0;
	Task* task = nullptr;
	std::unique_lock<std::mutex> lck(mtx_tasks);
	taskCount = proxy->tasks.size();	/* 记录这个时刻的任务数量 */
	lck.unlock();

	ui->tableWidget->setRowCount(taskCount);
	for (int i = 0; i < taskCount; i++)
	{
		lck.lock();
		task = proxy->tasks.at(i);
		lck.unlock();

		/* 已结束的任务通知代理删除 */
		if (task->isEnd) {
			proxy->deleteTask(i);
			taskCount--;
			i--;
		}
		/* 未结束且已获得两端信息的任务展示在GUI首页 */
		else if (task->isReady) {
			ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(task->source)));
			ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(task->dest)));
			ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(task->up_bytes)));
			ui->tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(task->down_bytes)));
			ui->tableWidget->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(task->domain)));
		}
	}
}
