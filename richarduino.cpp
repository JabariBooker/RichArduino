#include "RichArduino.h"

RichArduino::RichArduino(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::RichArduinoClass)
{
    ui->setupUi(this);

    string message;
    usb = new USB(message);

    QString mes(message.c_str());
    ui->outputField->textCursor().insertHtml(mes);
}

RichArduino::~RichArduino() {
    delete ui;
	delete usb;
}

void RichArduino::on_reconnect_clicked() {
    if(usb != nullptr) delete usb;

    string message;

    usb = new USB(message);

    QString mes(message.c_str());
    ui->outputField->textCursor().insertHtml(mes);
    ui->outputField->ensureCursorVisible();
}

void RichArduino::on_reset_clicked(){
//    string message;
//    usb->reset(message);

//    QString mes(message.c_str());
//    ui->outputField->textCursor().insertHtml(mes);
//    ui->outputField->ensureCursorVisible();
}

void RichArduino::on_fileExplore_clicked() {
	QString filePath = QFileDialog::getOpenFileName(this, tr("Load asm file"), "", tr("asm file (*.asm)"));

    if (!filePath.isEmpty()) ui->filePathField->setText(filePath);
    on_open_clicked();
}

void RichArduino::on_open_clicked() {
    QString filePath = ui->filePathField->displayText();

	if (!filePath.isEmpty()) {

		QFile asmFile(filePath);

		if (asmFile.open(QIODevice::ReadOnly)) {

			QString asmCode(asmFile.readAll());
            ui->programField->setText(asmCode);
		}
        else{
            QString message = qMesError + "Failed to open: " + filePath + qMesEnd;
            ui->outputField->textCursor().insertHtml(message);
            ui->outputField->ensureCursorVisible();
        }
	}
}

void RichArduino::on_saveAsm_clicked() {
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save asm file"), "", tr("asm file (*.asm)"));

	if (!filePath.isEmpty()) {
		QFile outFile(filePath);

		if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QTextStream out(&outFile);
            out << (ui->programField->toPlainText());
            QString message = qMesSuccess + "Saved assembly code to: " + filePath + qMesEnd;
            ui->outputField->textCursor().insertHtml(message);
            ui->outputField->ensureCursorVisible();
		}
		
	}
}

void RichArduino::on_saveBin_clicked(){
    QString code = ui->programField->toPlainText();
    if (!code.isEmpty()) {
        string text(code.toLatin1().data()),
               message;
        QString machineCode(assembler.assemble(text, message).c_str());

        if(machineCode == "failed"){
            QString mes(message.c_str());
            ui->outputField->textCursor().insertHtml(mes);
            ui->outputField->ensureCursorVisible();
            return;
        }

        QString filePath = QFileDialog::getSaveFileName(this, tr("Save bin file"), "", tr("bin file (*.bin)"));
        if(!filePath.isEmpty()){
            QFile outFile(filePath);

            if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&outFile);
                out << machineCode;
                QString message = qMesSuccess + "Saved machine code to: " + filePath + qMesEnd;
                ui->outputField->textCursor().insertHtml(message);
                ui->outputField->ensureCursorVisible();
            }
        }
    }
}

void RichArduino::on_read_clicked() {
	readPt in = nullptr;
	int bytes;

    string message;

    bool success = usb->read(in, bytes, message);

    QString mes(message.c_str());
    ui->outputField->textCursor().insertHtml(mes);

    if (success) {
		if (bytes != -1) {
			uint8_t* data = (uint8_t*)in;

			QString output = "";

			for (int i = 0; i < bytes; ++i) {
				output.append(QString::number(*(data + i), 16));
				QString end = (i % 4 == 0) ? "\n" : " ";
				output.append(end);
            }

            output = qMesNormal + output + qMesEnd;
            ui->outputField->textCursor().insertHtml(output);
            ui->outputField->ensureCursorVisible();
		}
		delete[] in;
	}
}

void RichArduino::on_upload_clicked() {
    QString code = ui->programField->toPlainText();
	if (!code.isEmpty()) {
        string text(code.toLatin1().data()), message;

        QString machineCode(assembler.assemble(text, message).c_str());

        if(machineCode == "failed"){
            QString mes(message.c_str());
            ui->outputField->textCursor().insertHtml(mes);
            ui->outputField->ensureCursorVisible();
            return;
        }

		QVector<QString> machineWords = machineCode.split('\n').toVector();

        int numWords = machineWords.size();

		uint32_t *machineCodeData = new uint32_t[numWords + 1];

        machineCodeData[0] = numWords;

        for (int i = 1; i < numWords + 1; ++i) {
            QString word = machineWords.at(i-1);

            if(!word.isEmpty()){
                machineCodeData[i] = word.toULong(nullptr, 16);
            }
        }

//        cout << numWords + 1 << " lines" << endl;

//        for (int i = 0; i < numWords + 1; ++i) {
//            cout << setw(8) << setfill('0') << hex << machineCodeData[i] << endl;
//        }

        usb->send(machineCodeData, sizeof(uint32_t) * (numWords + 1), message);

        QString mes(message.c_str());
        ui->outputField->textCursor().insertHtml(mes);
        ui->outputField->ensureCursorVisible();

		delete[] machineCodeData;
	}
}
