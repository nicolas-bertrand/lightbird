#ifndef ICONFIGURATION_H
# define ICONFIGURATION_H

# include <QDomElement>
# include <QString>

namespace LightBird
{
    /// @brief Interface with tools to access and edit the configuration of the
    /// server or the plugins.
    /// This interface is threaeSafe.
    class IConfiguration
    {
    public:
        virtual ~IConfiguration() {}

        /// @brief Returns the path to the configuration file, including its name.
        virtual QString     getPath() const = 0;
        /// @brief Returns the value of the node identified by nodeName.
        /// @param nodeName : The node name. NodeName can be in the form
        /// "parentNode/[..]/parentName[n]/nodeName[.attribut]", or simply "nodeName".
        /// @return The node value.
        virtual QString     get(const QString &nodeName) const = 0;
        /// @brief Count the number of occurence of nodeName in his parent node.
        /// @param nodeName : The node name. NodeName can be in the form
        /// "parentNode/[..]/parentName[n]/nodeName", or simply "nodeName".
        /// @return The number of nodeName in his parent node.
        virtual unsigned    count(const QString &nodeName) const = 0;
        /// @brief Sets value of the node identified by nodeName.
        /// If nodeName does not exist, it will be created.
        /// If nodeName occured multiple times in the tree,
        /// only the first one will be edited.
        /// @param nodeName : The node name. NodeName can be in the form
        /// "parentNode/[..]/parentName[n]/nodeName[.attribut]", or simply "nodeName".
        /// @param nodeValue : The value to set or change.
        virtual void        set(const QString &nodeName, const QString &nodeValue) = 0;
        /// @brief Remove a node and all its childs. Only the first node that match is removed.
        /// @param nodeName : The node name. NodeName can be in the form
        /// "parentNode/[..]/parentName[n]/nodeName[n]", or simply "nodeName".
        /// @return If a node had been removed.
        virtual bool        remove(const QString &nodeName) = 0;
        /// @brief Gets the configuration tree as a Qt Object for read access.
        /// As soon as you have finished working with the tree, it MUST be released
        /// via release(). Otherwise, a deadlock will occure.
        /// @see release
        /// @return the configuration tree
        virtual QDomElement readDom() const = 0;
        /// @brief Gets the configuration tree as a Qt Object for write access.
        /// As soon as you have finished working with the tree, it MUST be released
        /// via release(). Otherwise, a deadlock will occure.
        /// @see release
        /// @return the configuration tree.
        virtual QDomElement writeDom() = 0;
        /// @brief Release the tree locked by readDom() or writeDom().
        /// @see readDom
        /// @see writeDom
        virtual void        release() const = 0;
        /// @brief Saves the modifications made to the configuration into the file.
        /// If the instance represents the configuration of the server, the event
        /// configuration_saved is sent.
        /// @return If the configuration has been saved.
        virtual bool        save() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IConfiguration, "cc.lightbird.IConfiguration")

#endif // ICONFIGURATION_H
