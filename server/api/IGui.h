#ifndef IGUI_H
# define IGUI_H

namespace LightBird
{
    /// @brief Allows plugins to provide a Graphical User Interface, using Qt.
    /// All the methods of this interface are guaranteed to be called in the GUI
    /// Thread, since Qt doesn't supports GUI operations in other threads than
    /// the main thread. The method gui() is called one time per plugin instance,
    /// just after IPlugin::onLoad. Notice that this interface is never called
    /// if the server has been started with the argument "noGui". To check
    /// that, use IGuis from IApi.
    class IGui
    {
    public:
        virtual ~IGui() {}

        /// @brief Allows plugins to create widgets, connect signals and slots,
        /// initialize and show theirs GUIs.
        /// This method is called just after IPlugin::onLoad, in the GUI Thread.
        virtual void    gui() = 0;
        /// @brief This method tells the plugin to show all its GUIs. It should
        /// do nothing if the windows are already displayed. show() is generally
        /// called by other plugins, and the plugin that implements it is free to
        /// do it or not.
        virtual void    show() = 0;
        /// @brief The plugin should hide all its windows. hide() is generally
        /// called by other plugins, and the plugin that implements it is free to
        /// do it or not.
        virtual void    hide() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IGui, "cc.lightbird.IGui")

#endif // IGUI_H
