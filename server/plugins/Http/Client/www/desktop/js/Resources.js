// Manages the resources.
var gl_resources;

function Resources()
{
    var self = this;
    gl_resources = self;
    
    self.init = function ()
    {
    }

// Load a resource.
// @resource : The name of the resource to load.
// @callback : A function that will be called when the resource has been loaded.
// The content of the html is given in parameter.
// @return True if the resource is going to be loaded.
self.load = function (resource, callback)
{
	if (self[resource] == undefined)
        self.initialize(resource);
	if (callback != undefined)
		self[resource].queue.push(callback);
	if (self[resource].loading)
		return (true);
	// Load the resource
	if (!self[resource].loaded)
	{
		self[resource].loading = true;
		// Load the HTML
		request("GET", self[resource].html, function(HttpRequest)
		{
			if (HttpRequest.status == 200)
				gl_resources[resource].content = HttpRequest.responseText;
			else
				gl_resources[resource].content = "<h2>Unable to load the resource</h2>";
			if (gl_resources[resource].jsLoaded ||
				gl_resources[resource].js == undefined)
				gl_resources.loaded(resource);
		});
		// Load the CSS
		if (self[resource].css != undefined)
			loadJsCssFile(self[resource].css);
		// Load the JavaScript
		if (self[resource].js != undefined)
			loadJsCssFile(self[resource].js);
	}
	// The resource has already been loaded
	else if (self[resource].content != undefined)
		gl_resources.loaded(resource);
	return (true);
}

self.jsLoaded = function (resource)
{
	self[resource].jsLoaded = true;
	if (self[resource].content != undefined)
		self.loaded(resource);
}

self.loaded = function (resource)
{
	self[resource].loading = false;
	self[resource].loaded = true;
	for (i = 0, s = self[resource].queue.length; i < s; ++i)
		self[resource].queue[i](self[resource].content, self[resource].callback);
	self[resource].queue.splice(0, self[resource].queue.length);
}

self.callJs = function(resource, task, parameter)
{
	if (self[resource] != undefined && self[resource].callback != undefined)
		window[self[resource].callback](task, parameter);
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
		node.setAttribute("src", "/Client/" + name);
	}
	else if (name.match("\.css$") != null)
	{
		node = document.createElement("link");
		node.setAttribute("rel", "stylesheet");
		node.setAttribute("type", "text/css");
		node.setAttribute("href", "/Client/" + name);
	}
	if (node != undefined)
		document.getElementsByTagName("head")[0].appendChild(node);
}

// Register a new resource that can be loaded an any time using the method load.
// @resource : The name of the resource to register. It must correspond to the
// files "resource.(css|js|html)". The Javascript must have a method
// "initialize + resource" which is called the first time the resource is loaded.
self.initialize = function(resource)
{
	self[resource] = new Object();
	self[resource].html = "resources/" + resource + ".html";
	self[resource].css = "resources/" + resource + ".css";
	self[resource].js = "resources/" + resource + ".js";
	self[resource].callback = "initialize_resource_" + resource;
	self[resource].loaded = false;
	self[resource].jsLoaded = false;
	self[resource].loading = false;
	self[resource].queue = new Array();
}

    self.init();
    return (self);
}
