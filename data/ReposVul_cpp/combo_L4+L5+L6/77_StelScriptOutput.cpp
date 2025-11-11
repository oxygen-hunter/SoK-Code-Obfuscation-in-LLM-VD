#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

// Init static variables.
QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

void StelScriptOutput::init(const QString& outputFilePath)
{
	outputFile.setFileName(outputFilePath);
	switch (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
	{
		case true:
			qDebug() << "ERROR: Cannot open file" << outputFilePath;
			break;
		default:
			break;
	}
}

void StelScriptOutput::deinit()
{
	outputFile.close();
}

void StelScriptOutput::writeLog(QString msg)
{
	msg += "\n";
	outputFile.write(qPrintable(msg), msg.size());
	outputText += msg;
}

void StelScriptOutput::reset(void)
{
	outputFile.resize(0);
	outputFile.reset();
	outputText.clear();
}

void StelScriptOutput::saveOutputAs(const QString &name)
{
	QFile asFile;
	const QFileInfo outputInfo(outputFile);
	const QDir dir=outputInfo.dir();
	const QFileInfo newFileNameInfo(name);

	const bool okToSaveToAbsolutePath=StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	auto saveHelper = [&](const QString& fileName){
		asFile.setFileName(fileName);
		if (!asFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		{
			qDebug() << "ERROR: Cannot open file" << asFile.fileName();
			return;
		}
		qDebug() << "saving copy of output.txt to " << asFile.fileName();
		asFile.write(qPrintable(outputText), outputText.size());
		asFile.close();
	};

	if (!okToSaveToAbsolutePath && ((newFileNameInfo.isAbsolute() || (name.contains("..")))))
	{
		qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
		qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
		qWarning() << "  Storing to " << dir.absolutePath() + "/" + newFileNameInfo.fileName() << " instead";
		saveHelper(dir.absolutePath() + "/" + newFileNameInfo.fileName());
	}
	else if (okToSaveToAbsolutePath && (newFileNameInfo.isAbsolute()))
	{
		saveHelper(name);
	}
	else
	{
		saveHelper(dir.absolutePath() + "/" + name);
	}
}