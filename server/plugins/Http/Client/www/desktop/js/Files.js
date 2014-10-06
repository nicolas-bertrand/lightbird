// Manages the files.
var gl_files;

function Files()
{
    var self = this;
    gl_files = self;
    
    self.init = function ()
    {
        self.list = new Array(); // The list of the files of the user currently connected.
        self.date; // The date of the last update of the files list from the server.
        self.onAdd = new Array(); // The list of the functions to call when files are added.
        self.onUpdate = new Array(); // The list of the functions to call when files are updated.
        self.onDelete = new Array(); // The list of the functions to call when files are deleted.
        self.updateInterval; // The interval used to update the files list. from the server.
    }
    
    // Downloads the files list.
    self.onConnect = function ()
    {
        F.request("GET", "command/files/get", function (httpRequest)
        {
            if (httpRequest.status == 200)
            {
                var json = jsonParse(httpRequest.responseText);
                self.list = json.files;
                self.date = json.date;
                self.updateInterval = setInterval(self.update, C.Files.updateInterval);
                var ids = [];
                for (var i = 0; i < self.list.length; ++i)
                    ids.push(i);
                for (var i = 0; i < self.onAdd.length; ++i)
                    self.onAdd[i].handler(ids);
            }
        });
    }
    
    // Clears the files list.
    self.onDisconnect = function ()
    {
        self.list = new Array();
        if (self.updateInterval)
            clearInterval(self.updateInterval);
        delete self.updateInterval;
    }
    
    // Adds the files to the list.
    // @param files: The files to add: [{name, size, id_directory}].
    // @return The id of the files, or -1 for the ones that can't be added.
    self.add = function (files)
    {
        var id = []; // The id of the files that can be added
        var result = []; // The id of all the files (or -1)
        
        for (var i = 0; i < files.length; ++i)
        {
            var file = files[i];
            // Ensures that there is no file with the same name in this directory
            var duplicate = false;
            for (var j = 0; j < self.list.length; ++j)
                if (self.list[j].id_directory == file.id_directory && self.list[j].name == file.name)
                {
                    duplicate = true;
                    break ;
                }
        
            // Adds the file to the list
            if (!duplicate)
            {
                id.push(self.list.length);
                result.push(self.list.length);
                var date = F.ISODateString(new Date());
                self.list.push({name: file.name, size: file.size, created: date, modified: date, id_directory: file.id_directory, type: "other"});
            }
            else
                result.push(-1);
        }

        // Calls the onAdd event of the bound objects.
        for (var i = 0; i < self.onAdd.length; ++i)
            self.onAdd[i].handler(id);

        return result;
    }
    
    // Deletes the files in parameter.
    // @param files: The list of the files to delete.
    self.delete = function (files)
    {
        var filesId = [];
        var cancelUploads = [];
        for (var i = 0; i < files.length; ++i)
        {
            var id = self.list[files[i]].id;
            // If the file has no id, it is still being uploaded
            if (!id)
                cancelUploads.push(files[i]);
            else
                filesId.push(id);
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
        files.sort(function (a, b) { return (a - b); });
        for (var i = files.length - 1; i >= 0; --i)
            self.list.splice(files[i], 1);
        
        // Calls the onDelete event of the bound objects.
        for (var i = 0; i < self.onDelete.length; ++i)
            self.onDelete[i].handler(files);
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
                        // Checks if the files uploading are in the list
                        for (var u in gl_uploads.currentUploads)
                        {
                            var local = gl_files.list[gl_uploads.currentUploads[u].id];
                            if (local.id_directory == remote.id_directory && local.name == remote.name)
                            {
                                for (var f in remote)
                                    local[f] = remote[f];
                                delete local.id;
                                found = true;
                                console.log(local.name, "UPLOAD");
                                break;
                            }
                        }
                        // Searches the file in the files list
                        if (!found)
                            for (var j = 0; j < self.list.length; ++j)
                            {
                                var local = self.list[j];
                                if (local.id == remote.id)
                                {
                                    for (var f in remote)
                                        local[f] = remote[f];
                                    modified.push(local.id);
                                    found = true;
                                    console.log(local.name, "MODIFIED");
                                    break;
                                }
                            }
                        // If the file was not found, it is a new one
                        if (!found)
                        {
                            created.push(self.list.length);
                            self.list.push(remote);
                            console.log(json.files[i].name, "CREATED");
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
                        for (var j = 0; j < self.list.length; ++j)
                        {
                            if (json.deleted[i] == self.list[j].id)
                            {
                                console.log(self.list[j].name, "DELETED");
                                deleted.push(j);
                            }
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

    self.init();
    return (self);
}
