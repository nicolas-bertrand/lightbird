// Manages the uploads.
var gl_uploads;

function Uploads()
{
    var self = this;
    gl_uploads = self;
    
    self.init = function ()
    {
        self.pendingUploads = {}; // The files waiting to be uploaded.
        self.currentUploads = {}; // The files currently uploading.
        // This object represents an upload in the above arrays
        var upload =
        {
            id: 0, // The id of the file in gl_files
            cancelId: "", // A uuid used to cancel the upload
            size: 0, // The size of the file
            uploaded: 0, // The amount of data uploaded so far
            percent: 0, // The percentage of the file uploaded
            previousPercent: undefined, // The percentage the last time onUpload was called
            request: {}, // The XMLHttpRequest
            file: {}, // The File to upload
            cancelled: false, // True if the upload has been cancelled
            finished: false // True if the upload is finished
        };
        self.onUpload = new Array(); // The list of the functions to call while a file is uploading.
        self.onUploadInterval; // The interval that calls onUpload
        
        // These events allows to drop a file in the browser
        $(document.body).bind("dragover", self._onDragover);
        $(document.body).bind("drop", self._onDrop);
    }
    
    // Uploads the files in parameter.
    // @param files: The FileList to upload.
    self.add = function (files)
    {
        // Adds the file to the list
        var f = [];
        for (var i = 0; i < files.length; ++i)
            f.push({name: files[i].name, size: files[i].size, id_directory: ""});
        var ids = gl_files.add(f);
        // setTimeout is used to let the browser display the files
        setTimeout(function ()
        {
            // Creates the uploads
            for (var i = 0; i < ids.length; ++i)
            {
                if (ids[i] >= 0)
                    self._create(ids[i], files[i]);
                else
                    console.log("The file " + files[i].name + " already exists in this directory.");
            }
        }, 0);
    }

    // Cancels the uploads.
    // @param files: The array of the id of the files to cancel.
    self.cancel = function (files)
    {
        var upload;
        var cancelIds = [];
        for (var i = 0; i < files.length; ++i)
            if (self.pendingUploads[files[i]])
                delete self.pendingUploads[files[i]];
        for (var i = 0; i < files.length; ++i)
            if ((upload = self.currentUploads[files[i]]))
            {
                delete self.currentUploads[files[i]];
                upload.cancelled = true;
                upload.request.abort();
                cancelIds.push(upload.cancelId);
            }
        // Cancels the current uploads
        if (cancelIds.length)
            F.request("POST", "command/uploads/cancel", null, JSON.stringify(cancelIds), "application/json");
    }
    
    // Allows an object to be notified of the progression of the uploads.
    self.bindOnUpload = function (object, handler)
    {
        for (var i = 0; i < self.onUpload.length; ++i)
            if (self.onUpload[i].object == object)
                self.onUpload.splice(i--, 1);
        self.onUpload.push({object: object, handler: handler});
    }
    
    // Unbinds the onUpload event.
    self.unbindOnUpload = function (object)
    {
        for (var i = 0; i < self.onUpload.length; ++i)
            if (self.onUpload[i].object == object)
                self.onUpload.splice(i--, 1);
    }
    
    // Private
    {
        // Creates an upload.
        self._create = function (id, file)
        {
            var request = new XMLHttpRequest();
            var upload = self.pendingUploads[id] =
            {
                id: id,
                cancelId: F.getUuid(),
                size: file.size,
                uploaded: 0,
                percent: 0,
                previousPercent: undefined,
                request: request,
                file: file,
                cancelled: false,
                finished: false
            };
            
            // Creates the onUpload event
            if (!self.onUploadInterval)
                self.onUploadInterval = setInterval(self._callOnUpload, C.Uploads.onUploadInterval);
            
            // Starts the next upload
            var n = 0;
            for (var u in self.currentUploads)
                if (++n >= C.Uploads.concurrent)
                    return ;
            self._start();
        }
        
        // Starts the next upload.
        self._start = function ()
        {
            for (var u in self.pendingUploads)
                break ;
            if (!u)
                return ;
            var upload = self.pendingUploads[u]
            self.currentUploads[upload.id] = upload;
            delete self.pendingUploads[upload.id];
            
            var formData = new FormData();
            formData.append("file", upload.file);
            upload.request.upload.addEventListener('progress', function (e) { self._progress(e, upload); });
            upload.request.onreadystatechange = function() { if (upload.request.readyState == 4) self._finished(upload); };
            upload.request.open('POST', '/c/command/uploads/send?disconnectOnError=true&cancelId=' + upload.cancelId + F.getSession(false));
            upload.request.send(formData);
        }
        
        // The upload has progressed.
        self._progress = function (e, upload)
        {
            if (e.total)
            {
                upload.size = e.total;
                upload.uploaded = e.loaded;
                upload.previousPercent = upload.percent;
                upload.percent = Math.min(e.loaded / e.total, 1) * 100;
            }
        }

        // The file has been uploaded.
        self._finished = function (upload)
        {
            upload.finished = true;
            if (!upload.cancelled)
            {
                var filesUploaded = jsonParse(upload.request.responseText || "[]");
                if (upload.request.status == 200 && filesUploaded.length && filesUploaded[0])
                    gl_files.list[upload.id].id = filesUploaded[0];
                else
                    console.log("An error occurred during the upload of " + upload.file.name);
                self._callOnUpload();
            }
            delete self.currentUploads[upload.id];
            // Starts the next upload
            self._start();
            // Removes the onUpload event if there is no file to upload
            for (var u in self.currentUploads)
                return ;
            clearInterval(self.onUploadInterval);
            delete self.onUploadInterval;
        }
        
        self._onDragover = function (e)
        {
            e.preventDefault();
        }
        
        self._onDrop = function (e)
        {
            e.preventDefault();
            gl_uploads.add(e.originalEvent.dataTransfer.files);
        }
    
        // Calls the onUpload event of the bound objects.
        self._callOnUpload = function ()
        {
            for (var i = 0; i < self.onUpload.length; ++i)
                self.onUpload[i].handler();
        }
    }
    
    self.init();
    return (self);
}
