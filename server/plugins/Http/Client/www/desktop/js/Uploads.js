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
        var Upload =
        {
            id: 0, // A unique id in gl_uploads used to retrieve the upload in pendingUploads and currentUploads
            cancelId: "", // A uuid used to cancel the upload
            size: 0, // The size of the file
            uploaded: 0, // The amount of data uploaded so far
            percent: 0, // The percentage of the file uploaded
            previousPercent: undefined, // The percentage the last time onUpload was called
            request: {}, // The XMLHttpRequest
            file: {}, // The File to upload
            pending: true, // True if the upload was not started yet
            cancelled: false, // True if the upload has been cancelled
            finished: false // True if the upload is finished
        };
        self.nextId = 0; // The next id used to identify the uploads in pendingUploads and currentUploads.
        self.onUpload = new Array(); // The list of the functions to call while a file is uploading.
        self.onUploadInterval; // The interval that calls onUpload
        
        // These events allows to drop a file in the browser
        $(document.body).bind("dragover", self._onDragover);
        $(document.body).bind("drop", self._onDrop);
    }
    
    // Uploads the files in parameter.
    // @param fileList: The FileList to upload.
    self.add = function (fileList)
    {
        // Adds the file to the list
        var f = [];
        for (var i = 0; i < fileList.length; ++i)
            f.push({name: fileList[i].name, size: fileList[i].size, id_directory: ""});
        var files = gl_files.add(f);
        // setTimeout is used to let the browser display the files
        setTimeout(function ()
        {
            // Creates the uploads
            for (var i = 0; i < files.length; ++i)
            {
                if (files[i])
                    self._create(files[i], fileList[i]);
                else
                    console.log("The file " + fileList[i].name + " already exists in this directory.");
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
        {
            upload = files[i].upload;
            if (upload && upload.pending)
                delete self.pendingUploads[upload.id];
        }
        for (var i = 0; i < files.length; ++i)
        {
            upload = files[i].upload;
            if (upload && !upload.pending)
            {
                delete self.currentUploads[upload.id];
                upload.cancelled = true;
                upload.request.abort();
                cancelIds.push(upload.cancelId);
            }
            delete files[i].upload;
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
        self._create = function (file, fileData)
        {
            var request = new XMLHttpRequest();
            file.upload = 
            {
                id: self.nextId,
                cancelId: F.getUuid(),
                size: fileData.size,
                uploaded: 0,
                percent: 0,
                previousPercent: undefined,
                request: request,
                file: fileData,
                pending: true,
                cancelled: false,
                finished: false
            };
            self.pendingUploads[self.nextId] = file;
            self.nextId++;
            
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
            var file = self.pendingUploads[u];
            var upload = file.upload;
            upload.pending = false;
            self.currentUploads[u] = file;
            delete self.pendingUploads[u];
            
            var formData = new FormData();
            formData.append("file", upload.file);
            upload.request.upload.addEventListener('progress', function (e) { self._progress(e, upload); });
            upload.request.onreadystatechange = function() { if (upload.request.readyState == 4) self._finished(file); };
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
        self._finished = function (file)
        {
            var upload = file.upload;
            upload.finished = true;
            if (!upload.cancelled)
            {
                var filesUploaded = jsonParse(upload.request.responseText || "[]");
                if (upload.request.status == 200 && filesUploaded.length && filesUploaded[0])
                    file.info.id = filesUploaded[0];
                else
                    console.log("An error occurred during the upload of " + upload.file.name);
                self._callOnUpload();
            }
            delete self.currentUploads[upload.id];
            delete file.upload;
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
