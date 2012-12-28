#ifndef LIGHTBIRD_INITIALIZE_H
# define LIGHTBIRD_INITIALIZE_H

# include <QObject>

# include "Export.h"

namespace LightBird
{
    /// @brief A helper class that allows derived classes to easily tell if they
    /// have been properly initialized during construction.
    class LIB Initialize
    {
    public:
        Initialize(bool initialized = false);
        ~Initialize();
        Initialize(const Initialize &initialize);
        Initialize &operator=(const Initialize &initialize);

        /// @brief Returns true if the class have been properly initialized.
        operator bool() const;

    protected:
        /// @brief Sets if the class have been properly initialized.
        void    isInitialized(bool initialized = true);

    private:
        bool    initialized; ///< True if the class has been initialized.
    };
}

#endif // LIGHTBIRD_INITIALIZE_H
