#ifndef INITIALIZE_H
# define INITIALIZE_H

/// @brief A helper class that allows derived classes to easily tell if they
/// have been properly initialized during construction.
class Initialize
{
public:
    Initialize(bool initialized = false);
    ~Initialize();
    Initialize(const Initialize &);
    Initialize &operator=(const Initialize &);

    /// @brief Returns true if the class have been properly initialized.
    operator bool();

protected:
    /// @brief Sets if the class have been properly initialized.
    void    isInitialized(bool initialized = true);

private:
    bool    initialized; ///< True if the class has been initialized.
};

#endif // INITIALIZE_H
