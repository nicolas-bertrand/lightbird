function ResourceUploads(task)
{
    var resource = this;
    
    resource.init = function ()
    {
        // Nodes
        resource.node = new Object();
        resource.node.root = task.content.children(".uploads");
        resource.node.form = resource.node.root.children("form");
        resource.node.input = resource.node.form.children("input.file");
        
        // Events
        resource.node.input.change(resource.onChange);
    }
    
    resource.onResize = function (left, top, width, height)
    {
    }
    
    resource.close = function ()
    {
        for (var key in resource)
            resource[key] = undefined;
    }
    
    resource.onChange = function (e)
    {
        if (this.files.length <= 0)
            return ;
        var formData = new FormData();
        formData.append("file", this.files[0]);
        
        var request = new XMLHttpRequest();
        request.onreadystatechange = function()
        {
            if(request.readyState == 4)
                console.log(request.response);
        };

        request.upload.addEventListener('progress', function(e){
            console.log("progress", Math.ceil(e.loaded/e.total) * 100 + '%');
        });

        request.open('POST', '/c/command/uploads' + F.getSession(true));
        request.send(formData);
    }
    
    resource.init();
    return (resource);
}

function initialize_resource_uploads(task) { return (new ResourceUploads(task)); }
gl_resources.jsLoaded("uploads");
