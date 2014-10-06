#ifndef FILES_H
# define FILES_H

# include "IClient.h"

/// @brief Manages the files.
class Files
{
public:
    static const uint deleteMaxContentLength = 10000000; ///< The maximum size of the content-length of a delete request.
    static const uint deleteFileIdMaxSize = 1024; ///< The maximum size of a file id.

    Files();
    ~Files();

    /// @brief Returns the list of the files in json {date: "yyyy-MM-dd hh:mm:ss", files: [{files_informations}, {files_informations}]}.
    void    get(LightBird::IClient &client);
    /// @brief Returns the list of the files created/modified/deleted since the date in parameter:
    /// {date: "yyyy-MM-dd hh:mm:ss", files: [{files_informations}, {files_informations}], deleted: [id1, id2]}
    void    update(LightBird::IClient &client);
    /// @brief Deletes the files in the content of the request.
    /// The content must have the form of a JSON array:
    /// ["id_file_1","id_file_2","id_file_3"]
    /// The files are deleted while the JSON is received.
    /// The response contains a JSON array of the files that could not be deleted.
    void    deleteFiles(LightBird::IClient &client);

private:
    /// @brief Deletes a file.
    void    _deleteFile(LightBird::IClient &client, const QString &id);
    /// @brief A fatal error occurred. The client is disconnected.
    void    _deleteError(LightBird::IClient &client, const QString &message);
};


#endif // FILES_H
