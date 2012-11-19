function ResourceView(task, fileIndex)
{
    var resource = this;
    
    resource.init = function ()
    {
        // Members
        resource.root = $(task.content).children(".view")[0]; // The root element of the resource
        resource.file = gl_files.list[fileIndex]; // The file displayed by the resource
        resource.object; // The object that displays the file, based on its type
        
        // Capitalizes the first char of the type, to get the name of the object
        var type = resource.file.type[0].toUpperCase() + resource.file.type.slice(1);
        // Displays the file
        resource.object = new resource[type]();
        
        task.setResource(resource);
        // Transparent background
        task.setBackground(true);
    }
    
    // The task have been resized.
    resource.onResize = function (left, top, width, height)
    {
        resource.object.onResize(left, top, width, height);
    }
    
    // Closes the resource.
    resource.close = function ()
    {
        for (var key in resource)
            resource[key] = undefined;
    }

resource.Image = function ()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.background = "transparent"; // The background currently displayed (transparent, black or default)
        self.oldBackground; // Used by changeResize to get back to the previous background
        self.image; // The img element
        self.resize = self.keepRatio; // The function used to resize the image when onResize is called
        self.horizontalAlign = false; // False if the css class horizontal align is defined
        self.isFocus = true; // If the task had the focus in the last mouse down event
    
        // Displays the image
        var file = resource.file;
		var url = file.name + "?id=" + file.id + "&token=" + getToken(file.name);
        resource.root.innerHTML = "<img src=" + url + " alt=" + file.name + " />";
        self.image = $(resource.root).children("img")[0];
        self.setOverflow(false);
        
        // Events
        $(resource.root).click(function (e) { self.changeBackground(e); });
        $(self.image).click(function (e) { self.changeResize(e); });
        $(resource.root).mousedown(function (e) { self.isFocus = task.isFocus(); });
    }
    
    // Resizes the image to fit in the task dimensions.
    self.onResize = function (left, top, width, height)
    {
        self.resize(left, top, width, height);
    }
    
    // Resizes the image while keeping its natural ratio.
    self.keepRatio = function (left, top, w, h)
    {
        var naturalWidth = self.image.naturalWidth ? self.image.naturalWidth : resource.file.width;
        var naturalHeight = self.image.naturalHeight ? self.image.naturalHeight : resource.file.height;
        var width = w;
        var height = h;
        
        // Computes the size in order to display all the image with the natural ratio
        if (naturalWidth > width)
        {
            height = Math.floor(width * naturalHeight / naturalWidth);
            if (h < height)
            {
                width = Math.floor(h * naturalWidth / naturalHeight);
                height = h;
            }
        }
        else if (naturalHeight > height)
            width = height * naturalWidth / naturalHeight;
        // The image can be displayed at full size
        if (width >= naturalWidth && height >= naturalHeight)
        {
            width = naturalWidth;
            height = naturalHeight;
        }
        // Centers the image vertically
        var marginTop = 0;
        if (height < h)
            marginTop += Math.floor((h - height) / 2);
        self.image.style.marginTop = marginTop + "px";
        // Sets the minimal size
        if (height < C.View.imageMinHeight)
        {
            height = C.View.imageMinHeight;
            width = Math.floor(height * naturalWidth / naturalHeight);
            self.setOverflow(true);
        }
        else
            self.setOverflow(false);
        // Adjust the horizontal alignment
        if (height == h || width != w)
        {
            if (!self.horizontalAlign)
            {
                self.horizontalAlign = true;
                $(self.image).addClass("horizontal_align");
            }
        }
        else if (self.horizontalAlign)
        {
            self.horizontalAlign = false;
            $(self.image).removeClass("horizontal_align");
        }
        // Resizes the image
        $(self.image).width(width);
        $(self.image).height(height);
    }
    
    // Displays the image with its natural size.
    self.naturalSize = function (left, top, width, height)
    {
        var naturalWidth = self.image.naturalWidth ? self.image.naturalWidth : resource.file.width;
        var naturalHeight = self.image.naturalHeight ? self.image.naturalHeight : resource.file.height;
        
        // Centers the image vertically
        var marginTop = 0;
        if (naturalHeight < height)
            marginTop += Math.floor((height - naturalHeight) / 2);
        self.image.style.marginTop = marginTop + "px";
        // Adjust the horizontal alignment
        if (height == naturalHeight || width > naturalWidth)
        {
            if (!self.horizontalAlign)
            {
                self.horizontalAlign = true;
                $(self.image).addClass("horizontal_align");
            }
        }
        else if (self.horizontalAlign)
        {
            self.horizontalAlign = false;
            $(self.image).removeClass("horizontal_align");
        }
    }
    
    // Resizes the image so that it fills all the space.
    self.scale = function (left, top, width, height)
    {
        if (height < C.View.imageMinHeight)
            height = C.View.imageMinHeight;
        $(self.image).width(width);
        $(self.image).height(height);
    }
    
    // Changes the color of the background.
    // @param background : If the defined, the background is changed to this value.
    self.changeBackground = function (e, background)
    {
        if ((e && e.which != 1) || !self.isFocus)
            return ;
        if (background)
        {
            if (background == "transparent")
                self.background = "default";
            else if (background == "black")
                self.background = "transparent";
            else if (background == "default")
                self.background = "black";
        }
        if (!e || e.target == resource.root)
        {
            if (self.background == "transparent")
            {
                task.setBackground(false, "image_background_black");
                self.background = "black";
                self.setOverflow(false);
            }
            else if (self.background == "black")
            {
                task.setBackground();
                self.background = "default";
                self.setOverflow(true);
            }
            else
            {
                task.setBackground(true);
                self.background = "transparent";
                self.setOverflow(true);
            }
            self.onResize(task.left, task.top, task.width, task.height);
        }
        // If the background changed in naturalSize, we save it
        if (self.resize == self.naturalSize)
            self.oldBackground = self.background;
    }
    
    // Changes the resize function of the image.
    self.changeResize = function (e)
    {
        if (e.which != 1 || !self.isFocus)
            return ;
        $(resource.root).removeClass("scroll");
        // Go to scale from keepRatio, or from naturalSize if the image is smaller than the task
        if (self.resize == self.keepRatio || (self.resize == self.naturalSize && self.image.naturalWidth < task.width && self.image.naturalHeight < task.height))
        {
            self.resize = self.scale;
            self.oldBackground = self.background;
            self.changeBackground(undefined, "default");
        }
        // Go to naturalSize only if the image doesn't fit the task size
        else if (self.resize == self.scale && (self.image.naturalWidth > task.width || self.image.naturalHeight > task.height))
        {
            self.resize = self.naturalSize;
            self.image.style.width = "auto";
            self.image.style.height = "auto";
            $(resource.root).addClass("scroll");
            if (self.oldBackground)
                self.changeBackground(undefined, self.oldBackground);
        }
        else
        {
            self.resize = self.keepRatio;
            // Restores the background
            if (self.oldBackground)
            {
                self.changeBackground(undefined, self.oldBackground);
                delete self.oldBackground;
            }
        }
        self.setOverflow(true);
        self.image.style.marginTop = "0px";
        self.onResize(task.left, task.top, task.width, task.height);
    }
    
    // Sets the overflow of the task.
    self.setOverflow = function (overflow)
    {
        // The overflow is only disabled in keepRatio mode
        if (overflow || self.resize == self.keepRatio)
            task.setOverflow(overflow);
    }

    self.init();
    return (self);
}

    resource.init();
    return (resource);
}

function initialize_resource_view(task, fileIndex) { return new ResourceView(task, fileIndex); }
gl_resources.jsLoaded("view");
