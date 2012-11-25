// Manages the files.
var gl_files;

function Files()
{
    var self = this;
    gl_files = self;
    
    self.init = function ()
    {
    }
    
    // Downloads the files list
    self.getFiles = function ()
    {
        request("GET", "command/files/get", function (HttpRequest)
        {
            if (HttpRequest.status == 200)
                self.list = jsonParse(HttpRequest.responseText);
        });
    }
    
    self.init();
    return (self);
}
