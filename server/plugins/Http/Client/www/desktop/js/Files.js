// Manages the files.
var gl_files;

function Files()
{
    var self = this;
    gl_files = self;
    
    self.init = function ()
    {
        self.list = new Array(); // The list of the files of the user currently connected.
    }
    
    // Downloads the files list.
    self.onConnect = function ()
    {
        request("GET", "command/files/get", function (HttpRequest)
        {
            if (HttpRequest.status == 200)
                self.list = jsonParse(HttpRequest.responseText);
        });
    }
    
    // Clears the files list.
    self.onDisconnect = function ()
    {
        self.list = new Array();
    }
    
    self.init();
    return (self);
}
