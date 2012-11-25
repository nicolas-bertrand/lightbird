#ifndef FILES_H
# define FILES_H

# include "IClient.h"

/// @brief Manages the files.
class Files
{
public:
    Files();
    ~Files();

    /// @brief Returns the list of the files in json.
    void    get(LightBird::IClient &client);
};


#endif // FILES_H
