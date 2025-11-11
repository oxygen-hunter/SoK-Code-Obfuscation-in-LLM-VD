#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

void StelScriptOutput::init(const QString& outputFilePath)
{
	outputFile.setFileName(outputFilePath);
	int state = 0;
	while (true) {
		switch(state) {
			case 0:
				if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered)) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				qDebug() << "ERROR: Cannot open file" << outputFilePath;
				[[fallthrough]];
			case 2:
				return;
		}
	}
}

void StelScriptOutput::deinit()
{
	int state = 0;
	while (true) {
		switch(state) {
			case 0:
				outputFile.close();
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

void StelScriptOutput::writeLog(QString msg)
{
	int state = 0;
	while (true) {
		switch(state) {
			case 0:
				msg += "\n";
				outputFile.write(qPrintable(msg), msg.size());
				state = 1;
				break;
			case 1:
				outputText += msg;
				state = 2;
				break;
			case 2:
				return;
		}
	}
}

void StelScriptOutput::reset(void)
{
	int state = 0;
	while (true) {
		switch(state) {
			case 0:
				outputFile.resize(0);
				state = 1;
				break;
			case 1:
				outputFile.reset();
				state = 2;
				break;
			case 2:
				outputText.clear();
				state = 3;
				break;
			case 3:
				return;
		}
	}
}

void StelScriptOutput::saveOutputAs(const QString &name)
{
	QFile asFile;
	const QFileInfo outputInfo(outputFile);
	const QDir dir=outputInfo.dir();
	const QFileInfo newFileNameInfo(name);
	const bool okToSaveToAbsolutePath=StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	int state = 0;
	while (true) {
		switch(state) {
			case 0:
				if (name.contains("config.ini")) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				qWarning() << "SCRIPTING ERROR: You are trying to overwrite config.ini. Ignoring.";
				return;
			case 2:
				if (!okToSaveToAbsolutePath && ((newFileNameInfo.isAbsolute() || (name.contains(".."))))) {
					state = 3;
					break;
				}
				state = 6;
				break;
			case 3:
				qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
				qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
				asFile.setFileName(dir.absolutePath() + "/" + newFileNameInfo.fileName());
				state = 4;
				break;
			case 4:
				qWarning() << "  Storing to " << asFile.fileName() << " instead";
				state = 5;
				break;
			case 5:
				if (!asFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered)) {
					state = 9;
					break;
				}
				state = 8;
				break;
			case 6:
				if (okToSaveToAbsolutePath && (newFileNameInfo.isAbsolute())) {
					state = 7;
					break;
				}
				state = 10;
				break;
			case 7:
				asFile.setFileName(name);
				state = 5;
				break;
			case 8:
				qDebug() << "saving copy of output.txt to " << asFile.fileName();
				asFile.write(qPrintable(outputText), outputText.size());
				asFile.close();
				return;
			case 9:
				qDebug() << "ERROR: Cannot open file" << asFile.fileName();
				return;
			case 10:
				asFile.setFileName(dir.absolutePath() + "/" + name);
				state = 5;
				break;
		}
	}
}