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
// The html of the resource is given in parameter.
self.load = function (resource, callback)
{
    if (self[resource] == undefined)
        self.register(resource);
    resource = self[resource];
    if (callback != undefined)
        resource.callbacks.push(callback);
    if (resource.loading)
        return ;
    // Load the resource
    if (!resource.loaded)
    {
        resource.loading = true;
        // Load the HTML
        F.request("GET", resource.files.html, function(HttpRequest)
        {
            resource.html = HttpRequest.responseText;
            self.loaded(resource);
        });
        // Load the CSS
        F.request("GET", resource.files.css, function(HttpRequest)
        {
            resource.css = HttpRequest.responseText;
            self.loaded(resource);
        });
        // Load the JavaScript
        $("head")[0].appendChild($("<script></script>").attr("src", "/c/" + resource.files.js)[0]);
    }
    // The resource has already been loaded
    else
        self.loaded(resource);
}

// Called by the resource javascript the first time it is loaded.
self.jsLoaded = function (resource)
{
    if (self[resource] && self[resource].loading)
    {
        self[resource].js = true;
        self.loaded(self[resource]);
    }
}

// If all the files of the resource have been loaded, its callbacks are called.
self.loaded = function (resource)
{
    if (!resource.html || !resource.css || !resource.js)
        return ;
    if (resource.loading)
    {
        resource.loading = false;
        resource.loaded = true;
        resource.css = $("<style></style>").html(resource.css).appendTo($("head"));
    }
    if (resource.loaded)
    {
        // Callbacks
        for (i = 0, s = resource.callbacks.length; i < s; ++i)
            resource.callbacks[i](resource.html);
        resource.callbacks = new Array();
    }
}

// Creates a new instance of the resource by calling "'initialize_resource_' + resource".
// The two last parameters are passed to it.
// @return The new instance.
self.initialize = function(resource, task, parameter)
{
    if (self[resource] && self[resource].loaded)
        return (window[self[resource].initialize](task, parameter));
}

// Registers a new resource that can be loaded an any time using the load method.
// @resource : The name of the resource to register. It must correspond to the
// files "resource.(css|js|html)". The JavaScript must have a method
// "'initialize_resource_' + resource" which is used to create the instances of the resource.
self.register = function(resource)
{
    self[resource] = new Object();
    self[resource].files = {
        html: "resources/" + resource + ".html",
        css: "resources/" + resource + ".css",
        js: "resources/" + resource + ".js"
    };
    self[resource].initialize = "initialize_resource_" + resource;
    self[resource].loading = false;
    self[resource].loaded = false;
    self[resource].html = "";
    self[resource].css = "";
    self[resource].js = "";
    self[resource].callbacks = new Array();
}

    self.init();
    return (self);
}
