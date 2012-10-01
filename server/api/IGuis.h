#ifndef IGUIS_H
# define IGUIS_H

# include <QString>

namespace LightBird
{
    /// @brief Allows plugins to manage the Gui of other plugins.
    /// This works only if the server had not been started with the "noGui" argument.
    /// You can check that with the method noGui.
    class IGuis
    {
    public:
        virtual ~IGuis() {}

        /// @brief Tell the plugins to show their GUIs.
        /// @param id : The id of the targeted plugin. If empty, all the plugins
        /// will be requested to show their GUIs.
        virtual void    show(const QString &id = "") = 0;
        /// @brief Tell the plugins to hide their GUIs.
        /// @param id : The id of the targeted plugin. If empty, all the plugins
        /// will be requested to hide their GUIs.
        virtual void    hide(const QString &id = "") = 0;
        /// @return True if the GUIs of the server are disabled using the "noGui"
        /// argument of the server.
        virtual bool    noGui() const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IGuis, "cc.lightbird.IGuis")

#endif // IGUIS_H
