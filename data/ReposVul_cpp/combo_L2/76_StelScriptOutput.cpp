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
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		qDebug() << "ERROR: Cannot open file" << outputFilePath;
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

	if (name.contains("config.ini"))
	{
		qWarning() << "SCRIPTING ERROR: You are trying to overwrite config.ini. Ignoring.";
		return;
	}

	if (!okToSaveToAbsolutePath && ((newFileNameInfo.isAbsolute() || (name.contains("..")))))
	{
		qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
		qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
		asFile.setFileName(dir.absolutePath() + "/" + newFileNameInfo.fileName());
		qWarning() << "  Storing to " << asFile.fileName() << " instead";
	}
	else if (okToSaveToAbsolutePath && (newFileNameInfo.isAbsolute()))
	{
		asFile.setFileName(name);
	}
	else
	{
		asFile.setFileName(dir.absolutePath() + "/" + name);
	}

	if (!asFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
	{
		qDebug() << "ERROR: Cannot open file" << asFile.fileName();
		return;
	}
	qDebug() << "saving copy of output.txt to " << asFile.fileName();
	asFile.write(qPrintable(outputText), outputText.size());
	asFile.close();
}