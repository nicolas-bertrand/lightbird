/* This file holdes the resource manager.  */

// The resources singleton
var gl_resources;
var gl_errorMessage = "<h2>Erreur !</h2><p>Impossible de r&eacute;cup&eacute;rer le contenu de la page =/</p>";

function Resources()
{
	// Load the resources
	/*this.load("Explorer");
	this.load("Information");
	this.load("Player");
	this.load("Upload");*/
}

// Load a resource.
// @resource : The name of the resource to load.
// @callback : A function that will be called when the resource has been loaded.
// The content of the html is given in parameter.
// @return True if the resource is going to be loaded.
Resources.prototype.load = function (resource, callback)
{
	if (this[resource] == undefined)
        this.initialize(resource);
	if (callback != undefined)
		this[resource].queue.push(callback);
	if (this[resource].loading)
		return (true);
	// Load the resource
	if (!this[resource].loaded)
	{
		this[resource].loading = true;
		// Load the HTML
		request("GET", this[resource].html, function(HttpRequest)
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
			loadJsCssFile(this[resource].css);
		// Load the JavaScript
		if (this[resource].js != undefined)
			loadJsCssFile(this[resource].js);
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
		window[this[resource].callback](parameter);
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

// Register a new resource that can be loaded an any time using the method load.
// @resource : The name of the resource to register. It must correspond to the
// files "resource.(css|js|html)". The Javascript must have a method
// "initialize + resource" which is called the first time the resource is loaded.
Resources.prototype.initialize = function(resource)
{
	this[resource] = new Object();
	this[resource].html = "resources/" + resource + ".html";
	this[resource].css = "resources/" + resource + ".css";
	this[resource].js = "resources/" + resource + ".js";
	this[resource].callback = "initialize_" + resource;
	this[resource].loaded = false;
	this[resource].jsLoaded = false;
	this[resource].loading = false;
	this[resource].queue = new Array();
}
