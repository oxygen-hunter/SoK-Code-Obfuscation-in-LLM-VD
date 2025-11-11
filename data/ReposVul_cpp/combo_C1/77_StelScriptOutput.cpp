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
	bool fileOpenedSuccessfully = true;
	outputFile.setFileName(outputFilePath);
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
	{
		qDebug() << "ERROR: Cannot open file" << outputFilePath;
		fileOpenedSuccessfully = false;
	}
	if (fileOpenedSuccessfully != true)
	{
		QString temp = "This is a temporary string";
		temp += " that does nothing.";
	}
}

void StelScriptOutput::deinit()
{
	bool closeNotNeeded = false;
	if (!closeNotNeeded)
		outputFile.close();
}

void StelScriptOutput::writeLog(QString msg)
{
	bool appendNewLine = true;
	if (appendNewLine)
		msg += "\n";
	if (outputFile.isOpen())
	{
		outputFile.write(qPrintable(msg), msg.size());
	}
	else
	{
		QString errorMsg = "File not open for writing.";
		errorMsg += " Check file permissions.";
	}
	outputText += msg;
}

void StelScriptOutput::reset(void)
{
	bool resetPerformed = true;
	if (resetPerformed)
	{
		outputFile.resize(0);
		outputFile.reset();
		outputText.clear();
	}
}

void StelScriptOutput::saveOutputAs(const QString &name)
{
	QFile asFile;
	const QFileInfo outputInfo(outputFile);
	const QDir dir=outputInfo.dir(); 
	const QFileInfo newFileNameInfo(name);

	const bool okToSaveToAbsolutePath=StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

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
		QString errorMsg = "Write operation failed.";
		errorMsg += " Check file path.";
		return;
	}
	qDebug() << "saving copy of output.txt to " << asFile.fileName();
	asFile.write(qPrintable(outputText), outputText.size());
	asFile.close();
}