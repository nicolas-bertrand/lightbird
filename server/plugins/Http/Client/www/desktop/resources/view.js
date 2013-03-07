function ResourceView(task, playlistInterface, fileIndex)
{
    var resource = this;
    
    resource.init = function ()
    {
        // Members
        resource.file = gl_files.list[fileIndex]; // The file displayed by the resource
        resource.fileIndex = fileIndex; // The index of the file displayed
        resource.root = $(task.content).children("." + resource.file.type)[0]; // The root element of the resource
        resource.isFocus = true; // If the task had the focus in the last mouse down event
        resource.object; // The object that displays the file, based on its type. Implements the file and the player interfaces.
        resource.playlistInterface = playlistInterface; // The playlist that manages this file (may be undefined)
        
        // Capitalizes the first char of the type, to get the name of the object
        var type = resource.file.type[0].toUpperCase() + resource.file.type.slice(1);
        // Displays the file
        resource.object = new resource[type]();
        
        // Default values
        task.setResource(resource);
        resource.playlistInterface.readyToPlay(resource.object);
        $(task.content).addClass("view_" + resource.file.type);
        
        // Events
        if (C.View.focusBeforeAction)
            $(task.content).mousedown(function (e) { resource.isFocus = task.isFocus(); });
        $(task.content).mouseenter(function (e) { gl_player.setFileName(resource.fileIndex); });
        $(task.content).mouseleave(function (e) { gl_player.setFileName(); });
    }
    
    // The task have been resized.
    resource.onResize = function (left, top, width, height)
    {
        resource.object.onResize(left, top, width, height);
    }
    
    // Closes the resource.
    resource.close = function ()
    {
        gl_player.closeFile(resource.object);
        if (resource.object.close)
            resource.object.close();
        for (var key in resource)
            resource[key] = undefined;
    }

    // Replaces < and > by their HTML equivalent.
    resource.escape = function (text)
    {
        return (text.replace(/</g, "&lt;").replace(/>/g, "&gt;"));
    }
    
// Displays a document.
resource.Document = function ()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.pre = $(resource.root).children("pre"); // The element that stores the text of the document
        
        request("GET", resource.file.name + "?fileId=" + resource.file.id, function (HttpRequest)
        {
            self.pre.html(resource.escape(HttpRequest.responseText));
        });
    }
    
    self.close = function ()
    {
    }
    
    self.onResize = function (left, top, width, height)
    {
    }
    
// File interface
    {
        self.getFileIndex = function ()
        {
            return (resource.fileIndex);
        }
        
        self.setPlaylist = function (playlistInterface)
        {
            resource.playlistInterface = playlistInterface;
        }
    }
 
    self.init();
    return (self);
}

