#include "RichArduino.h"

RichArduino::RichArduino(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::RichArduinoClass)
{
    ui->setupUi(this);
	
    do {
		usb = new USB();
    } while (!usb->initialized());
}

RichArduino::~RichArduino() {
    delete ui;
	delete usb;
}

void RichArduino::on_reconnect_clicked() {
    if(usb != nullptr) delete usb;
    do {
		usb = new USB();
    } while (!usb->initialized());
}

void RichArduino::on_fileExplore_clicked() {
	QString filePath = QFileDialog::getOpenFileName(this, tr("Load asm file"), "", tr("asm file (*.asm)"));

    if (!filePath.isEmpty()) ui->filePathField->setText(filePath);
}

void RichArduino::on_open_clicked() {
    QString filePath = ui->filePathField->displayText();

	if (!filePath.isEmpty()) {

		QFile asmFile(filePath);

		if (asmFile.open(QIODevice::ReadOnly)) {

			QString asmCode(asmFile.readAll());
            ui->programField->setText(asmCode);
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
		}
		
	}
}

void RichArduino::on_read_clicked() {
	readPt in = nullptr;
	int bytes;

	usb->read(in, bytes);

	if (!in) {
		if (bytes != -1) {
			uint8_t* data = (uint8_t*)in;

			QString output = "";

			for (int i = 0; i < bytes; ++i) {
				output.append(QString::number(*(data + i), 16));
				QString end = (i % 4 == 0) ? "\n" : " ";
				output.append(end);
			}

            ui->outputField->setText(output);
		}
		delete[] in;
	}
}

void RichArduino::on_upload_clicked() {
	Assembler assembler;

    QString code = ui->programField->toPlainText();
	if (!code.isEmpty()) {
		std::string text(code.toLatin1().data());
		QString machineCode(assembler.assemble(text).c_str());

		QVector<QString> machineWords = machineCode.split('\n').toVector();

		size_t numWords = machineWords.size() - 2;	//top instruction all 0's and last element will be an empty string

		uint32_t *machineCodeData = new uint32_t[numWords + 1];

		machineCodeData[0] = numWords;
		
		for (int i = 1; i < numWords + 1; ++i) {
			uint32_t instr = machineWords.at(i).toULong(nullptr, 16);

			machineCodeData[i] = instr;
		}

		for (int i = 0; i < /*30*/numWords + 1; ++i) {
			cout << setw(8) << setfill('0') << hex << machineCodeData[i] << endl;
		}

		usb->send(machineCodeData, sizeof(uint32_t) * (numWords + 1));

		delete[] machineCodeData;
	}
}
