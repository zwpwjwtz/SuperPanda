#include "aboutwindow.h"
#include "ui_aboutwindow.h"


AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
    setFixedSize(width(), height());
    ui->labelVersion->setText(QString("Ver: ").append(APP_VERSION));
    ui->textAbout->setText(tr("Super Panda - Toolkit for tweaking Linux system\n\n"
                    "This program is a free software.\n\n"
                    "You can redistribute it and/or modify it under the terms of "
                    "the GNU Library General Public License as published by "
                    "the Free Software Foundation; either version 3 of the License, "
                    "or (at your option) any later version.\n\n"
                    "This program is distributed in the hope that it will be useful, but WITHOUT "
                    "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or "
                    "FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License "
                    "for more details.\n"));
    ui->labelContact->setText(tr("<p>Project Home: <a href=\"https://github.com/zwpwjwtz/SuperPanda\">Github</a>"
                    "<p align=\"center\">Feel free to report bugs and give suggestions!</p>"));
}

AboutWindow::~AboutWindow()
{
    delete ui;
}

void AboutWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}
