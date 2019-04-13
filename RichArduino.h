#pragma once

#include <QWidget>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <string>
#include <cstdint>
#include "Assembler.h"
#include "USB.h"
#include "ui_RichArduino.h"

class RichArduino : public QWidget
{
	Q_OBJECT

public:
	RichArduino(QWidget *parent = Q_NULLPTR);
	~RichArduino();

private:
    Ui::RichArduinoClass *ui;
    Assembler *assembler;
    USB *usb;

private slots:
	void on_fileExplore_clicked();	//select filepath in file explorer
	void on_reconnect_clicked();	//reconnects to RichArduino
	void on_open_clicked();			//open specified file
	void on_saveAsm_clicked();			//save asm code to file
    void on_saveBin_clicked();			//save machine code to file
	void on_read_clicked();			//assemble code and show in output
	void on_upload_clicked();		//sends assembled code over USB
};
