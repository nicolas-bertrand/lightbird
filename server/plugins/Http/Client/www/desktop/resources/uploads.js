function Uploads(task)
{
    // Nodes
    this.node = new Object();
    this.node.uploads = getElementsByClassName("uploads", task.content, true);
    this.node.form = getElementsByClassName("form", task.content, true);
    this.node.sessions = getElementsByClassName("sessions", task.content, true);
    this.node.template = getElementsByClassName("session_template", task.content, true);
    // Initialization
    this.path = "/";
    this.createSession();
}

// Creates a new uploads session and displays the last session created.
Uploads.prototype.createSession = function ()
{
    var session = (new UploadsSession(this)).session;
    
    if (this.node.sessions.firstChild)
    {
        removeClassName(this.node.sessions.firstChild, "hidden");
        this.node.sessions.insertBefore(session, this.node.sessions.firstChild);
    }
    else
        this.node.sessions.appendChild(session);
    this.updateParity(session.nextSibling);
}

// Updates the odd/even class name of the sessions.
Uploads.prototype.updateParity = function (node)
{
    for (var i = 1; node; node = node.nextSibling, i++)
    {
        removeClassName(node, "odd");
        removeClassName(node, "even");
        if (i % 2)
            setClassName(node, "odd");
        else
            setClassName(node, "even");
    }
}

// Manages an upload session which consists of several files to upload.
// We need to create a new frame and files input for each session.
// uploads: The uploads task.
function UploadsSession(uploads)
{
    this.uploads = uploads; // The uploads task
    this.session = undefined; // The main node of the session
    this.node = new Object(); // Some nodes in the session
    this.node.progress = undefined; // The progress bar of the upload
    this.node.uploading = undefined; // Show the progress of an uploading file
    this.node.cancel = undefined; // Allows to cancel an upload session
    this.node.stop = undefined; // Allows to stop an upload session
    this.path = uploads.path; // The virtual directory where the file will be uploaded
    this.id = "upload" + Math.floor(Math.random() * 1000000000); // The id is used to send commands to the server
    this.row = undefined; // The row of the table in the progress bar
    this.files = undefined; // The list of the files uploaded by the session
    this.numberFilesUploaded = 0; // The number of files uploaded so far
    this.size = 0; // The total size of the upload
    this.currentFile = undefined; // The file being uploaded
    this.complete = false; // True if all the files have been uploaded
    var session = this;
    
    // Creates the session using the template
    this.session = document.createElement("div");
    setClassName(this.session, "session");
    setClassName(this.session, "hidden"); // The session is hidden until it is uploading something
    this.session.innerHTML = this.uploads.node.template.innerHTML;
    this.node.progress = getElementsByClassName("progress", this.session, true);
    this.node.uploading = getElementsByClassName("uploading", this.session, true);
    this.node.cancel = getElementsByClassName("cancel", this.session, true);
    this.node.stop = getElementsByClassName("stop", this.session, true);
    this.row = this.session.getElementsByTagName("table")[0].rows[0];
    this.files = this.row.cells;
    addEvent(this.node.cancel, "mousedown",  function () { session.cancel(); });
    addEvent(this.node.stop, "mousedown",  function () { session.stop(); });
    // Creates a new input
    this.uploads.node.form.target = this.id;
    this.uploads.node.form.innerHTML = "<input class=\"file\" type=\"file\" name=\"file\" multiple=\"true\" />";
    this.uploads.node.input = this.uploads.node.form.getElementsByTagName("input")[0];
    addEvent(this.uploads.node.input, "change",  function () { session.start(); });
    // Creates the frame of the session
    var frame = getElementsByClassName("frame", this.session, true);
    frame.innerHTML = "<iframe class=\"frame\" src=\"/Client/blank\" name=\"" + this.id + "\"></iframe>";
}

// Starts the upload of the files.
UploadsSession.prototype.start = function ()
{
    var uri = "Execute/Uploads";
    var id = this.id;
    var path = encodeURIComponent(this.path);
    this.uploads.node.form.action = "/Client/" + uri + "?id=" + id + "&path=" + path + "&token=" + getToken(uri);
    this.uploads.node.form.submit();
    this.createProgressBar();
    this.uploads.createSession();
}

