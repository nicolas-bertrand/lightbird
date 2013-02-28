// Manages the windows.
var gl_windows;

function Windows()
{
    var self = this;
    gl_windows = self;
    
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.windows = $("#windows")[0];
        self.node.template = $("#window_template")[0];
        
        // Members
        self.zIndex = 1; // Used to put the window that have the focus on top
    }
    
    // Opens a new window.
    // @param page : The page to move in the window.
    self.open = function (page)
    {
        var window = new Window(page);
    }
    
    // Hide all the displayed windows.
    self.hide = function ()
    {
        var windows = $(self.node.windows).children(".window");
        for (var i = 0; i < windows.length; ++i)
            windows[i].object.page.hide();
    }
    
    // Returns a top z-index.
    self.getZIndex = function ()
    {
        return (++self.zIndex);
    }
    
    // Clears the z-index if there is no window displayed.
    self.clearZIndex = function ()
    {
        if ($(self.node.windows).children(".window.display").length == 0)
            self.zIndex = 1;
    }
    
    self.init();
    return (self);
}

// Handles a window.
// @param page : The page to move in the window.
function Window(page)
{
    var self = this;
    
    self.init = function ()
    {
        // Creates the window
        self.element = $(gl_windows.node.template).clone()[0];
        $(self.element).addClass("window");
        $(self.element).removeAttr("id");
        self.element.object = self;
        
        // Nodes
        self.node = new Object();
        self.node.top = $(self.element).children(".top")[0];
        self.node.top_left = $(self.node.top).children(".left")[0];
        self.node.top_right = $(self.node.top).children(".right")[0];
        self.node.top_top = $(self.node.top).children(".top")[0];
        self.node.close = $(self.node.top).children(".close")[0];
        self.node.hide = $(self.node.top).children(".hide")[0];
        self.node.window = $(self.node.top).children(".window")[0];
        self.node.fullscreen = $(self.node.top).children(".fullscreen")[0];
        self.node.middle = $(self.element).children(".middle")[0];
        self.node.middle_left = $(self.node.middle).children(".left")[0];
        self.node.middle_right = $(self.node.middle).children(".right")[0];
        self.node.bottom = $(self.element).children(".bottom")[0];
        self.node.bottom_left = $(self.node.bottom).children(".left")[0];
        self.node.bottom_right = $(self.node.bottom).children(".right")[0];
        
        // Focus
        $(self.element).mousedown(function (e) { self.page.display(); });
        $(self.node.top).dblclick(function (e) { self.dblclick(e); });
        // Buttons
        $(self.node.close).click(function (e) { self.buttons(e); });
        $(self.node.hide).click(function (e) { self.buttons(e); });
        $(self.node.window).click(function (e) { self.buttons(e); });
        $(self.node.fullscreen).click(function (e) { self.buttons(e); });
        // Resize
        $(self.node.top).mousedown(function (e) { self.mouseDown(e, "move"); });
        $(self.node.top_left).mousedown(function (e) { self.mouseDown(e, "nw"); });
        $(self.node.top_right).mousedown(function (e) { self.mouseDown(e, "ne"); });
        $(self.node.top_top).mousedown(function (e) { self.mouseDown(e, "n"); });
        $(self.node.middle_left).mousedown(function (e) { self.mouseDown(e, "w"); });
        $(self.node.middle_right).mousedown(function (e) { self.mouseDown(e, "e"); });
        $(self.node.bottom).mousedown(function (e) { self.mouseDown(e, "s"); });
        $(self.node.bottom_left).mousedown(function (e) { self.mouseDown(e, "sw"); });
        $(self.node.bottom_right).mousedown(function (e) { self.mouseDown(e, "se"); });
        
        // Members
        self.left = C.Window.default.left;
        self.top = C.Window.default.top;
        self.width = C.Window.default.width;
        self.height = C.Window.default.height;
        self.topHeight = C.Window.topHeight;
        self.bottomHeight = C.Window.bottomHeight;
        self.page = page;
        self.page.setContainer(self);
        
        // If the page has already been in a window, we restore its old coordinates
        if (self.page.oldWindowCoordinates)
        {
            self.left = self.page.oldWindowCoordinates.left;
            self.top = self.page.oldWindowCoordinates.top;
            self.width = self.page.oldWindowCoordinates.width;
            self.height = self.page.oldWindowCoordinates.height;
        }
        
        // Adds the SVG icons
        self.addIcon($(self.node.close)[0], "#ff7733", 1, gl_svg.Window.close);
        self.addIcon($(self.node.hide)[0], "#ffdd33", 4, gl_svg.Window.hide);
        self.addIcon($(self.node.window)[0], "#46da57", 0, gl_svg.Window.window);
        self.addIcon($(self.node.fullscreen)[0], "#33bbff", 0, gl_svg.Window.fullscreen);
        
        // Adds the window to the list
        $(self.element).appendTo(gl_windows.node.windows);
    }
    
    // Update the element of the window according to its new size.
    self.onResize = function ()
    {
        self.element.style.left = self.left - C.Window.border + "px";
        self.element.style.top = self.top - self.topHeight + "px";
        self.element.style.width = self.width + C.Window.border * 2 + "px";
        self.element.style.height = Math.max(self.height + self.topHeight + C.Window.border, 0) + "px";
        self.node.middle.style.height = Math.max(self.height + C.Window.border - self.bottomHeight, 0) + "px";
        self.page.onResize();
    }
    
    // Click on the buttons
    self.buttons = function (e)
    {
        if (e.which != 1)
            return ;
        if (e.currentTarget == self.node.close)
            self.page.close();
        else if (e.currentTarget == self.node.hide)
            self.page.hide();
        else if (e.currentTarget == self.node.window)
        {
            var page = self.page;
            page.setContainer(gl_desktop);
            page.onResize();
            page.display();
        }
    }
    
    // Starts to move or resize the window.
    self.mouseDown = function (e, direction)
    {
        if (e.which != 1 || gl_desktop.drag.isDragging() || e.target != e.currentTarget)
            return ;
        gl_desktop.drag.start(e, e.target, self, "mouseMove", undefined, undefined, { direction : direction, width : $(self.element).width(), height : $(self.element).height(), top : $(self.element).offset().top, left : $(self.element).offset().left });
        self.direction = direction;
    }
    
    // Moves or resize the window depending on self.direction.
    self.mouseMove = function (e, p)
    {
        var mouse = gl_desktop.drag.getMouse();
        var element = gl_desktop.drag.getElement();
        var y = e.pageY - (mouse.y + element.y);
        var x = e.pageX - (mouse.x + element.x);
        var top, left, width, height;
        
        // Changes the coordinates of the window
        if (p.direction == "move")
        {
            left = e.pageX - mouse.x;
            top = e.pageY - mouse.y;
        }
        if (p.direction == "n" || p.direction == "nw" || p.direction == "ne")
        {
            top = p.top + y;
            height = p.height - y;
        }
        if (p.direction == "s" || p.direction == "sw" || p.direction == "se")
            height = y + p.height;
        if (p.direction == "e" || p.direction == "ne" || p.direction == "se")
            width = x + p.width;
        if (p.direction == "w" || p.direction == "nw" || p.direction == "sw")
        {
            left = p.left + x;
            width = p.width - x;
        }
        // Applies the limits of the new coordinates
        if (left != undefined)
        {
            if (width < C.Window.minWidth)
                left = p.left + p.width - C.Window.minWidth;
            self.left = left + C.Window.border;
        }
        if (top != undefined)
        {
            if (top < C.Desktop.topHeight - C.Window.limit.top)
            {
                top = C.Desktop.topHeight - C.Window.limit.top;
                if (p.direction == "n" || p.direction == "nw" || p.direction == "ne")
                    height = p.height - (top - (element.y));
            }
            else if (top > C.Desktop.topHeight + gl_desktop.middleHeight - C.Window.limit.bottom)
            {
                top = C.Desktop.topHeight + gl_desktop.middleHeight - C.Window.limit.bottom;
                if (p.direction == "n" || p.direction == "nw" || p.direction == "ne")
                    height = p.height - (top - (element.y));
            }
            if (height < C.Window.minHeight)
                top = p.top + p.height - C.Window.minHeight;
            self.top = top + self.topHeight;
        }
        if (width != undefined)
        {
            if (width < C.Window.minWidth)
                width = C.Window.minWidth;
            self.width = Math.max(width - C.Window.border * 2, 0);
        }
        if (height != undefined)
        {
            if (height < C.Window.minHeight)
                height = C.Window.minHeight;
            self.height = height - (self.topHeight + C.Window.border);
        }
        self.onResize();
    }
    
    // Puts the focus on the window.
    self.focus = function()
    {
        // Puts the window on top
        self.element.style.zIndex = gl_windows.getZIndex();
        // The elements of the page are above the window
        self.page.setZIndex(gl_windows.getZIndex());
    }
    
    // Hides all the other pages displayed or lock the window.
    self.dblclick = function (e)
    {
        if (e.target != self.node.top)
            return ;
        $(gl_desktop.node.pages).children(".display").each(function ()
        {
            if (this != self.page.content)
                this.object.hide();
        });
    }
    
    // Creates an icon and adds it to the destination.
    self.addIcon = function (destination, color, top, path)
    {
        var paper = Raphael(destination, 30, 35);
        var rect = paper.rect(0, 0, 30, 35);
        rect.attr("fill", "white");
        rect.attr("stroke", "none");
        rect.attr("opacity", "0.5");
        rect.hide();
        var line = paper.rect(0, 0, 30, 5);
        line.attr("fill", color);
        line.attr("stroke", "none");
        line.hide();
        var icon = paper.path(path);
        icon.translate(10, 13 + top);
        icon.attr("fill", "black");
        icon.attr("stroke", "none");
        icon.attr("opacity", "0.6");
        icon.glow({ width : 10, color : "white", opacity : 0.2 });
        $(destination).mouseenter(function ()
        {
            line.show();
            rect.show();
        });
        $(destination).mouseleave(function ()
        {
            line.hide();
            rect.hide();
        });
    }
    
// Container API
// These methods allow the pages to interact with their container.
    {
        // Displays the window.
        self.display = function ()
        {
            self.onResize();
            self.focus();
            $(self.element).addClass("display");
            $(self.page.icon).addClass("window");
        }

        // Hides the window.
        self.hide = function ()
        {
            $(self.element).removeClass("display");
            gl_windows.clearZIndex();
        }
        
        // Closes the window.
        self.close = function ()
        {
            // Saves the coordinates of the window for the next time it is opened, if necessary
            self.page.oldWindowCoordinates = { left : self.left, top : self.top, width : self.width, height : self.height };
            // Clears the window
            self.page = undefined;
            self.element.object = undefined;
            $(self.element).remove();
            for (var key in self)
                self[key] = undefined;
            gl_windows.clearZIndex();
        }
        
        // Notifies that the window is no longer the container of the page.
        // @parem page : The page concerned.
        self.containerChanged = function(page)
        {
            self.close();
        }
    }
    
    self.init();
    return (self);
}