// Displays the image using several resize methods and backgrounds.
resource.Image = function ()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.background = "transparent"; // The background currently displayed (transparent, black or default)
        self.container = $(resource.root).children(".container")[0]; // The div that contains the image element
        self.image = $(self.container).children("img")[0]; // The img element
        self.resize = self.keepRatio; // The function used to resize the image when onResize is called
        
        // Default values
        task.setBackground(false); // No background
        task.setBorder(false); // No borders
        task.setOverflow(false); // No overflow
    
        // Displays the image
        self.image.src = "/c/" + resource.file.name + "?fileId=" + resource.file.id + getSession();
        self.image.alt = resource.file.name;
        
        // Events
        $(task.content).click(function (e) { self.changeBackground(e); });
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
        // Sets the minimal size
        if (height < C.View.minHeight)
        {
            height = C.View.minHeight;
            width = Math.floor(height * naturalWidth / naturalHeight);
            task.setOverflow(true);
        }
        else
            task.setOverflow(false);
        // Centers the image
        var marginLeft = (width < w ? Math.floor((w - width) / 2) : 0);
        var marginTop = (height < h ? Math.floor((h - height) / 2) : 0);
        self.center(marginLeft, marginTop);
        // Resizes the image
        $(self.container).width(width);
        $(self.container).height(height);
        if (self.background == "transparent")
        {
            $(resource.root).width(width - 2 < w - 2 ? width - 2 : w - 2); // (w - 2) and (h - 2) takes the minimal size into account
            $(resource.root).height(height - 2 < h - 2 ? height - 2 : h - 2);
        }
        else
        {
            $(resource.root).width(w - 2);
            $(resource.root).height(h - 2);
        }
    }
    
    // Displays the image with its natural size.
    self.naturalSize = function (left, top, width, height)
    {
        var naturalWidth = self.image.naturalWidth ? self.image.naturalWidth : resource.file.width;
        var naturalHeight = self.image.naturalHeight ? self.image.naturalHeight : resource.file.height;
        
        // Centers the image
        var marginLeft = (naturalWidth < width ? Math.floor((width - naturalWidth) / 2) : 0);
        var marginTop = (naturalHeight < height ? Math.floor((height - naturalHeight) / 2) : 0);
        self.center(marginLeft, marginTop);
        // Resizes the containers
        $(self.container).width(width);
        $(self.container).height(height);
        $(resource.root).width(width - 2);
        $(resource.root).height(height - 2);
    }
    
    // Resizes the image so that it fills all the space.
    self.scale = function (left, top, width, height)
    {
        $(resource.root).width(width - 2);
        $(resource.root).height(height - 2);
        if (height < C.View.minHeight)
        {
            task.setOverflow(true);
            height = C.View.minHeight;
        }
        else
            task.setOverflow(false);
        $(self.container).width(width);
        $(self.container).height(height);
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
        if (!e || e.target != self.image)
        {
            if (self.background == "transparent")
            {
                task.setBackground(true, "view_background_black");
                self.background = "black";
            }
            else if (self.background == "black")
            {
                task.setBackground(true);
                self.background = "default";
            }
            else
            {
                task.setBackground(false);
                self.background = "transparent";
            }
            self.center(0, 0, true);
            self.onResize(task.left, task.top, task.width, task.height);
        }
    }
    
    // Changes the resize function of the image.
    self.changeResize = function (e)
    {
        if (e.which != 1 || !resource.isFocus)
            return ;
        // Default values
        $(resource.root).removeClass("scroll");
        self.image.style.width = "100%";
        self.image.style.height = "100%";
        // Go to scale from keepRatio, or from naturalSize if the image is smaller than the task
        if (self.resize == self.keepRatio || (self.resize == self.naturalSize && self.image.naturalWidth < task.width && self.image.naturalHeight < task.height))
            self.resize = self.scale;
        // Go to naturalSize only if the image doesn't fit the task size
        else if (self.resize == self.scale && (self.image.naturalWidth > task.width || self.image.naturalHeight > task.height))
        {
            self.resize = self.naturalSize;
            self.image.style.width = "auto";
            self.image.style.height = "auto";
            $(resource.root).addClass("scroll");
        }
        else
            self.resize = self.keepRatio;
        self.center(0, 0, true);
        self.onResize(task.left, task.top, task.width, task.height);
    }
    
    // Centers the image horizontally and vertically based on the resize method and the background.
    // @param reset : Resets the centering.
    self.center = function (marginLeft, marginTop, reset)
    {
        if (reset)
        {
            self.image.style.marginLeft = "0px";
            self.image.style.marginTop = "0px";
            self.container.style.marginLeft = "0px";
            self.container.style.top = "-1px";
            resource.root.style.marginLeft = "0px";
            resource.root.style.marginTop = "0px";
        }
        else if (self.resize == self.keepRatio)
        {
            if (self.background == "transparent")
            {
                resource.root.style.marginLeft = marginLeft + "px";
                resource.root.style.marginTop = marginTop + "px";
            }
            else
            {
                self.container.style.marginLeft = marginLeft + "px";
                self.container.style.top = (marginTop - 1) + "px";
            }
        }
        else
        {
            self.image.style.marginLeft = marginLeft + "px";
            self.image.style.marginTop = marginTop + "px";
        }
    }
    
// File interface
    {
        self.getFileIndex = function ()
        {
            return (resource.fileIndex);
        }
        
        self.setPlaylist = function (playlistInterface)
        {
            resource.playlistInterface = playlistInterface;
        }
    }

    self.init();
    return (self);
}

// Displays the other files types.
resource.Other = function ()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.pre = $(resource.root).children("pre"); // The element that stores the text of the file
        
        request("GET", resource.file.name + "?fileId=" + resource.file.id, function (HttpRequest)
        {
            self.pre.html(resource.escape(HttpRequest.responseText));
        });
    }
    
    self.close = function ()
    {
    }
    
    self.onResize = function (left, top, width, height)
    {
    }
    
// File interface
    {
        self.getFileIndex = function ()
        {
            return (resource.fileIndex);
        }
        
        self.setPlaylist = function (playlistInterface)
        {
            resource.playlistInterface = playlistInterface;
        }
    }
 
    self.init();
    return (self);
}

