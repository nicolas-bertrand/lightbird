function ResourceView(task, fileIndex)
{
    var resource = this;
    
    resource.init = function ()
    {
        // Members
        resource.root = $(task.content).children(".view")[0]; // The root element of the resource
        resource.file = gl_files.list[fileIndex]; // The file displayed by the resource
        resource.isFocus = true; // If the task had the focus in the last mouse down event
        resource.object; // The object that displays the file, based on its type
        
        // Capitalizes the first char of the type, to get the name of the object
        var type = resource.file.type[0].toUpperCase() + resource.file.type.slice(1);
        // Displays the file
        resource.object = new resource[type]();
        
        // Default values
        task.setResource(resource);
        gl_player.addFile(fileIndex, resource.object);
        
        // Events
        $(resource.root).mousedown(function (e) { resource.isFocus = task.isFocus(); });
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
        
        // Default values
        $(task.content).addClass("view_image");
        task.setBackground(true); // Transparent background
    
        // Displays the image
        var file = resource.file;
		var url = file.name + "?fileId=" + file.id + getSession();
        resource.root.innerHTML = "<img src=\"" + url + "\" alt=\"" + file.name + "\" class=\"file\" />";
        self.image = $(resource.root).children("img")[0];
        self.setOverflow(false);
        
        // Events
        $(resource.root).click(function (e) { self.changeBackground(e); });
        $(self.image).click(function (e) { self.changeResize(e); });
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
        if (height < C.View.minHeight)
        {
            height = C.View.minHeight;
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
        if (height < C.View.minHeight)
            height = C.View.minHeight;
        $(self.image).width(width);
        $(self.image).height(height);
    }
    
    // Changes the color of the background.
    // @param background : If the defined, the background is changed to this value.
    self.changeBackground = function (e, background)
    {
        if ((e && e.which != 1) || !resource.isFocus)
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
                task.setBackground(false, "view_background_black");
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
        if (e.which != 1 || !resource.isFocus)
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

resource.Video = function ()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.video; // The video element
      //self.video.mediaId; // The id of the media, used to communicate with the server
        self.background = "transparent"; // The background currently displayed (transparent or black)
        self.format = "webm";
        
        // Defalut values
        task.setOverflow(false);
        self.changeBackground(null, "black");
        $(task.content).addClass("view_video");
        
        // Creates the video
        var file = resource.file;
        resource.root.innerHTML = "<video autoplay />";
        self.video = $(resource.root).children("video")[0];
        self.video.mediaId = getUuid();
        // Checks the supported video formats
        if (self.video.canPlayType("video/webm; codecs=\"vp8.0, vorbis\""))
            self.format = "webm";
        else if (self.video.canPlayType("video/ogg; codecs=\"theora, vorbis\""))
            self.format = "ogg";
        else if (self.video.canPlayType("video/mp4; codecs=\"avc1.4D401E, mp4a.40.2\""))
            self.format = "mp4";
        // The browser can't play any common formats
        else
            ;
        self.video.src = "command/video." + self.format + "?fileId=" + file.id + "&mediaId=" + self.video.mediaId + getSession();
        
        // Events
        $(resource.root.parentNode).click(function (e) { self.changeBackground(e); });
        $(self.video).click(function (e) { self.playPause(e); });
    }
    
    self.onResize = function (left, top, w, h)
    {
        var naturalWidth = self.video.videoWidth ? self.video.videoWidth : resource.file.width;
        var naturalHeight = self.video.videoHeight ? self.video.videoHeight : resource.file.height;
        w += 2;
        h += 2;
        var width = w;
        var height = h;
        
        // Computes the size in order to display all the video with the natural ratio
        height = Math.floor(width * naturalHeight / naturalWidth);
        if (h < height)
        {
            width = Math.floor(h * naturalWidth / naturalHeight);
            height = h;
        }
        // Centers the video vertically
        var marginTop = 0;
        if (height < h)
            marginTop += Math.floor((h - height) / 2);
        resource.root.style.marginTop = marginTop + "px";
        // Sets the minimal size
        if (height < C.View.minHeight)
        {
            height = C.View.minHeight;
            width = Math.floor(height * naturalWidth / naturalHeight);
            task.setOverflow(true);
        }
        else
            task.setOverflow(false);
        // Resizes the video
        $(self.video).width(width);
        $(self.video).height(height);
        $(resource.root).width(width - 2);
        $(resource.root).height(height - 2);
    }
    
    self.playPause = function (e)
    {
        if (e.which != 1)
            return ;
        if (self.video.paused)
            self.video.play();
        else
            self.video.pause();
    }
    
    // Changes the color of the background.
    // @param background : If defined, the background is changed to this value.
    self.changeBackground = function (e, background)
    {
        if ((e && e.which != 1) || !resource.isFocus)
            return ;
        if (!e || e.target == resource.root.parentNode)
        {
            if (self.background == "transparent")
            {
                task.setBackground(false, "view_background_black");
                self.background = "black";
            }
            else
            {
                task.setBackground(true);
                self.background = "transparent";
            }
        }
    }

    // Player interface
    {
        self.play = function ()
        {
            self.video.play();
        }
        
        self.pause = function ()
        {
            self.video.pause();
        }
        
        self.getMedia = function ()
        {
            return (self.video);
        }
        
        self.seek = function (time)
        {
            var paused = self.video.paused;
            
            // Clears the current play back
            self.video.pause();
            self.video.src = "";
            request("GET", "command/video/stop?mediaId=" + self.video.mediaId);
            // Seeks to the new position
            self.video.mediaId = getUuid();
            self.video.src = "command/video." + self.format + "?fileId=" + resource.file.id + "&mediaId=" + self.video.mediaId + "&start=" + time + getSession();
            if (!paused)
                self.video.play();
        }
    }
 
    self.init();
    return (self);
}

    resource.init();
    return (resource);
}

function initialize_resource_view(task, fileIndex) { return new ResourceView(task, fileIndex); }
gl_resources.jsLoaded("view");
