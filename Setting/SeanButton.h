#ifndef SEANBUTTON_H
#define SEANBUTTON_H

#include <QPushButton>
#include <QPixmap>
#include <QWidget>

class SeanButton : public QPushButton
{
	Q_OBJECT

public:
	SeanButton(QWidget *parent);
	~SeanButton();
	void setImageWithTag(unsigned long long tag);
	int bTag;
private:

};

#endif // SEANBUTTON_H