// Plays the video.
resource.Video = function ()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.container = $(resource.root).children(".container")[0]; // The div that contains the video element
        self.video = $(self.container).children("video")[0]; // The video element
        self.video.mediaId; // The id of the media, used to communicate with the server
        self.video.timeOffset = 0; // The server side seek
        self.background = "black"; // The background currently displayed (black or transparent)
        self.format = "webm";
        
        // Defalut values
        task.setBackground(true, "view_background_black");
        task.setBorder(false);
        task.setOverflow(false);
        
        // Creates the video
        var file = resource.file;
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
        self.video.src = "/c/command/video." + self.format + "?fileId=" + file.id + "&mediaId=" + self.video.mediaId + getSession();
        
        // Events
        $(task.content).click(function (e) { self.changeBackground(e); });
        $(self.video).click(function (e) { self.playPause(e); });
    }
    
    // Closes the video player.
    self.close = function ()
    {
        self.video.pause();
        self.video.src = "";
        for (var key in self)
            self[key] = undefined;
    }
    
    self.onResize = function (left, top, w, h)
    {
        var naturalWidth = self.video.videoWidth ? self.video.videoWidth : resource.file.width;
        var naturalHeight = self.video.videoHeight ? self.video.videoHeight : resource.file.height;
        var width = w;
        var height = h;
        
        // Computes the size in order to display all the video with the natural ratio
        height = Math.floor(width * naturalHeight / naturalWidth);
        if (h < height)
        {
            width = Math.floor(h * naturalWidth / naturalHeight);
            height = h;
        }
        // Sets the minimal size
        if (height < C.View.minHeight)
        {
            height = C.View.minHeight;
            width = Math.floor(height * naturalWidth / naturalHeight);
            task.setOverflow(true);
        }
        else
            task.setOverflow(false);
        // Centers the video 
        var marginLeft = (width < w ? Math.floor((w - width) / 2) : 0);
        var marginTop = (height < h ? Math.floor((h - height) / 2) : 0);
        // Resizes the video
        $(self.container).width(width);
        $(self.container).height(height);
        if (self.background == "transparent")
        {
            resource.root.style.marginLeft = marginLeft + "px";
            resource.root.style.marginTop = marginTop + "px";
            $(resource.root).width(width - 2 < w - 2 ? width - 2 : w - 2); // (w - 2) and (h - 2) takes the minimal size into account
            $(resource.root).height(height - 2 < h - 2 ? height - 2 : h - 2);
        }
        else
        {
            self.container.style.marginLeft = marginLeft + "px";
            self.container.style.top = (marginTop - 1) + "px";
            $(resource.root).width(w - 2);
            $(resource.root).height(h - 2);
        }
    }
    
    // Play / pause the video.
    self.playPause = function (e)
    {
        if (e.which != 1)
            return ;
        if (self.video.paused)
        {
            self.video.play();
            gl_player.resumeFile(self);
        }
        else
            self.video.pause();
    }
    
    // Changes the background color.
    self.changeBackground = function (e)
    {
        if (e.which != 1 || !resource.isFocus || e.target == self.video)
            return ;
        if (self.background == "transparent")
        {
            task.setBackground(true, "view_background_black");
            self.background = "black";
        }
        else
        {
            task.setBackground(false);
            self.background = "transparent";
        }
        resource.root.style.marginLeft = "0px";
        resource.root.style.marginTop = "0px";
        self.container.style.marginLeft = "0px";
        self.container.style.top = "-1px";
        self.onResize(task.left, task.top, task.width, task.height);
    }

// File and Player interfaces
    {
        self.getFileIndex = function ()
        {
            return (resource.fileIndex);
        }
        
        self.setPlaylist = function (playlistInterface)
        {
            resource.playlistInterface = playlistInterface;
        }
        
        self.play = function ()
        {
            self.video.play();
        }
        
        self.pause = function ()
        {
            self.video.pause();
        }
        
        self.seek = function (time)
        {
            var paused = self.video.paused;
            
            // Clears the current play back
            self.video.pause();
            self.video.src = "";
            request("GET", "command/video/stop?mediaId=" + self.video.mediaId);
            // Seeks to the new position
            self.video.timeOffset = time;
            self.video.mediaId = getUuid();
            self.video.src = "/c/command/video." + self.format + "?fileId=" + resource.file.id + "&mediaId=" + self.video.mediaId + "&start=" + time + getSession();
            if (!paused)
                self.video.play();
        }
        
        self.getMedia = function ()
        {
            return (self.video);
        }
    }
 
    self.init();
    return (self);
}

    resource.init();
    return (resource);
}

function initialize_resource_view(task, parameters) { return new ResourceView(task, parameters.playlistInterface, parameters.fileIndex); }
gl_resources.jsLoaded("view");
