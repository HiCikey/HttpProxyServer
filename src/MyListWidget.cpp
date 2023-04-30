#include "MyListWidget.h"
#include <QMessageBox>

MyListWidget::MyListWidget(ProxyServer* p, int m, QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::MyListWidgetClass())
	, mode(m)
{
	ui->setupUi(this);
	resize(375, 375);
	move(760, 200);
	setWindowModality(Qt::ApplicationModal);
	setAttribute(Qt::WA_DeleteOnClose);
	proxy = p;
}

MyListWidget::~MyListWidget()
{
	delete ui;
}

void MyListWidget::setDetail()
{
	std::set<std::string> black_list;

	// 通过代理获取当前黑名单
	if (mode == MODEL_IP) {
		setWindowTitle("客户端IP黑名单");
		black_list = proxy->getIpList();
	}
	else if (mode == MODEL_DOMAIN) {
		setWindowTitle("访问的域名黑名单");
		black_list = proxy->getDomainList();
	}
	else if (mode == MODEL_TYPE) {
		setWindowTitle("传输文件类型黑名单");
		black_list = proxy->getTypeList();
	}

	// 打印黑名单到窗口
	for (std::string str : black_list)
		ui->listWidget->addItem(new QListWidgetItem(QString::fromStdString(str)));

	// 设置按钮点击事件
	QObject::connect(ui->btn_add, &QPushButton::clicked, this, &MyListWidget::buttonAddClicked);
	QObject::connect(ui->btn_edit, &QPushButton::clicked, this, &MyListWidget::buttonEditClicked);
	QObject::connect(ui->btn_del, &QPushButton::clicked, this, &MyListWidget::buttonDeleteClicked);
}


void MyListWidget::buttonAddClicked()
{
	std::string str = ui->lineEdit->text().toStdString();
	if (str.empty()) {
		QMessageBox::warning(this, "警告", "请先输入您要添加的内容");
		return;
	}

	// 标记添加操作是否成功
	bool success = false;
	if (mode == MODEL_IP) {
		if (proxy->addIp(str))
			success = true;
	}
	else if (mode == MODEL_DOMAIN) {
		if (proxy->addDomain(str))
			success = true;
	}
	else if (mode == MODEL_TYPE) {
		if (proxy->addType(str))
			success = true;
	}
	// 检查添加操作是否成功并进行相应操作
	if (success)
		ui->listWidget->addItem(ui->lineEdit->text());
	else
		QMessageBox::warning(this, "警告", "添加的内容已存在");
	ui->lineEdit->setText("");
}


void MyListWidget::buttonEditClicked()
{
	// 黑名单为空
	if (ui->listWidget->currentRow() == -1)
		return;

	std::string curStr = ui->lineEdit->text().toStdString();
	if (curStr.empty()) {
		QMessageBox::warning(this, "警告", "请先输入修改后的内容");
		return;
	}

	// 先添加修改后的内容，再删除原来的内容
	bool success = false;
	std::string prevStr = ui->listWidget->currentItem()->text().toStdString();
	if (mode == MODEL_IP){
		if (proxy->addIp(curStr)) {
			proxy->deleteIp(prevStr);
			success = true;
		}
	}
	else if (mode == MODEL_DOMAIN){
		if (proxy->addDomain(curStr)) {
			proxy->deleteDomain(prevStr);
			success = true;
		}
	}
	else if (mode == MODEL_TYPE){
		if (proxy->addType(curStr)) {
			proxy->deleteType(prevStr);
			success = true;
		}
	}

	// 检查修改操作是否成功并进行相应操作
	if (success)
		ui->listWidget->currentItem()->setText(QString::fromStdString(curStr));
	else
		QMessageBox::warning(this, "警告", "修改后的内容已存在");
	ui->lineEdit->setText("");
}


void MyListWidget::buttonDeleteClicked()
{
	// 黑名单为空
	if (ui->listWidget->currentRow() == -1)
		return;

	// 调用函数删除数据库中对应项
	QListWidgetItem* item = ui->listWidget->takeItem(ui->listWidget->currentRow());
	if (mode == model::MODEL_IP)
		proxy->deleteIp(item->text().toStdString());
	else if (mode == model::MODEL_DOMAIN)
		proxy->deleteDomain(item->text().toStdString());
	else if (mode == model::MODEL_TYPE)
		proxy->deleteType(item->text().toStdString());
	delete item;
}
