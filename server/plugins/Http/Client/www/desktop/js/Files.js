// Manages the files.
var gl_files;

function Files()
{
    var self = this;
    gl_files = self;
    
    self.init = function ()
    {
        self.list = new Array(); // The list of the files of the user currently connected.
        self.onAdd = new Array(); // The list of the functions to call when a file is added.
        self.onDelete = new Array(); // The list of the functions to call when a file is deleted.
    }
    
    // Downloads the files list.
    self.onConnect = function ()
    {
        F.request("GET", "command/files/get", function (httpRequest)
        {
            if (httpRequest.status == 200)
                self.list = jsonParse(httpRequest.responseText);
        });
    }
    
    // Clears the files list.
    self.onDisconnect = function ()
    {
        self.list = new Array();
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
    
    // Events
    {
        // Allows an object to be notified when a file is added.
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
        
        // Allows an object to be notified when a file is deleted.
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
