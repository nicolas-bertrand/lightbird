#include <QUuid>

#include "IMime.h"
#include "ITableFiles.h"

#include "Plugin.h"
#include "Session.h"

Session::Session()
{
    this->id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    this->account = Plugin::getInstance().getApi().database().getTable(LightBird::ITable::Accounts)->toAccounts();
    this->creation = QDateTime::currentDateTime();
    this->update = QDateTime::currentDateTime();
}

Session::~Session()
{
    delete this->account;
}

Session::Session(const Session &session)
{
    *this = session;
}

Session &Session::operator=(const Session &session)
{
    if (this != &session)
    {
        this->id = session.id;
        this->account = Plugin::getInstance().getApi().database().getTable(LightBird::ITable::Accounts)->toAccounts();
        this->creation = session.creation;
        this->update = session.update;
    }
    return (*this);
}

bool    Session::operator==(const QString &id)
{
    if (id == this->id)
        return (true);
    return (false);
}

const QString   &Session::getId()
{
    return (this->id);
}

LightBird::ITableAccounts   &Session::getAccount()
{
    return (*this->account);
}

bool    Session::identify(const QString &identifiant)
{
    return (this->account->setIdFromIdentifiantAndSalt(identifiant, this->id));
}

QDateTime   &Session::getCreation()
{
    return (this->creation);
}

QDateTime   &Session::getUpdate()
{
    return (this->update);
}

void        Session::setUpdate()
{
    this->update = QDateTime::currentDateTime();
}
