#include "MainWidget.h"
#include <iostream>
#include "mysql.h"

using namespace std;

MainWidget::MainWidget(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWidgetClass())
{
	ui->setupUi(this);
	setWindowTitle("代理服务器");

	proxy = new ProxyServer();
	timer = new QTimer(this);
	workThreads.emplace_back(proxyThread, proxy);
	QObject::connect(timer, &QTimer::timeout, this, &MainWidget::updateTaskTable);
	timer->start(UPDATE_INTERVAL);
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
* 定时更新GUI首页的任务表格
*/
void MainWidget::updateTaskTable()
{
	printf("\n\033[1;31m==========> updating starts <==========\033[0m\n");
	int taskCount = 0;
	Task* task = nullptr;
	std::unique_lock<std::mutex> lck(mtx_tasks);
	taskCount = proxy->tasks.size();	/* 记录这个时刻的任务数量 */
	lck.unlock();
	printf("\n\033[1;31m==========> I released the lock <==========\033[0m\n");

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
	printf("\n\033[1;31m==========> updating ends <==========\033[0m\n");
}
