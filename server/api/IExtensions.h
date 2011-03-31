#ifndef IEXTENSIONS_H
# define IEXTENSIONS_H

# include <QString>

namespace Streamit
{
    /// @brief Via this interface, the plugins can get pointers on extensions, and release them.
    class IExtensions
    {
    public:
        virtual ~IExtensions() {}

        /// @brief Allows to get pointers on the plugins that implements the extension in parameter.
        /// These extensions has to be released as soon as they are unused (like a mutex).
        virtual QList<void *> get(const QString &name) = 0;
        /// @brief Release the extensions in the list. Must be called as soon as possible after a get().
        virtual void          release(QList<void *> extensions) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::IExtensions, "cc.lightbird.IExtensions");

#endif // IEXTENSIONS_H
