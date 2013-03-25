function ResourceUploads(task)
{
    var self = this;
    
    self.init = function()
    {
        // Nodes
        self.node = new Object();
        self.node.uploads = getElementsByClassName("uploads", task.content, true);
        self.node.form = getElementsByClassName("form", task.content, true);
        self.node.file = getElementsByClassName("file", self.node.form, true);
        self.node.file.input = self.node.file.getElementsByTagName("input")[0];
        self.node.path = getElementsByClassName("path", self.node.form, true).getElementsByTagName("input")[0];
        self.node.submit = getElementsByClassName("submit", self.node.form, true);
        self.node.sessions = getElementsByClassName("sessions", task.content, true);
        self.node.template = getElementsByClassName("session_template", task.content, true);
        // Initialization
        self.path = "/";
        self.node.path.defaultValue = T.Uploads.destination_folder;
        self.node.path.value = self.node.path.defaultValue;
        self.node.submit.value = T.Uploads.upload;
        $(self.node.path).focus(function (e) { self.mousePathInput(e); });
        $(self.node.path).blur(function (e) { self.mousePathInput(e); });
        $(self.node.file).mousemove(function (e) { self.mouseMoveFile(e); });
        self.createSession();
    }

    // Creates a new uploads session.
    self.createSession = function ()
    {
        var session = (new UploadsSession(self)).session;
        
        $(self.node.sessions).prepend(session);
        self.updateParity(self.node.sessions);
    }

    // Hides/displays the default text when the path input is focus/blur.
    self.mousePathInput = function (e)
    {
        if (e.type == "focus" && self.node.path.value == self.node.path.defaultValue)
            self.node.path.value = "";
        else if (e.type == "blur" && self.node.path.value == "")
            self.node.path.value = self.node.path.defaultValue;
        if (self.node.path.value != self.node.path.defaultValue)
            $(self.node.path).addClass("focus");
        else
            $(self.node.path).removeClass("focus");
    }

    // Moves the input file under the mouse so that the user can select a file anywhere in the file div.
    self.mouseMoveFile = function (e)
    {
        var offset = $(self.node.file).offset();
        var input = self.node.file.input;
        input.style.left = (e.pageX - offset.left) - (input.offsetWidth - 30) + 'px';
        input.style.top = (e.pageY - offset.top) - (input.offsetHeight / 2)  + 'px';
    }

    // Updates the odd/even class name of the childs of the parent.
    self.updateParity = function (parent, limit)
    {
        var node = parent.firstChild;
        for (var i = 1; node && (!limit || i < limit); node = node.nextSibling, i++)
        {
            $(node).removeClass("odd");
            $(node).removeClass("even");
            if (i % 2)
                $(node).addClass("odd");
            else
                $(node).addClass("even");
        }
    }
    
    self.init();
    return (self);
}

