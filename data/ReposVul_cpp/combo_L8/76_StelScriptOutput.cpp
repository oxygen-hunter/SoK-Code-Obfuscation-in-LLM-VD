#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"
#include <Python.h>

QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

extern "C" {
    void initOutputFile(const char* outputFilePath) {
        QFile file;
        file.setFileName(QString::fromUtf8(outputFilePath));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
            qDebug() << "ERROR: Cannot open file" << outputFilePath;
    }

    void closeOutputFile() {
        QFile file;
        file.close();
    }

    void writeLogToFile(const char* msg) {
        QString message = QString::fromUtf8(msg) + "\n";
        QFile file;
        file.write(qPrintable(message), message.size());
    }

    void saveOutputAsFile(const char* name) {
        QFile asFile;
        const QFileInfo outputInfo(outputFile);
        const QDir dir=outputInfo.dir();
        const QFileInfo newFileNameInfo(QString::fromUtf8(name));
        
        const bool okToSaveToAbsolutePath=StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

        if (QString::fromUtf8(name).contains("config.ini"))
        {
            qWarning() << "SCRIPTING ERROR: You are trying to overwrite config.ini. Ignoring.";
            return;
        }

        if (!okToSaveToAbsolutePath && ((newFileNameInfo.isAbsolute() || (QString::fromUtf8(name).contains("..")))))
        {
            qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
            qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
            asFile.setFileName(dir.absolutePath() + "/" + newFileNameInfo.fileName());
            qWarning() << "  Storing to " << asFile.fileName() << " instead";
        }
        else if (okToSaveToAbsolutePath && (newFileNameInfo.isAbsolute()))
        {
            asFile.setFileName(QString::fromUtf8(name));
        }
        else
        {
            asFile.setFileName(dir.absolutePath() + "/" + QString::fromUtf8(name));
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
}

void StelScriptOutput::init(const QString& outputFilePath)
{
    initOutputFile(outputFilePath.toUtf8().data());
}

void StelScriptOutput::deinit()
{
    closeOutputFile();
}

void StelScriptOutput::writeLog(QString msg)
{
    writeLogToFile(msg.toUtf8().data());
    outputText += msg + "\n";
}

void StelScriptOutput::saveOutputAs(const QString &name)
{
    saveOutputAsFile(name.toUtf8().data());
}

void StelScriptOutput::reset(void)
{
    outputFile.resize(0);
    outputFile.reset();
    outputText.clear();
}