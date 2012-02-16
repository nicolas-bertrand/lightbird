function Uploads(task)
{
    var upload = this;
    // Nodes
    this.node = new Object();
    this.node.uploads = getElementsByClassName("uploads", task.content, true);
    this.node.form = getElementsByClassName("form", task.content, true);
    this.node.input = getElementsByClassName("file", task.content, true);
    this.node.frame = getElementsByClassName("frame", task.content, true);
    // Default values
    this.id = "upload" + Math.floor(Math.random() * 1000000000);
    this.path = "/";
    this.node.form.target = this.id;
    this.node.frame.innerHTML = "<iframe class=\"frame\" src=\"/Client/blank\" name=\"" + this.id + "\"></iframe>";
    addEvent(this.node.input, "change",  function () { upload.start(); });
}

Uploads.prototype.start = function ()
{
    var uri = "Execute/Uploads";
    this.node.form.action = "/Client/" + uri + "?id=" + this.id + "&path=" + encodeURIComponent(this.path) + "&token=" + getToken(uri);
    this.node.form.submit();
    console.log(this.node.form.action);
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
