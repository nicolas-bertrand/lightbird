#include <QDir>

#include "LightBird.h"
#include "Plugin.h"

Files::Files(LightBird::IApi &a, const QString &t) : api(a), timerName(t)
{
    // Reads the configuration
    if (!(this->removeFileTimer = this->api.configuration(true).get("removeFileTimer").toUInt() * 60000))
        this->removeFileTimer = 3600000;
    if (!(this->directoryChangedTimer = this->api.configuration(true).get("directoryChangedTimer").toUInt() * 1000))
        this->directoryChangedTimer = 15000;
    // Starts to watch the filesPath
    if (this->api.configuration(true).get("filesPathWatcher") == "true")
    {
        this->fileSystemWatcher.addPath(LightBird::TableFiles::getFilesPath());
        this->connect(&this->fileSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(_directoryChanged(QString)), Qt::QueuedConnection);
    }
    this->api.timers().setTimer(this->timerName, this->removeFileTimer);
}

Files::~Files()
{
}

void    Files::timer()
{
    LightBird::TableEvents  event;
    LightBird::TableFiles   file;
    QString                 path;
    QString                 filesPath = LightBird::TableFiles::getFilesPath();

    // Checks if some files need to be removed
    if (this->removeFileDate < QDateTime::currentDateTime())
    {
        QStringListIterator it(event.getEvents("remove_file_later"));
        while (it.hasNext())
        {
            event.setId(it.next());
            if (QFile::remove((path = event.getInformation("path").toString())))
                this->api.log().trace("File removed", Properties("path", path).toMap(), "Files", "timer");
            if (!QFileInfo(path).isFile())
                event.remove();
        }
        this->removeFileDate = QDateTime::currentDateTime().addMSecs(this->removeFileTimer);
    }
    // Checks if some files can be added to the database
    if (!this->newFiles.isEmpty())
    {
        QMutableMapIterator<QString, File> it(this->newFiles);
        while (it.hasNext())
        {
            it.next();
            // The file is already in the database or doesn't exists
            if (!file.getIdFromPath(it.key()).isEmpty() || !QFileInfo(filesPath + it.key()).isFile())
                it.remove();
            // If the file has not been modified since directoryChangedTimer we can add it
            else if (it.value().date < QDateTime::currentDateTime() && it.value().size == QFileInfo(filesPath + it.key()).size())
            {
                file.add(it.key(), it.key());
                this->api.log().debug("File added", Properties("name", it.key()).add("idFile", file.getId()).toMap(), "Files", "timer");
                it.remove();
            }
            // The file has been modified
            else
            {
                it.value().size = QFileInfo(filesPath + it.key()).size();
                it.value().date = QDateTime::currentDateTime().addMSecs(this->directoryChangedTimer);
            }
        }
        if (this->newFiles.isEmpty())
            this->api.timers().setTimer(this->timerName, this->removeFileTimer);
    }
}

void    Files::_directoryChanged(const QString &path)
{
    LightBird::TableFiles   file;
    QDir                    directory(path);

    QListIterator<QFileInfo> it(directory.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable | QDir::Writable, QDir::Time));
    while (it.hasNext())
    {
        // A file not in the database has been added to the directory
        if (!file.setIdFromPath(it.peekNext().fileName()) && !file.setIdFromVirtualPath(it.peekNext().fileName()))
        {
            if (!this->newFiles.contains(it.peekNext().fileName()))
            {
                if (this->newFiles.isEmpty())
                    this->api.timers().setTimer(this->timerName, this->directoryChangedTimer);
                this->newFiles[it.peekNext().fileName()].size = it.peekNext().size();
            }
            this->newFiles[it.peekNext().fileName()].date = QDateTime::currentDateTime().addMSecs(this->directoryChangedTimer);
        }
        it.next();
    }
}
