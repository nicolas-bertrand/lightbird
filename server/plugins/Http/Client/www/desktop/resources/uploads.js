function Uploads(task)
{
    var upload = this;
    // Nodes
    this.node = new Object();
    this.node.uploads = getElementsByClassName("uploads", task.content, true);
    this.node.form = getElementsByClassName("form", task.content, true);
    this.node.input = getElementsByClassName("file", task.content, true);
    this.node.sessions = getElementsByClassName("sessions", task.content, true);
    this.node.template = getElementsByClassName("session_template", task.content, true);
    // Initialization
    addEvent(this.node.input, "change",  function () { upload.start(); });
    this.path = "/";
    this.prepare();
}

Uploads.prototype.prepare = function ()
{
    var session = document.createElement("div");
    setClassName(session, "session");
    setClassName(session, "hidden");
    session.id = "upload" + Math.floor(Math.random() * 1000000000);
    session.innerHTML = this.node.template.innerHTML;
    this.node.form.target = session.id;
    var frame = getElementsByClassName("frame", session, true);
    frame.innerHTML = "<iframe class=\"frame\" src=\"/Client/blank\" name=\"" + session.id + "\"></iframe>";
    if (this.node.sessions.firstChild)
    {
        removeClassName(this.node.sessions.firstChild, "hidden");
        this.node.sessions.insertBefore(session, this.node.sessions.firstChild);
    }
    else
        this.node.sessions.appendChild(session);
}

Uploads.prototype.start = function ()
{
    var uri = "Execute/Uploads";
    var id = this.node.sessions.firstChild.id;
    var path = encodeURIComponent(this.path);
    this.node.form.action = "/Client/" + uri + "?id=" + id + "&path=" + path + "&token=" + getToken(uri);
    this.node.form.submit();
    this.prepare();
}

Uploads.prototype.extractFilename = function (path)
{
    if (path.substr(0, 12) == "C:\\fakepath\\")
        return (path.substr(12)); // modern browser
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
