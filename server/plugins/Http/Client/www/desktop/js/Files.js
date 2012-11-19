// The files singleton
var gl_files;

// Manages the files.
function Files()
{
    var self = this;
    gl_files = self;
    
    self.init = function ()
    {
        // Downloads the files list
        request("GET", "Execute/FilesGet", function (HttpRequest)
        {
            if (HttpRequest.status == 200)
            {
                self.list = jsonParse(HttpRequest.responseText);
            }
        });
    }
    
    self.init();
    return (self);
}
