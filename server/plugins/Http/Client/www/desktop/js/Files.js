// Manages the files.
var gl_files;

// This object inherit Array and stores and manage the files of the user.
function Files()
{
    var self = [];
    gl_files = self;
    
    self.init = function ()
    {
        // Represents a file in the array.
        var File =
        {
            info: // Stores the informations of the file
            {
                id: "", // The id of the file (empty while it is uploading)
                name: "", // The name of the file
                id_directory: "", // The id of the file directory
                modified: "", // The date of the last modification of the file
                created: "", // The date at which the file was added to the database
            },
            upload: {} // This object is defined while the file is being uploaded (see gl_uploads.Upload)
        };
        self.date; // The date of the last update of the files list from the server.
        self.onAdd = new Array(); // The list of the functions to call when files are added.
        self.onUpdate = new Array(); // The list of the functions to call when files are updated.
        self.onDelete = new Array(); // The list of the functions to call when files are deleted.
        self.updateInterval; // The interval used to update the files list. from the server.
        self.filesDeleted = {}; // The list of the files deleted. This ensures that they are not added later by the files update.
        self.filesDeletedModified = (new Date()).getTime(); // The last time self.filesDeleted was modified, which allows to clean it after C.Files.cleanFilesDeleted milliseconds.
        self.filesExtensionsTypes = {}; // Associates the files extensions with their types.
        
        // Gets the json of self.filesExtensionsTypes
        F.request("GET", "filesExtensionsTypes.json", function (httpRequest)
        {
            if (httpRequest.status == 200)
                self.filesExtensionsTypes = jsonParse(httpRequest.responseText);
        });
    }
    
    // Downloads the files list.
    self.onConnect = function ()
    {
        F.request("GET", "command/files/get", function (httpRequest)
        {
            if (httpRequest.status == 200)
            {
                var json = jsonParse(httpRequest.responseText);
                self.date = json.date;
                self._clear();
                for (var i = 0; i < json.files.length; ++i)
                    self.push({info: json.files[i]});
                for (var i = 0; i < self.onAdd.length; ++i)
                    self.onAdd[i].handler(self);
                self.updateInterval = setInterval(self.update, C.Files.updateInterval);
            }
        });
    }
    
    // Clears the files list.
    self.onDisconnect = function ()
    {
        self._clear();
        if (self.updateInterval)
            clearInterval(self.updateInterval);
        delete self.updateInterval;
    }
    
    // Adds the files to the list.
    // @param files: The files to add: [{name, size, id_directory}].
    // @return: The Files or null for the ones that can't be added.
    self.add = function (files)
    {
        var onAddFiles = []; // The id of the files that can be added
        var result = []; // The id of all the files (or -1)
        
        for (var i = 0; i < files.length; ++i)
        {
            var file = files[i];
            // Ensures that there is no file with the same name in this directory
            var duplicate = false;
            for (var j = 0; j < self.length; ++j)
                if (self[j].info.id_directory == file.id_directory && self[j].info.name == file.name)
                {
                    duplicate = true;
                    break ;
                }
        
            // Adds the file to the list
            if (!duplicate)
            {
                var date = F.ISODateString(new Date());
                var dotIndex = file.name.lastIndexOf('.');
                var extension = "";
                var type = "other";
                if (dotIndex >= 0)
                {
                    extension = file.name.substring(dotIndex + 1);
                    type = self.filesExtensionsTypes[extension.toLowerCase()] || "other";
                }
                var file = {info: {name: file.name, size: file.size, created: date, modified: date, id_directory: file.id_directory, type: type, extension: extension}};
                self.push(file);
                onAddFiles.push(file);
                result.push(file);
            }
            else
                result.push(null);
        }

        // Calls the onAdd event of the bound objects.
        if (onAddFiles.length)
            for (var i = 0; i < self.onAdd.length; ++i)
                self.onAdd[i].handler(onAddFiles);

        return result;
    }
    
    // Deletes the files in parameter.
    // @param files: The list of the files to delete.
    self.delete = function (deleted)
    {
        // Cleans self.filesDeleted
        if ((new Date()).getTime() - self.filesDeletedModified > C.Files.cleanFilesDeleted)
            self.filesDeleted = {};
        self.filesDeletedModified = (new Date()).getTime();

        // Gets the files that are on the server (and not uploading)
        var filesId = [];
        var cancelUploads = [];
        for (var i = 0; i < deleted.length; ++i)
        {
            if (deleted[i].upload)
                cancelUploads.push(deleted[i]);
            else
            {
                self.filesDeleted[deleted[i].info.id] = true;
                filesId.push(deleted[i].info.id);
            }
        }
        
        // Cancels the upload of the files to delete
        if (cancelUploads.length)
            gl_uploads.cancel(cancelUploads);
        
        // Sends the delete request to the server
        if (filesId.length)
            F.request("POST", "command/files/delete", function (httpRequest)
            {
                if (httpRequest.status == 200)
                {
                    var filesNotDeleted = jsonParse(httpRequest.responseText);
                    if (filesNotDeleted.length)
                        console.log("Files not deleted: ", filesNotDeleted);
                }
            }, JSON.stringify(filesId), "application/json");
        
        // Removes the files from the list
        for (var i = 0; i < deleted.length; ++i)
            for (j = 0; self.length; ++j)
                if (deleted[i].info == self[j].info)
                {
                    self.splice(j, 1);
                    break;
                }
        
        // Calls the onDelete event of the bound objects.
        for (var i = 0; i < self.onDelete.length; ++i)
            self.onDelete[i].handler(deleted);
    }
    
    // Updates the files list by sending an update request to the server.
    self.update = function ()
    {
        F.request("GET", "command/files/update?date=" + self.date, function (httpRequest)
        {
            if (httpRequest.status == 200)
            {
                var json = jsonParse(httpRequest.responseText);
                var modified = []; // The list of the files modified
                var created = []; // The list of the files created
                if (json.files)
                {
                    for (var i = 0; i < json.files.length; ++i)
                    {
                        var remote = json.files[i];
                        var found = false;
                        // Ensures that the file was not already deleted
                        if (self.filesDeleted[remote.id])
                            continue;
                        // Checks if the files uploading are in the list
                        for (var u in gl_uploads.currentUploads)
                        {
                            var local = gl_uploads.currentUploads[u].info;
                            if (local.id_directory == remote.id_directory && local.name == remote.name)
                            {
                                for (var f in remote)
                                    local[f] = remote[f];
                                delete local.id;
                                found = true;
                                break;
                            }
                        }
                        // Searches the file in the files list
                        if (!found)
                            for (var j = 0; j < self.length; ++j)
                            {
                                var local = self[j].info;
                                if (local.id == remote.id)
                                {
                                    for (var f in remote)
                                        local[f] = remote[f];
                                    modified.push(self[j]);
                                    found = true;
                                    break;
                                }
                            }
                        // If the file was not found, it is a new one
                        if (!found)
                        {
                            var file = {info: remote};
                            created.push(file);
                            self.push(file);
                        }
                    }
                    // Updates the modified files
                    if (modified.length)
                        for (var i = 0; i < self.onUpdate.length; ++i)
                            self.onUpdate[i].handler(modified);
                    // Adds the created files
                    if (created.length)
                        for (var i = 0; i < self.onAdd.length; ++i)
                            self.onAdd[i].handler(created);
                }
                if (json.deleted)
                {
                    var deleted = [];
                    for (var i = 0; i < json.deleted.length; ++i)
                        for (var j = 0; j < self.length; ++j)
                        {
                            if (json.deleted[i] == self[j].info.id)
                                deleted.push(self[j]);
                        }
                    if (deleted.length)
                        self.delete(deleted);
                }
                self.date = json.date;
            }
        });
    }
    
    // Events
    {
        // Allows an object to be notified when files are added.
        self.bindOnAdd = function (object, handler)
        {
            for (var i = 0; i < self.onAdd.length; ++i)
                if (self.onAdd[i].object == object)
                    self.onAdd.splice(i--, 1);
            self.onAdd.push({object: object, handler: handler});
        }
        
        self.unbindOnAdd = function (object)
        {
            for (var i = 0; i < self.onAdd.length; ++i)
                if (self.onAdd[i].object == object)
                    self.onAdd.splice(i--, 1);
        }
        
        // Allows an object to be notified when files are updated.
        self.bindOnUpdate = function (object, handler)
        {
            for (var i = 0; i < self.onUpdate.length; ++i)
                if (self.onUpdate[i].object == object)
                    self.onUpdate.splice(i--, 1);
            self.onUpdate.push({object: object, handler: handler});
        }
        
        self.unbindOnUpdate = function (object)
        {
            for (var i = 0; i < self.onUpdate.length; ++i)
                if (self.onUpdate[i].object == object)
                    self.onUpdate.splice(i--, 1);
        }
        
        // Allows an object to be notified when files are deleted.
        self.bindOnDelete = function (object, handler)
        {
            for (var i = 0; i < self.onDelete.length; ++i)
                if (self.onDelete[i].object == object)
                    self.onDelete.splice(i--, 1);
            self.onDelete.push({object: object, handler: handler});
        }
        
        self.unbindOnDelete = function (object)
        {
            for (var i = 0; i < self.onDelete.length; ++i)
                if (self.onDelete[i].object == object)
                    self.onDelete.splice(i--, 1);
        }
    }
    
    // Private
    {
        // Clears the files array.
        self._clear = function ()
        {
            while (self.length > 0)
            self.shift();
        }
    }

    self.init();
    return (self);
}
