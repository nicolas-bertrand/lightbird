// Manages the uploads.
var gl_uploads;

function Uploads()
{
    var self = this;
    gl_uploads = self;
    
    self.init = function ()
    {
        self.list = {}; // The list of the uploads in progress. The key is the id of the file.
        $(document.body).bind("dragover", self.onDragover);
        $(document.body).bind("drop", self.onDrop);
    }
    
    self.add = function (file)
    {
        // Adds the file to the list
        var id = gl_files.add(file.name, file.size, "");
        if (id < 0)
        {
            console.log("The file " + file.name + " already exists in this directory.");
            return false;
        }
        var upload = self.list[id] = {size: file.size, uploaded: 0};
        var request = new XMLHttpRequest();
        
        // Called while the file is uploading
        request.upload.addEventListener('progress', function (e)
        {
            if (e.total)
            {
                upload.size = e.total;
                upload.uploaded = e.loaded;
                console.log("Progress " + file.name, Math.ceil(e.loaded / e.total * 100) + '%');
            }
        });
        
        // Called when the file has been uploaded
        request.onreadystatechange = function()
        {
            if(request.readyState == 4)
            {
                var filesUploaded = jsonParse(request.responseText || "[]");
                if (request.status == 200 && filesUploaded.length && filesUploaded[0])
                    gl_files.list[id].id = filesUploaded[0];
                else
                    console.log("An error occured during the upload of " + file.name);
                delete self.list[id];
            }
        };

        // Sends the file
        var formData = new FormData();
        formData.append("file", file);
        request.open('POST', '/c/command/uploads?disconnectOnError=true' + F.getSession(false));
        request.send(formData);
    }
    
    self.onDragover = function (e)
    {
        e.preventDefault();
    }
    
    self.onDrop = function (e)
    {
        var files = e.originalEvent.dataTransfer.files;
        for (var i = 0; i < files.length; ++i)
            gl_uploads.add(files[i]);
        e.preventDefault();
    }
    
    self.init();
    return (self);
}
