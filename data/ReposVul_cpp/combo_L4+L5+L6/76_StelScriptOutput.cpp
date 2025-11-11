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
	switch (outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
	{
	case false:
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

void recursiveSaveOutputAs(const QString &name, QFile &asFile, const QFileInfo &outputInfo, const QDir &dir, const QFileInfo &newFileNameInfo, bool okToSaveToAbsolutePath, QString &outputText, int step = 0)
{
	switch (step)
	{
	case 0:
		if (name.contains("config.ini"))
		{
			qWarning() << "SCRIPTING ERROR: You are trying to overwrite config.ini. Ignoring.";
			return;
		}
		recursiveSaveOutputAs(name, asFile, outputInfo, dir, newFileNameInfo, okToSaveToAbsolutePath, outputText, 1);
		break;
	case 1:
		if (!okToSaveToAbsolutePath && ((newFileNameInfo.isAbsolute() || (name.contains("..")))))
		{
			qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
			qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
			asFile.setFileName(dir.absolutePath() + "/" + newFileNameInfo.fileName());
			qWarning() << "  Storing to " << asFile.fileName() << " instead";
			recursiveSaveOutputAs(name, asFile, outputInfo, dir, newFileNameInfo, okToSaveToAbsolutePath, outputText, 3);
		}
		else if (okToSaveToAbsolutePath && (newFileNameInfo.isAbsolute()))
		{
			asFile.setFileName(name);
			recursiveSaveOutputAs(name, asFile, outputInfo, dir, newFileNameInfo, okToSaveToAbsolutePath, outputText, 3);
		}
		else
		{
			asFile.setFileName(dir.absolutePath() + "/" + name);
			recursiveSaveOutputAs(name, asFile, outputInfo, dir, newFileNameInfo, okToSaveToAbsolutePath, outputText, 3);
		}
		break;
	case 3:
		if (!asFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		{
			qDebug() << "ERROR: Cannot open file" << asFile.fileName();
			return;
		}
		qDebug() << "saving copy of output.txt to " << asFile.fileName();
		asFile.write(qPrintable(outputText), outputText.size());
		asFile.close();
		break;
	default:
		break;
	}
}

void StelScriptOutput::saveOutputAs(const QString &name)
{
	QFile asFile;
	const QFileInfo outputInfo(outputFile);
	const QDir dir=outputInfo.dir();
	const QFileInfo newFileNameInfo(name);

	bool okToSaveToAbsolutePath=StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	recursiveSaveOutputAs(name, asFile, outputInfo, dir, newFileNameInfo, okToSaveToAbsolutePath, outputText);
}