#ifndef IRESPONSE_H
# define IRESPONSE_H

# include <QString>
# include <QByteArray>
# include <QMap>
# include <QUrl>

# include "IContent.h"

namespace Streamit
{
    /// @brief Represents a response. Contains informations about it,
    /// whose depends on the protocol that transport it.
    /// Despite Request is protocol independant, its structure is inspired of HTTP.
    class IResponse
    {
    public:
        virtual ~IResponse() {}

        /// @brief The version of the protocol used.
        virtual const QString   &getVersion() const = 0;
        /// @brief The version of the protocol used.
        virtual void            setVersion(const QString &version) = 0;
        /// @brief The code of the response.
        virtual int             getCode() const = 0;
        /// @brief The code of the response.
        virtual void            setCode(int code) = 0;
        /// @brief The message that corresponds to the code of the response.
        virtual const QString   &getMessage() const = 0;
        /// @brief The message that corresponds to the code of the response.
        virtual void            setMessage(const QString &message) = 0;
        /// @brief The type of the content of the response.
        virtual const QString   &getType() const = 0;
        /// @brief The type of the content of the response.
        virtual void            setType(const QString &type) = 0;
        /// @brief A header is a set of meta-information related to the response.
        /// This method allows one to access to the header and modified it.
        /// @return The header of the response.
        virtual QMap<QString, QString>  &getHeader() = 0;
        /// @brief The content of the response.
        virtual Streamit::IContent  &getContent() = 0;
        /// @brief This method is used to store additionnals informations on the response
        virtual QVariantList    &getInformations() = 0;
    };
}

#endif // IRESPONSE_H
