#ifndef LIGHTBIRD_IREQUEST_H
# define LIGHTBIRD_IREQUEST_H

# include <QMap>
# include <QString>
# include <QUrl>
# include <QVariantMap>

# include "IContent.h"

namespace LightBird
{
    /// @brief Represents a request. Contains informations about it,
    /// whose depends on the protocol that transport it.
    /// Despite Request is protocol independent, its structure is inspired of HTTP.
    class IRequest
    {
    public:
        virtual ~IRequest() {}

        /// @brief The name of the protocol of the request.
        virtual const QString   &getProtocol() const = 0;
        /// @brief The method is the name of the action processed by the request.
        virtual const QString   &getMethod() const = 0;
        /// @brief The method is the name of the action processed by the request.
        virtual void            setMethod(const QString &method) = 0;
        /// @brief The URI represents the location of the resource targeted by the request.
        virtual const QUrl      &getUri() const = 0;
        /// @brief The URI represents the location of the resource targeted by the request.
        virtual void            setUri(const QUrl &uri) = 0;
        /// @brief The version of the protocol used.
        virtual const QString   &getVersion() const = 0;
        /// @brief The version of the protocol used.
        virtual void            setVersion(const QString &version) = 0;
        /// @brief The type of the content of the request.
        virtual const QString   &getType() const = 0;
        /// @brief The type of the content of the request.
        virtual void            setType(const QString &type) = 0;
        /// @brief This method is used to store additionnals informations on the response.
        virtual QVariantMap     &getInformations() = 0;
        /// @brief A header is a set of meta-information related to the request.
        /// This method allows one to access to the header and modified it.
        /// @return The header of the request.
        virtual QMap<QString, QString> &getHeader() = 0;
        /// @brief The content of the request.
        virtual LightBird::IContent &getContent() = 0;
        /// @brief If there is an error in the request (it will be not executed).
        virtual bool    isError() const = 0;
        /// @brief If there is an error in the request (it will be not executed).
        virtual void    setError(bool error = true) = 0;
    };
}

#endif // LIGHTBIRD_IREQUEST_H
