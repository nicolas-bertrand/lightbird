/* The Window object is define in this file. */
var gl_higherId = 0;	// Contains the higher id assigned to a window (so the id is unique).

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
	this.root.id = "window_" + gl_higherId++;
	this.focus = false;
	this.hidden = false;
	this.maximize = new Object();
	this.maximize.is = false;
	this.border = true;
}

// Management
Window.prototype.open = function ()
{
	animation(this.root, 250, animationOpacity, true, function(node)
	{
		// Make sure that the window is not focus if it has not to be
		if (!gl_windows[node.id].isFocus())
			gl_windows[node.id].isFocus(false);
	});
	//this.setTop(10, true);
	//animation(this.root, 250, animationMove, {x : 0, y : 10});
}

Window.prototype.close = function ()
{
	// Call the destructor of the window object content
	if (this.getObject().onClose)
		this.getObject().onClose();
	animation(this.root, 250, animationOpacity, false, function(node) {document.getElementById("windows").removeChild(node.parentNode);});
	//animation(this.root, 250, animationMove, {x : 0, y : 10});
	// Let the window on the top while it fade out
	this.setZIndex(this.getZIndex() + 10);
}

Window.prototype.isFocus = function (focus)
{
	if (focus == undefined)
		return (this.focus);
	this.focus = focus;
	if (focus)
	{
		this.setZIndex(gl_windowNextZIndex++);
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
		this.isDisplay(false);
	else
		this.isDisplay(true);
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
Window.prototype.setWidth = function (width)	{ this.root.style.width = width + "px"; this.onResizeWindow(); }

Window.prototype.getHeight = function ()		{ return (toNumber(this.root.style.height)); }
Window.prototype.setHeight = function (height)	{ this.root.style.height = height + "px"; this.onResizeWindow(); }

Window.prototype.getTop = function ()			{ return (toNumber(this.root.style.top)); }
Window.prototype.setTop = function (top, relative) { (!relative) ? (this.root.style.top = top + "px") : (this.root.style.top = toNumber(this.root.style.top) + top + "px"); }

Window.prototype.getLeft = function ()			{ return (toNumber(this.root.style.left)); }
Window.prototype.setLeft = function (left, relative) { (!relative) ? (this.root.style.left = left + "px") : (this.root.style.left = toNumber(this.root.style.left) + left + "px"); }

Window.prototype.getType = function ()			{ return (this.type); }
Window.prototype.setType = function (type)		{ this.type = type; }

Window.prototype.getObject = function ()		{ return (this.object); }
Window.prototype.setObject = function (object)	{ this.object = object; }

Window.prototype.getArgument = function ()		{ return (this.argument); }
Window.prototype.setArgument = function (argument) { this.argument = argument; }

Window.prototype.isDisplay = function (display)
{
	if (display == undefined)
		return (!(this.root.style.display == "none"));
	(display) ? (this.root.style.display = "block") : (this.root.style.display = "none");
}

Window.prototype.getZIndex = function ()		{ return (this.root.style.zIndex); }
Window.prototype.setZIndex = function (zIndex)	{ this.root.style.zIndex = zIndex; }

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
