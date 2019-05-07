#pragma once

#include <QWidget>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <string>
#include <cstdint>
#include <thread>
#include "Assembler.h"
#include "USB.h"
#include "MessageFormats.h"
#include "ui_RichArduino.h"

using namespace std;

class RichArduino : public QWidget
{
	Q_OBJECT

public:
	RichArduino(QWidget *parent = Q_NULLPTR);
	~RichArduino();

    friend void autoReconnect(RichArduino &richArduino, USB *usb);

private:
    Ui::RichArduinoClass *ui;
    USB *usb;
    Assembler assembler;

    QString openFile = "";

private slots:
	void on_fileExplore_clicked();	//select filepath in file explorer
	void on_reconnect_clicked();	//reconnects to RichArduino
	void on_open_clicked();			//open specified file
    void on_saveAsm_clicked();		//save changes to asm code
    void on_saveAsmAs_clicked();    //save asm code to file
    void on_saveBin_clicked();      //save machine code to file
	void on_upload_clicked();		//sends assembled code over USB
    void on_reset_clicked();        //reset RichArduino
};