// Manages an upload session which consists of several files to upload.
// We need to create a new frame and files input for each session.
// @param uploads : The uploads task.
function UploadsSession(uploads)
{
    var self = this;
    
    self.init = function()
    {
        self.uploads = uploads; // The uploads task
        self.session = undefined; // The main node of the session
        self.node = new Object(); // Some nodes in the session
        self.node.progress_container = undefined; // The progress bar container
        self.node.progress = undefined; // The progress bar of the upload
        self.node.uploading = undefined; // Show the progress of an uploading file
        self.node.chart = undefined; // The upload chart
        self.node.files_list = undefined; // The list of the files that will be uploaded
        self.node.cancel = undefined; // Allows to cancel an upload session
        self.node.stop = undefined; // Allows to stop an upload session
        self.path = uploads.path; // The virtual directory where the file will be uploaded
        self.filesInDirectory = undefined; // The list of the files in the path
        self.id = F.getUuid(); // The id is used to send commands to the server
        self.row = undefined; // The row of the table in the progress bar
        self.files = undefined; // The list of the files uploaded by the session
        self.numberFilesUploaded = 0; // The number of files uploaded so far
        self.size = 0; // The total size of the upload
        self.currentFile = undefined; // The file being uploaded
        self.complete = false; // True if all the files have been uploaded
        
        // Creates the session using the template
        self.session = document.createElement("div");
        $(self.session).addClass("session");
        $(self.session).addClass("hidden"); // The session is hidden until it is uploading something
        self.session.innerHTML = self.uploads.node.template.innerHTML;
        self.node.progress_container = getElementsByClassName("progress_container", self.session, true);
        self.node.progress = getElementsByClassName("progress", self.session, true);
        self.node.uploading = getElementsByClassName("uploading", self.session, true);
        self.node.chart = getElementsByClassName("chart", self.session, true);
        self.node.files_list = getElementsByClassName("files_list", self.session, true);
        self.node.cancel = getElementsByClassName("cancel", self.session, true);
        self.node.stop = getElementsByClassName("stop", self.session, true);
        self.row = self.session.getElementsByTagName("table")[0].rows[0];
        self.files = self.row.cells;
        $(self.node.cancel).mousedown(function () { self.cancel(); });
        $(self.node.stop).mousedown(function () { self.stop(); });
        // Creates a new input for the files
        self.uploads.node.form.target = self.id;
        $(self.uploads.node.file.input).remove();
        self.uploads.node.file.input = document.createElement("input");
        self.uploads.node.file.input.type = "file";
        self.uploads.node.file.input.name = "file";
        self.uploads.node.file.input.multiple = "true";
        self.uploads.node.file.appendChild(self.uploads.node.file.input);
        $(self.uploads.node.file.input).change(function () { self.check(); });
        // Creates the frame of the session
        var frame = getElementsByClassName("frame", self.session, true);
        frame.innerHTML = "<iframe class=\"frame\" src=\"/c/blank\" name=\"" + self.id + "\"></iframe>";
    }
    
    // Called when the user has selected the files to upload.
    // Sends a request to the server to get the list of the files in the directory and detect conflicts.
    self.check = function ()
    {
        // Creates the progress bar and displays the session
        self.createProgressBar();
        $(self.session).removeClass("hidden");
        // Sends the request to get the list of the files in the path
        if ((self.path = self.uploads.node.path.value) == self.uploads.node.path.defaultValue)
            self.path = "";
        var path = encodeURIComponent(self.path);
        F.request("POST", "command/uploads/check?path=" + path, function (HttpRequest)
        {
            if (HttpRequest.status == 200)
                self.checkResult(jsonParse(HttpRequest.responseText));
        });
        // Creates a new session for the next time the user chooses files
        self.uploads.createSession();
        // If an older session has started to upload yet, it is removed
        $(self.session.nextSibling).remove();
    }

    // Displays the files that will be uploaded.
    // @param filesInDirectory : The list of the files already in the upload directory.
    self.checkResult = function (filesInDirectory)
    {
        var j;
        var lastConflict = 0;
        var row;
        var textWidth = false;
        
        self.filesInDirectory = filesInDirectory;
        for (var i = 0; i < self.files.length; ++i)
        {
            // The file is already on the server
            if ((j = self.filesInDirectory.indexOf(self.files[i].name)) != -1)
            {
                self.files[i].rename = self.checkFileName(self.files[i].name);
                $(self.files[i]).addClass("conflict");
                row = self.node.files_list.insertRow(lastConflict++);
                row.insertCell(-1).innerHTML = self.files[i].name;
                $(row).addClass("conflict");
                // The double click on the file name cell switches between the rename and ignore state
                $(row.cells[0]).dblclick(function (e) { self.checkAction(e.delegateTarget.parentNode, !$(e.delegateTarget.parentNode.cells[0]).hasClass("rename") ? "rename" : "ignore"); });
                // Size
                row.size = row.insertCell(-1);
                $(row.size).addClass("size");
                if (self.files[i].size)
                    row.size.innerHTML = F.sizeToString(self.files[i].size);
                // Ignore
                row.ignore = row.insertCell(-1)
                row.ignore.innerHTML = T.Uploads.ignore;
                $(row.ignore).addClass("action selected");
                $(row.ignore).click(function (e) { self.checkAction(e.delegateTarget.parentNode, "ignore"); });
                // Replace
                row.replace = row.insertCell(-1);
                row.replace.innerHTML = T.Uploads.replace;
                $(row.replace).addClass("action");
                $(row.replace).click(function (e) { self.checkAction(e.delegateTarget.parentNode, "replace"); });
                // Rename
                row.rename = row.insertCell(-1)
                row.rename.innerHTML = T.Uploads.rename;
                $(row.rename).addClass("action");
                $(row.rename).click(function (e) { self.checkAction(e.delegateTarget.parentNode, "rename"); });
            }
            // The file is not on the server
            else
            {
                row = self.node.files_list.insertRow(-1);
                row.insertCell(-1).innerHTML = self.files[i].name;
                $(row.cells[0]).dblclick(function (e) { self.checkAction(e.delegateTarget.parentNode, "rename"); });
                // Size
                row.size = row.insertCell(-1);
                $(row.size).addClass("size");
                if (self.files[i].size)
                    row.size.innerHTML = F.sizeToString(self.files[i].size);
                // Ignore
                row.ignore = row.insertCell(-1)
                row.ignore.innerHTML = T.Uploads.ignore;
                $(row.ignore).addClass("action");
                $(row.ignore).click(function (e) { self.checkAction(e.delegateTarget.parentNode, "ignore"); });
                if (lastConflict)
                    row.insertCell(-1);
                // Rename
                row.rename = row.insertCell(-1);
                row.rename.innerHTML = T.Uploads.rename;
                $(row.rename).addClass("action");
                $(row.rename).click(function (e) { self.checkAction(e.delegateTarget.parentNode, "rename"); });
            }
            row.file = self.files[i];
            // Sets the width of the columns
            if (!textWidth)
            {
                row.size.style.width = $(row.size).textWidth() + "px";
                row.ignore.style.width = $(row.ignore).textWidth() + "px";
                if (row.replace)
                    row.replace.style.width = $(row.replace).textWidth() + "px";
                row.rename.style.width = $(row.rename).textWidth() + "px";
                textWidth = true;
            }
        }
        self.uploads.updateParity(self.node.files_list.firstChild);
    }
    
    // Ensures that the file name unique by adding adding a number if it already exists.
    self.checkFileName = function (name)
    {
        var extension = "";
        if (name.lastIndexOf('.') != -1)
        {
            extension = name.substr(name.lastIndexOf('.'));
            name = name.substr(0, name.lastIndexOf('.'));
        }
        return (self.checkUniqueName(name, extension) + extension);
    }
    self.checkUniqueName = function (name, extension)
    {
        if (self.filesInDirectory.indexOf(name + extension) == -1)
            return (name);
        if (/ - [0-9]+$/.test(name))
        {
            var n = Number(name.match(/[0-9]+$/)) + 1;
            name = name.replace(/[0-9]+$/, n);
        }
        else
            name += " - 1";
        return (self.checkUniqueName(name, extension));
    }
    
    // Changes the action to apply on a file.
    // @param row : The row of the file.
    // @param action : The action to apply (ignore, replace, rename).
    self.checkAction = function (row, action)
    {
        var rows = self.node.files_list.rows;
        var i = 0;
        
        $(row["ignore"]).removeClass("selected");
        $(row["replace"]).removeClass("selected");
        $(row["rename"]).removeClass("selected");
        // The file is already on the server
        if ($(row).hasClass("conflict"))
        {
            $(row[action]).addClass("selected");
            if (action == "rename")
            {
                if (!$(row.cells[0]).hasClass("rename"))
                {
                    $(row.cells[0]).addClass("rename");
                    var input = document.createElement("input");
                    input.type = "text";
                    input.value = row.file.rename;
                    row.cells[0].innerHTML = "";
                    row.cells[0].appendChild(input);
                    input.focus();
                    $(input).addClass("rename");
                    $(input).keydown(function (e) { self.checkInputKeyDown(e); });
                    $(input).blur(function (e) { self.checkInputBlur(e); });
                }
                else
                    row.cells[0].firstChild.focus();
            }
            else
            {
                $(row.cells[0]).removeClass("rename");
                row.cells[0].innerHTML = row.file.name;
            }
        }
        else
        {
            $(row).removeClass("change");
            $(row).removeClass("renameConflict");
            $(row.cells[0]).removeClass("rename");
            // Applies the action
            if (row.state != action)
            {
                $(row[action]).addClass("selected");
                $(row).addClass("change");
                row.state = action;
                if (action == "rename")
                {
                    $(row.cells[0]).addClass("rename");
                    var input = document.createElement("input");
                    input.type = "text";
                    input.value = row.file.name;
                    row.cells[0].innerHTML = "";
                    row.cells[0].appendChild(input);
                    input.focus();
                    $(input).keydown(function (e) { self.checkInputKeyDown(e); });
                }
                else
                {
                    row.cells[0].innerHTML = row.file.name;
                }
            }
            // Cancels the action
            else
            {
                delete row.state;
                row.cells[0].innerHTML = row.file.name;
            }
        }
    }
    
    // Called each the input that contains the name of the file is modified.
    self.checkInputKeyDown = function (e)
    {
        // The timeout is used to wait that the value of the input has changed
        setTimeout(function ()
        {
            // If enter is pressed we leave the input
            if (e.which == 13)
                e.target.blur();
            // The name of the file has changed
            if (e.target.parentNode && e.target.value != e.target.parentNode.parentNode.file.name)
                $(e.target).addClass("rename");
            else
                $(e.target).removeClass("rename");
            if (!$(e.target).hasClass("conflict") && $(e.target).hasClass("rename"))
            {
                if (self.filesInDirectory.indexOf(e.target.value) != -1)
                    $(e.target.parentNode.parentNode).addClass("renameConflict");
                else
                    $(e.target.parentNode.parentNode).removeClass("renameConflict");
            }
        }, 0);
    }
    
    // Called when the file name input blur.
    self.checkInputBlur = function (e)
    {
        // If the value of the input is the original name of the file, it has not been renamed
        if (e.target.value == e.target.parentNode.parentNode.file.name)
            self.checkAction(e.target.parentNode.parentNode, "ignore");
    }

    // Starts the upload of the files.
    self.start = function ()
    {
        // Submit the form
        var id = self.id;
        var path = encodeURIComponent(self.path);
        self.uploads.node.form.action = "/c/command/uploads?id=" + id + "&path=" + path + F.getSession();
        self.uploads.node.form.submit();
        // Updates the progress bar in one second
        setTimeout(function () { self.requestProgress(); }, C.Uploads.requestProgressInterval);
    }

    // Creates the progression bar of the session and schedule its next update.
    self.createProgressBar = function ()
    {
        var files = self.uploads.node.file.input.files;
        var previous;
        
        // If multiple upload is supported by the browser
        if (files)
        {
            // Calculates the total size of the upload
            for (var i = 0; i < files.length; ++i)
                self.size += files[i].size;
            // Computes the percentage of each file in the upload
            for (var i = 0; i < files.length; ++i)
            {
                self.row.insertCell(-1);
                var file = self.files[self.files.length - 1];
                var width = files[i].size / self.size * 100;
                file.style.width = width + "%";
                file.name = self.extractFileName(files[i].name);
                file.size = files[i].size;
                if (i > 0)
                    previous = self.files[self.files.length - 2];
                file.length = (previous ? previous.length + previous.size : 0);
                file.progress = width + (previous ? previous.progress : 0);
                (i % 2) ? $(file).addClass("odd") : $(file).addClass("even");
            }
        }
        // Internet explorer doesn't support this feature yet
        else
        {
            self.row.insertCell(-1);
            self.files[0].name = self.extractFileName(self.uploads.node.file.input.value);
            self.files[0].size = 0;
            self.files[0].length = 0;
            self.files[0].progress = 100;
            $(self.files[0]).addClass("even");
        }
        /*if (self.files.length == 1)
            self.files[0].appendChild(self.node.uploading);*/
    }

    // Sends a request to the server in order to get the progression of the upload,
    // and update the progression bar.
    self.requestProgress = function ()
    {
        var callback = function (HttpRequest)
        {
            if (HttpRequest.status == 200)
            {
                var result = jsonParse(HttpRequest.responseText);
                // If the browser doesn't support the multiple upload, we didn't have the upload size
                if (!self.size)
                {
                    self.size = result.size;
                    self.files[0].size = result.size;
                }
                var percentage = result.progress / self.size * 100;
                // Schedule another update in one second
                if (result.complete === false)
                    setTimeout(function () { self.requestProgress(); }, C.Uploads.requestProgressInterval);
                else
                    percentage = 100;
                self.complete = result.complete;
                if (result.size)
                    self.updateProgress(percentage);
            }
        }
        // Sends the request
        F.request("GET", "command/uploads/progress?id=" + self.id, callback);
    }

    // Updates the progression bar.
    // percentage: The progression to display.
    self.updateProgress = function (percentage)
    {
        var uploaded = percentage * self.size / 100;
        
        for (var i = 0; i < self.files.length; ++i)
            if (self.files[i].progress >= percentage)
            {
                self.files[i].appendChild(self.node.uploading);
                self.node.uploading.style.width = (100 - (uploaded - self.files[i].length) / self.files[i].size * 100) + "%";
                if (self.currentFile)
                    $(self.currentFile).removeClass("current_file");
                if (self.files.length > 1)
                    $(self.files[i]).addClass("current_file");
                self.currentFile = self.files[i];
                self.numberFilesUploaded = i + 1;
                break ;
            }
            else if (i >= self.numberFilesUploaded - 1)
                $(self.files[i]).addClass("uploaded");
        if (percentage == 100)
        {
            $(self.currentFile).removeClass("current_file");
            $(self.files[self.files.length - 1]).addClass("uploaded");
            $(self.node.uploading).remove();
        }
    }

    // Stop an upload session.
    self.stop = function ()
    {
        if (!self.complete)
            F.request("GET", "command/uploads/stop?id=" + self.id);
        $(self.session).remove();
        self.uploads.updateParity();
    }

    // Cancels an upload session. All the uploaded files are removed.
    self.cancel = function ()
    {
        var files;
        var path = encodeURIComponent(self.path);

        if (self.complete)
            files = self.filesToJson();
        F.request("POST", "command/uploads/cancel?id=" + self.id + "&path=" + path, undefined, files, "application/json");
        $(self.session).remove();
        self.uploads.updateParity();
    }

    // Returns the files in the session as a json list.
    self.filesToJson = function ()
    {
        var json = "[";
        
        if (json.length > 0)
            json = "[\"" + (self.files[0].name).replace(/\\/g, "\\\\").replace(/\"/g, "\\\"") + "\"";
        for (var i = 1; i < self.files.length; ++i)
            json += ",\"" + (self.files[i].name).replace(/\\/g, "\\\\").replace(/\"/g, "\\\"") + "\"";
        return (json + "]")
    }

    // Cleans the name of the file.
    self.extractFileName = function (path)
    {
        if (path.substr(0, 12) == "C:\\fakepath\\")
            return path.substr(12); // modern browser
        var x = path.lastIndexOf('/');
        if (x >= 0) // Unix-based path
            return (path.substr(x + 1));
        x = path.lastIndexOf('\\');
        if (x >= 0) // Windows-based path
            return (path.substr(x + 1));
        return (path); // just the filename
    }
    
    self.init();
    return (self);
}

function initialize_resource_uploads(task) { return (new ResourceUploads(task)); }
gl_resources.jsLoaded("uploads");
