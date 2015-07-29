#include "autoupdate.h"
#include <QtGui/QApplication>
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QFile>
#include <QMessageBox>
int main(int argc, char *argv[])
{
	
	QApplication a(argc, argv);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
	QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
	if(argc!=2){
		QMessageBox::warning(NULL,"提示",QString("参数不正确,AutoUpdate.exe url"));
		return -1;
	}
	AutoUpdate w;
	w.show();
	w.StartUpdate(QString(argv[1]));
	return a.exec();
}
