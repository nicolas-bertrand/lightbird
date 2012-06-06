#ifndef FILES_H
# define FILES_H

# include <QVariantList>

# include "IClient.h"

/// @brief Manages the files.
class Files : public QObject
{
    Q_OBJECT

public:
    Files(QObject *parent = NULL);
    ~Files();

    /// @brief Returns the list of the files in json.
    void    get(LightBird::IClient &client);
};


#endif // FILES_H
