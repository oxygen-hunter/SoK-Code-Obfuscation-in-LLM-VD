#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

static QString getOutputFilePath(const QString& path) {
	return path;
}

static QString getMessageWithNewline(QString msg) {
	return msg + "\n";
}

static bool isAbsolutePathAllowed() {
	return StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();
}

static QString getCombinedPath(const QDir& dir, const QString& name) {
	return dir.absolutePath() + "/" + name;
}

void StelScriptOutput::init(const QString& outputFilePath)
{
	QString filePath = getOutputFilePath(outputFilePath);
	outputFile.setFileName(filePath);
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		qDebug() << "ERROR: Cannot open file" << filePath;
}

void StelScriptOutput::deinit()
{
	outputFile.close();
}

void StelScriptOutput::writeLog(QString msg)
{
	QString completeMsg = getMessageWithNewline(msg);
	outputFile.write(qPrintable(completeMsg), completeMsg.size());
	outputText += completeMsg;
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

	bool okToSaveToAbsolutePath = isAbsolutePathAllowed();

	if (!okToSaveToAbsolutePath && ((newFileNameInfo.isAbsolute() || (name.contains("..")))))
	{
		qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
		qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
		asFile.setFileName(getCombinedPath(dir, newFileNameInfo.fileName()));
		qWarning() << "  Storing to " << asFile.fileName() << " instead";
	}
	else if (okToSaveToAbsolutePath && (newFileNameInfo.isAbsolute()))
	{
		asFile.setFileName(name);
	}
	else
	{
		asFile.setFileName(getCombinedPath(dir, name));
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