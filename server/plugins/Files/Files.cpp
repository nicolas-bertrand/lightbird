#include <QDir>

#include "IIdentifier.h"

#include "LightBird.h"
#include "Plugin.h"

Files::Files(LightBird::IApi *a, const QString &t) : api(a), timerName(t)
{
    // Reads the configuration
    if (!(this->removeFileTimer = this->api->configuration(true).get("removeFileTimer").toUInt() * 60000))
        this->removeFileTimer = 3600000;
    // Starts to watch the filesPath
    if (this->api->configuration(true).get("filesPathWatcher") == "true")
    {
        this->fileSystemWatcher.addPath(LightBird::getFilesPath());
        this->connect(&this->fileSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(_directoryChanged(QString)), Qt::QueuedConnection);
    }
    // Initializes the addFiles timer
    this->connect(&this->addFiles, SIGNAL(timeout()), this, SLOT(_addFiles()), Qt::QueuedConnection);
    this->api->timers().setTimer(this->timerName, this->removeFileTimer);
}

Files::~Files()
{
}

void    Files::timer()
{
    this->_removeFiles();
}

void    Files::_directoryChanged(const QString &)
{
    // We wait 2 seconds before adding the new files
    this->addFiles.start(2000);
}

void    Files::_addFiles()
{
    LightBird::TableFiles   file;
    QDir                    directory(LightBird::getFilesPath());
    QStringList             removeFilesList;
    QString                 fileName;
    QList<void *>           extensions;
    LightBird::IIdentify::Information information;

    removeFilesList = this->_removeFiles();
    // Adds the new files to the database
    QListIterator<QFileInfo> it(directory.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable | QDir::Writable, QDir::Time));
    while (it.hasNext())
    {
        fileName = it.peekNext().fileName();
        // A file not in the database has been added to the filesPath
        if (!removeFilesList.contains(fileName) && !file.setIdFromPath(fileName) && !file.setIdFromVirtualPath(fileName))
        {
            file.add(fileName, fileName);
            LOG_INFO("File added", Properties("name", fileName).add("idFile", file.getId()).toMap(), "Files", "_addFiles");
            // Identify the file
            if (!(extensions = this->api->extensions().get("IIdentifier")).isEmpty())
                information = static_cast<LightBird::IIdentifier *>(extensions.first())->identify(file.getFullPath());
            this->api->extensions().release(extensions);
            file.setType(information.type_string);
            file.setInformations(information.data);
            information.data.clear();
        }
        it.next();
    }
    this->addFiles.stop();
}

QStringList Files::_removeFiles()
{
    LightBird::TableEvents  event;
    QStringList             remainingFiles;
    QString                 fileName;

    QStringListIterator ev(event.getEvents("remove_file_later"));
    while (ev.hasNext())
    {
        event.setId(ev.next());
        if (QFile::remove((fileName = event.getInformation("path").toString())))
            LOG_DEBUG("File removed", Properties("path", fileName).toMap(), "Files", "_removeFiles");
        QFileInfo file(fileName);
        if (!file.isFile())
            event.remove();
        else
            remainingFiles << file.fileName();
    }
    return (remainingFiles);
}
