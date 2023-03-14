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
	workThreads.emplace_back(proxyThread);
}

MainWidget::~MainWidget()
{
	delete ui;
	for(int i=0;i<workThreads.size();i++)
		workThreads[i].join();
}

void MainWidget::proxyThread()
{
	ProxyServer* proxy = new ProxyServer();
	proxy->proxyStartUp();
	delete proxy;
}
