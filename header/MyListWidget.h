#pragma once

#include <QWidget>
#include "ui_MyListWidget.h"
#include "proxyserver.h"
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MyListWidgetClass; };
QT_END_NAMESPACE

class MyListWidget : public QWidget
{
	Q_OBJECT

public:
	MyListWidget(ProxyServer* p, int m, QWidget* parent = nullptr);
	~MyListWidget();

	void setDetail();
private:
	Ui::MyListWidgetClass* ui;

	ProxyServer* proxy;
	int mode;
public slots:
	void buttonAddClicked();
	void buttonEditClicked();
	void buttonDeleteClicked();
};
