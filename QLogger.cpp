#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include "QLogger.h"

/****************************************************************************************
 ** QLogger is a library to register and print logs into a file.
 ** Copyright (C) 2016  Francesc Martinez <es.linkedin.com/in/cescmm/en>
 **
 ** This library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2.1 of the License, or (at your option) any later version.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

namespace QLogger
{
    void QLog_Trace(const QString &module, const QString &message)
    {
        QLog_(module, LogLevel::Trace, message);
    }

    void QLog_Debug(const QString &module, const QString &message)
    {
        QLog_(module, LogLevel::Debug, message);
    }

    void QLog_Info(const QString &module, const QString &message)
    {
        QLog_(module, LogLevel::Info, message);
    }

    void QLog_Warning(const QString &module, const QString &message)
    {
        QLog_(module, LogLevel::Warning, message);
    }

    void QLog_Error(const QString &module, const QString &message)
    {
        QLog_(module, LogLevel::Error, message);
    }

    void QLog_Fatal(const QString &module, const QString &message)
    {
        QLog_(module, LogLevel::Fatal, message);
    }

    void QLog_(const QString &module, LogLevel level, const QString &message)
    {
        QLoggerManager *manager = QLoggerManager::getInstance();

        QMutexLocker(&manager->mutex);

        QLoggerWriter *logWriter = manager->getLogWriter(module);

        if (logWriter and logWriter->getLevel() <= level)
                logWriter->write(module,message);
    }

    //QLoggerManager
    QLoggerManager * QLoggerManager::INSTANCE = NULL;

    QLoggerManager::QLoggerManager() : QThread(), mutex(QMutex::Recursive)
    {
        start();
    }

    QLoggerManager * QLoggerManager::getInstance()
    {
        if (!INSTANCE)
            INSTANCE = new QLoggerManager();

        return INSTANCE;
    }

    QString QLoggerManager::levelToText(const LogLevel &level)
    {
        switch (level)
        {
            case LogLevel::Trace:   return "Trace";
            case LogLevel::Debug:   return "Debug";
            case LogLevel::Info:    return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error:   return "Error";
            case LogLevel::Fatal:   return "Fatal";
            default:                return QString();
        }
    }

    bool QLoggerManager::addDestination(const QString &fileDest, const QStringList &modules, LogLevel level)
    {
        QLoggerWriter *log;
        for (const QString &module: modules)
        {
            if (!moduleDest.contains(module))
            {
                log = new QLoggerWriter(fileDest,level);
                moduleDest.insert(module, log);
                return true;
            }
        }
        return false;
    }

    void QLoggerManager::closeLogger()
    {
        exit(0);
        deleteLater();
    }

    QLoggerWriter::QLoggerWriter(const QString &fileDestination, LogLevel level)
    {
        m_fileDestination = fileDestination;
        m_level = level;
    }

    void QLoggerWriter::write(const QString &module, const QString &message)
    {
        QString _fileName = m_fileDestination;

        int MAX_SIZE = 1024 * 1024;

        QDir dir(QDir::currentPath());
        if (!dir.exists("logs"))
            dir.mkdir("logs");

        QFile file(_fileName);
        QString toRemove = _fileName.section('.', -1);
        QString fileNameAux = _fileName.left(_fileName.size() - toRemove.size()-1);
        bool renamed = false;
        QString newName = fileNameAux + "_%1__%2.log";

        //Renomenem l'arxiu si està ple
        if (file.size() >= MAX_SIZE)
        {
            //Creem un fixer nou
            QDateTime currentTime = QDateTime::currentDateTime();
            newName = newName.arg(currentTime.date().toString("dd_MM_yy")).arg(currentTime.time().toString("hh_mm_ss"));
            renamed = file.rename(_fileName, newName);

        }

        file.setFileName(_fileName);
        if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
        {
            QTextStream out(&file);
            QString dtFormat = QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss.zzz");

            if (renamed)
                out << QString("%1 - Previous log %2\n").arg(dtFormat).arg(newName);

            QString logLevel = QLoggerManager::levelToText(m_level);
            QString text = QString("[%1] [%2] {%3} %4\n").arg(dtFormat).arg(logLevel).arg(module).arg(message);
            out << text;
            file.close();
        }
    }
}