// Creates the progression bar of the session and schedule its next update.
UploadsSession.prototype.createProgressBar = function ()
{
    var files = this.uploads.node.input.files;
    var previous;
    
    // If multiple upload is supported by the browser
    if (files)
    {
        // Calculates the total size of the upload
        for (var i = 0; i < files.length; ++i)
            this.size += files[i].size;
        // Computes the percentage of each file in the upload
        for (var i = 0; i < files.length; ++i)
        {
            this.row.insertCell(-1);
            var file = this.files[this.files.length - 1];
            var width = files[i].size / this.size * 100;
            file.style.width = width + "%";
            file.name = this.extractFilename(files[i].name);
            file.size = files[i].size;
            if (i > 0)
                previous = this.files[this.files.length - 2];
            file.length = (previous ? previous.length + previous.size : 0);
            file.progress = width + (previous ? previous.progress : 0);
            (i % 2) ? setClassName(file, "odd") : setClassName(file, "even");
        }
    }
    // Internet explorer doesn't support this feature yet
    else
    {
        this.row.insertCell(-1);
        this.files[0].name = this.extractFilename(this.uploads.node.input.value);
        this.files[0].size = 0;
        this.files[0].length = 0;
        this.files[0].progress = 100;
        setClassName(this.files[0], "odd");
    }
    if (this.files.length == 1)
        this.files[0].appendChild(this.node.uploading);
    // Updates the progress bar in one second
    var session = this;
    setTimeout(function () { session.requestProgress(); }, 1000);
}

// Sends a request to the server in order to get the progression of the upload,
// and update the progression bar.
UploadsSession.prototype.requestProgress = function ()
{
    var session = this;
    var callback = function (HttpRequest)
    {
        if (HttpRequest.status == 200)
        {
            var result = jsonParse(HttpRequest.responseText);
            // If the browser doesn't support the multiple upload, we didn't have the upload size
            if (!session.size)
            {
                session.size = result.size;
                session.files[0].size = result.size;
            }
            var percentage = result.progress / session.size * 100;
            // Schedule another update in one second
            if (result.complete === false)
                setTimeout(function () { session.requestProgress(); }, 1000);
            else
                percentage = 100;
            session.complete = result.complete;
            session.updateProgress(percentage);
        }
    }
    // Sends the request
    request("GET", "Execute/UploadsProgress?id=" + this.id, callback);
}

// Updates the progression bar.
// percentage: The progression to display.
UploadsSession.prototype.updateProgress = function (percentage)
{
    var uploaded = percentage * this.size / 100;
    
    for (var i = 0; i < this.files.length; ++i)
        if (this.files[i].progress >= percentage)
        {
            this.files[i].appendChild(this.node.uploading);
            this.node.uploading.style.width = (100 - (uploaded - this.files[i].length) / this.files[i].size * 100) + "%";
            if (this.currentFile)
                removeClassName(this.currentFile, "current_file");
            if (this.files.length > 1)
                setClassName(this.files[i], "current_file");
            this.currentFile = this.files[i];
            this.numberFilesUploaded = i + 1;
            break ;
        }
        else if (i >= this.numberFilesUploaded - 1)
            setClassName(this.files[i], "uploaded");
    if (percentage == 100)
    {
        removeClassName(this.currentFile, "current_file");
        setClassName(this.files[this.files.length - 1], "uploaded");
        this.node.uploading.parentNode.removeChild(this.node.uploading);
    }
}

// Stop an upload session.
UploadsSession.prototype.stop = function ()
{
    if (!this.complete)
        request("GET", "Execute/UploadsStop?id=" + this.id);
    this.session.parentNode.removeChild(this.session);
}

// Cancels an upload session. All the uploaded files are removed.
UploadsSession.prototype.cancel = function ()
{
    var files;

    if (this.complete)
        files = this.filesToJson();
    request("POST", "Execute/UploadsCancel?id=" + this.id, undefined, files, "application/json");
    this.session.parentNode.removeChild(this.session);
}

// Returns the files in the session as a json list.
UploadsSession.prototype.filesToJson = function ()
{
    var json = "[";
    
    if (json.length > 0)
        json = "[\"" + (this.path + "/" + this.files[0].name).replace(/\\/g, "\\\\").replace(/\"/g, "\\\"") + "\"";
    for (var i = 1; i < this.files.length; ++i)
        json += ",\"" + (this.path + "/" + this.files[i].name).replace(/\\/g, "\\\\").replace(/\"/g, "\\\"") + "\"";
    return (json + "]")
}

// Cleans the name of the file.
UploadsSession.prototype.extractFilename = function (path)
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

function initialize_uploads(task) { return new Uploads(task); }
gl_resources.jsLoaded("uploads");
