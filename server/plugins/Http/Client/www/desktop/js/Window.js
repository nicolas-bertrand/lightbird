// The windows singleton
var gl_windows;

// Manages the windows.
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
        self.zIndex = 0; // Used to put the window that have the focus on top
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
        
        // Adds the SVG icons
        self._addIcon($(self.node.close)[0], "#ff7733", 1, "M8.946,7.407L7.421,8.932L4.479,5.99L1.525,8.943L0,7.418l2.954-2.954L0.015,1.525L1.54,0l2.938,2.939l2.929-2.928l1.524,1.525L6.004,4.464L8.946,7.407z");
        self._addIcon($(self.node.hide)[0], "#ffdd33", 4, "M0,0h10.006v3.045H0V0z");
        self._addIcon($(self.node.window)[0], "#46da57", 0, "M0,10.011V0h10v10.011H0zM7.992,2H2.015v6.021h5.977V2z");
        self._addIcon($(self.node.fullscreen)[0], "#33bbff", 0, "M9.989,6.029l-4,0.006v3.964h-2V6.038L0,6.044V3.982l3.989-0.006V0h2v3.972l4-0.006V6.029z");
        
        // Adds the window to the list
        $(self.element).appendTo(gl_windows.node.windows);
    }
    
    // Update the element of the window according to its new size.
    self.onResize = function ()
    {
        self.element.style.left = self.left - C.Window.border + "px";
        self.element.style.top = self.top - self.topHeight + "px";
        self.element.style.width = self.width + (C.Window.border + C.Desktop.taskBorder) * 2 + "px";
        self.element.style.height = self.height + C.Desktop.taskBorder * 2 + self.topHeight + C.Window.border + "px";
        self.node.middle.style.height = self.height + C.Desktop.taskBorder * 2 + C.Window.border - self.bottomHeight + "px";
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
            self.page.setContainer(gl_desktop);
            self.page.onResize();
            self.page.display();
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
        // Applies the new coordinates the their limits
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
            self.width = Math.max(width - (C.Window.border + C.Desktop.taskBorder) * 2, 0);
        }
        if (height != undefined)
        {
            if (height < C.Window.minHeight)
                height = C.Window.minHeight;
            self.height = height - (C.Desktop.taskBorder * 2 + self.topHeight + C.Window.border);
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
    
// Container interface

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
    }
    
    // Closes the window.
    self.close = function ()
    {
        $(self.element).remove();
        delete self;
    }
    
    // Notifies that the window is no longer the container of the page.
    // @parem page : The page concerned.
    self.containerChanged = function(page)
    {
        self.close();
    }
    
    // Creates an icon and adds it to the destination.
    self._addIcon = function (destination, color, top, path)
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
    
    self.init();
    return (self);
}

