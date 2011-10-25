/* This file holdes the resource manager.  */

// The resources singleton
var gl_resources;
var gl_errorMessage = "<h2>Erreur !</h2><p>Impossible de r&eacute;cup&eacute;rer le contenu de la page =/</p>";

function Resources()
{
	// Manage the explorer window
	this.Explorer = new Object();
	this.Explorer.html = "Explorer.html";
	this.Explorer.css = "Explorer.css";
	this.Explorer.js = "Explorer.js";
	this.Explorer.callback = "initializeExplorer";
	this.Explorer.loaded = false;
	this.Explorer.jsLoaded = false;
	this.Explorer.loading = false;
	this.Explorer.queue = new Array();
	
	// Display and modify informations on a file
	this.Information = new Object();
	this.Information.html = "Information.html";
	this.Information.css = "Information.css";
	this.Information.js = "Information.js";
	this.Information.callback = "initializeInformation";
	this.Information.loaded = false;
	this.Information.jsLoaded = false;
	this.Information.loading = false;
	this.Information.queue = new Array();
	
	// The media player
	this.Player = new Object();
	this.Player.html = "Player.html";
	this.Player.css = "Player.css";
	this.Player.js = "Player.js";
	this.Player.callback = "initializePlayer";
	this.Player.loaded = false;
	this.Player.jsLoaded = false;
	this.Player.loading = false;
	this.Player.queue = new Array();
	
	// The uploader
	this.Upload = new Object();
	this.Upload.html = "Upload.html";
	this.Upload.css = "Upload.css";
	this.Upload.js = "Upload.js";
	this.Upload.callback = "initializeUpload";
	this.Upload.loaded = false;
	this.Upload.jsLoaded = false;
	this.Upload.loading = false;
	this.Upload.queue = new Array();
	
	this.load("Explorer");
	this.load("Information");
	this.load("Player");
	this.load("Upload");
}

// Load a resource.
// @resource : The name og the resource to load.
// @callback : A function that will be called when the resource has been loaded.
// The content of the html is given in parameter.
// @return True if the resource is going to be loaded.
Resources.prototype.load = function (resource, callback)
{
	if (this[resource] == undefined)
	{
		callback(gl_errorMessage);
		return (false);
	}
	if (callback != undefined)
		this[resource].queue.push(callback);
	if (this[resource].loading)
		return ;
	// Load the resource
	if (!this[resource].loaded)
	{
		this[resource].loading = true;
		// Load the HTML
		request("GET", "html/" + this[resource].html, function(HttpRequest)
		{
			if (HttpRequest.status == 200)
				gl_resources[resource].content = HttpRequest.responseText;
			else
				gl_resources[resource].content = gl_errorMessage;
			if (gl_resources[resource].jsLoaded ||
				gl_resources[resource].js == undefined)
				gl_resources.loaded(resource);
		});
		// Load the CSS
		if (this[resource].css != undefined)
			loadJsCssFile("css/" + this[resource].css);
		// Load the JavaScript
		if (this[resource].js != undefined)
			loadJsCssFile("js/" + this[resource].js);
	}
	// The resource has already been loaded
	else if (this[resource].content != undefined)
		gl_resources.loaded(resource);
	return (true);
}

Resources.prototype.jsLoaded = function (resource)
{
	this[resource].jsLoaded = true;
	if (this[resource].content != undefined)
		this.loaded(resource);
}

Resources.prototype.loaded = function (resource)
{
	this[resource].loading = false;
	this[resource].loaded = true;
	for (i = 0, s = this[resource].queue.length; i < s; ++i)
		this[resource].queue[i](this[resource].content, this[resource].callback);
	this[resource].queue.splice(0, this[resource].queue.length);
}

Resources.prototype.callJs = function(resource, parameter)
{
	if (this[resource] != undefined && this[resource].callback != undefined)
		eval(this[resource].callback + "(\"" + parameter + "\")");
}

// Load a javascript or a css file.
// @name : The file to load. Its extension if used to find its type.
function loadJsCssFile(name)
{
	var node;

	if (name.match("\.js$") != null)
	{
		node = document.createElement("script");
		node.setAttribute("type", "text/javascript");
		node.setAttribute("src", "Client/" + name);
	}
	else if (name.match("\.css$") != null)
	{
		node = document.createElement("link");
		node.setAttribute("rel", "stylesheet");
		node.setAttribute("type", "text/css");
		node.setAttribute("href", "Client/" + name);
	}
	if (node != undefined)
		document.getElementsByTagName("head")[0].appendChild(node);
}
