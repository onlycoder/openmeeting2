#include "SeanButton.h"
#include <QDebug>

SeanButton::SeanButton(QWidget *parent=0)
	: QPushButton(parent)
{
	bTag = 0;
}

void SeanButton::setImageWithTag(unsigned long long tag){

	QPixmap *pixmap = NULL;   
	pixmap = new QPixmap(24, 24); 
	QString imStr = QString(":/setting/setting/0%1[24x24x8BPP].png").arg(3502+tag);

	pixmap->load(imStr);   
	QIcon *icon = new QIcon(*pixmap); 
	this->setIcon(*icon);
}
SeanButton::~SeanButton()
{

}