/* The Window object is define in this file. */
/*
function Window()
{
	var div = document.createElement("div");
	var nodes;
	var window_title_right;
	
	// Create the root node of the window
	div.innerHTML = document.getElementById("window_template").innerHTML;
	for (this.root = div.firstChild; this.root.nextSibling != undefined && this.root.className != "window"; this.root = this.root.nextSibling)
		;
	// Get some nodes in the window
	for (nodes = this.root.getElementsByTagName("div"), i = 0; i < nodes.length; ++i)
		if (nodes[i].className == "window_title_text")
			this.title = nodes[i];
		else if (nodes[i].className == "window_inner_content")
			this.content = nodes[i];
		else if (nodes[i].className == "window_title_right")
			window_title_right = nodes[i];
	this.outerContent = this.content.parentNode;
	// If the browser is Internet Explorer, style of the title of the window has to be changed
	if (/MSIE (\d+\.\d+);/.test(navigator.userAgent))
	{
		window_title_right.style.position = "relative";
		window_title_right.style.left = "-3px";
	}
	// Initialize some Window properties
	this.root.id = "window_" + getUid();
	this._focus = false;
	this.hidden = false;
	this.maximize = new Object();
	this.maximize.is = false;
	this.border = true;
}

Window.prototype.open = function ()
{
	animation(this.root, 250, animationOpacity, true, function(node)
	{
		// Make sure that the window is not focus if it has not to be
		if (!gl_windows[node.id].isFocus())
			gl_windows[node.id].isFocus(false);
	});
    this.page._display();
}

// @page : The page to close.
// @closePage : If the page in the window has to be closed too
Window.prototype.close = function (page, closePage)
{
    if (closePage)
        this.page.close();
    else
    {
        animation(this.root, 250, animationOpacity, false, function(node) {document.getElementById("windows").removeChild(node.parentNode);});
        // Let the window on the top while it fade out
        this.setZIndex(this.getZIndex() + 10);
        delete gl_windows[this.getId()];
    }
}

// @page : The page to display.
// @main : If the page have to get the focus.
Window.prototype.display = function (page, main)
{
    this.isHidden(false);
    if (main)
        setWindowFocus(this.getId());
}

// Returns true if the window is currently displayed.
Window.prototype.isDisplayed = function ()
{
    return (!this.isHidden());
}

// Hides the window.
Window.prototype.hide = function ()
{
    this.isHidden(true);
}

Window.prototype.focus = function ()
{
    setWindowFocus(this.getId());
}

Window.prototype.isFocus = function (focus)
{
	if (focus == undefined)
		return (this._focus);
	this._focus = focus;
	if (focus)
	{
        gl_windowNextZIndex += 2;
		this.setZIndex(gl_windowNextZIndex);
		this.setOpacity(1);
	}
	else
		this.setOpacity(gl_noFocusWindowOpacity);
}

Window.prototype.isHidden = function (hidden)
{
	if (hidden == undefined)
		return (this.hidden);
	this.hidden = hidden;
	if (hidden)
    {
		this.isDisplay(false);
        this.page.hide();
    }
	else
    {
		this.isDisplay(true);
        this.page._display();
    }
}

Window.prototype.isMaximize = function (maximize)
{
	if (maximize == undefined)
		return (this.maximize.is);
	this.maximize.is = maximize;
	if (maximize)
	{
		// Save the current state of the window
		this.maximize.left = this.getLeft();
		this.maximize.top = this.getTop();
		this.maximize.width = this.getWidth();
		this.maximize.height = this.getHeight();
		this.onResize = this._onResize;
		this.onResize();
	}
	else
	{
		this.setLeft(this.maximize.left);
		this.setTop(this.maximize.top);
		this.setWidth(this.maximize.width);
		this.setHeight(this.maximize.height);
		delete this.onResize;
	}
}

// Called when the browser window is resize. Allows to resize the
// maximized windows.
Window.prototype._onResize = function ()
{
	if (this.isMaximize)
	{
		this.setLeft(- C.Window.shadowLeft);
		this.setTop(- C.Window.shadowTop);
		this.setWidth(gl_browserSize.width + (C.Window.shadowLeft + C.Window.shadowRight));
		this.setHeight(gl_browserSize.height - (C.Window.shadowTop + C.Window.borderBottom));
	}
}

// Set a function that will be called when the window is resized.
Window.prototype.setOnResizeListenner = function (callback)
{
	this.onResizeListenner = callback;
}

// Call the onResizeWindowListenner and resize the content of the window
Window.prototype.onResizeWindow = function ()
{
	if (this.onResizeListenner != undefined)
		this.onResizeListenner(this.getId());
	this.outerContent.style.height = this.getHeight() + "px";
	this.outerContent.style.width = (this.getWidth() - C.Window.borderLeft - C.Window.borderRight) + "px";
	// Hide the border of the window if its size is 0
	if (this.getHeight() == 0)
		this.outerContent.style.borderWidth = "0px";
	else if (this.border)
		this.outerContent.style.borderWidth = "1px";
}

// Getters / Setters
Window.prototype.getId = function ()			{ return (this.root.id); }
Window.prototype.getRoot = function ()			{ return (this.root); }

Window.prototype.getTitle = function ()			{ return (this.title.innerHTML); }
Window.prototype.setTitle = function (title)	{ this.title.innerHTML = title; }

Window.prototype.getContent = function ()		{ return (this.content.innerHTML); }
Window.prototype.setContent = function (content) { this.content.innerHTML = content; }
Window.prototype.getContentNode = function ()	{ return (this.content); }

Window.prototype.getWidth = function ()			{ return (toNumber(this.root.style.width)); }
Window.prototype.setWidth = function (width)	{ this.width = width - C.Window.borderLeft - C.Window.borderRight; this.root.style.width = width + "px"; this.onResizeWindow(); }

Window.prototype.getHeight = function ()		{ return (toNumber(this.root.style.height)); }
Window.prototype.setHeight = function (height)	{ this.height = height; this.root.style.height = height + "px"; this.onResizeWindow(); }

Window.prototype.getTop = function ()			{ return (toNumber(this.root.style.top)); }
Window.prototype.setTop = function (top, relative) { this.top = top + C.Window.borderTop; (!relative) ? (this.root.style.top = top + "px") : (this.root.style.top = toNumber(this.root.style.top) + top + "px"); }

Window.prototype.getLeft = function ()			{ return (toNumber(this.root.style.left)); }
Window.prototype.setLeft = function (left, relative) { this.left = left + C.Window.borderLeft; (!relative) ? (this.root.style.left = left + "px") : (this.root.style.left = toNumber(this.root.style.left) + left + "px"); }

Window.prototype.getPage = function ()		    { return (this.page); }
Window.prototype.setPage = function (page)
{
    this.page = page;
    page.setContainer(this);
}

Window.prototype.isDisplay = function (display)
{
	if (display == undefined)
		return (!(this.root.style.display == "none"));
	(display) ? (this.root.style.display = "block") : (this.root.style.display = "none");
}

Window.prototype.getZIndex = function ()		{ return (this.root.style.zIndex); }
Window.prototype.setZIndex = function (zIndex)	{ this.page.setZIndex(zIndex + 1); this.root.style.zIndex = zIndex; }

Window.prototype.getOpacity = function ()		{ return (this.root.style.opacity); }
Window.prototype.setOpacity = function (opacity) { if (this.root.style.opacity != undefined) this.root.style.opacity = opacity; }

Window.prototype.getOverflow = function ()		{ return (this.outerContent.style.overflow); }
Window.prototype.setOverflow = function (overflow) { this.outerContent.style.overflow = overflow; }

Window.prototype.setBackground = function (background, border)
{
	node = this.outerContent.parentNode;
	removeClassName(node, "window_middle_middle");
	setClassName(node, "window_middle_middle_custom");
	if (background == false)
		node.style.backgroundImage = "none";
	else if (background != true)
		setClassName(node, background);
	this.border = true;
	if (border == false)
		this.border = border;
	if (!this.border)
	{
		this.outerContent.style.borderWidth = "0px";
		this.outerContent.style.top = C.Window.borderTop + "px";
		this.outerContent.style.left = C.Window.borderLeft + "px";
	}
}
*/