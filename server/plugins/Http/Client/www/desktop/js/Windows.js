/* This file holdes the windows manager. */

// Contains the data of the focused window
var gl_windowFocus = null;
// The next z-index of the window, used to put new and focused windows on the top of the screen.
var gl_windowNextZIndex = 1;
// The position where will be opened the next new window
var gl_windowInitialX = 150;
var gl_windowInitialY = 30;
var gl_windowInitialIncrement = 30;
var gl_windowInitialCurrent = 0;
var gl_windowInitialMax = 10;
// The old size of a window, before it has been maximized
var gl_beforeMaximiseWindow = new Object();
// The opacity of the windows that have not the focus
var gl_noFocusWindowOpacity = 0.7;
// The list of the windows saved when saveWindows was called. THey can be restored by calling restoreWindows.
var gl_savedWindows = undefined;
// A map of all the opened windows. The key is the id of the window.
var gl_windows = new Object();

// Create and open a new window.
// @page : The page displayed by the window.
// @focus : If the focus has to be put on the new window.
function openWindow(page, focus)
{
	// Create the window
	var window = new Window();
	// Initialize the window properties
	window.setTitle("page");
	window.setWidth(C.newWindowWidth);
	window.setHeight(C.newWindowHeight);
	window.setTop(gl_windowInitialY + gl_windowInitialCurrent * gl_windowInitialIncrement);
	window.setLeft(gl_windowInitialX + gl_windowInitialCurrent * gl_windowInitialIncrement);
	window.setPage(page);
	if (++gl_windowInitialCurrent >= gl_windowInitialMax)
		gl_windowInitialCurrent = 0;
	// Add the window to the map
	gl_windows[window.getId()] = window;
	if (focus != false)
		setWindowFocus(window.getId());
	// Attach the window in the document
	document.getElementById("windows").appendChild(window.root.parentNode);
	// Open the window
	window.open();
}

// Close a window
function closeWindow(id, event)
{
	if ((event != undefined && event.button == 2) || gl_windows[id] == undefined)
		return ;
	gl_windows[id].close(undefined, true);
	delete gl_windows[id];
	setNewWindowFocus();
}

// Start to move the window
function startMoveWindow(id, event)
{
	if ((event != undefined && event.button == 2) || gl_windows[id] == undefined)
		return ;
	oldMouseCoordinates = mouseCoordinates(event);
    oldWindowCoordinates = elementCoordinates(gl_windows[id].getRoot());
	gl_windowFocus = {id : id,
				   mouseX : oldMouseCoordinates.x - oldWindowCoordinates.x,
				   mouseY : oldMouseCoordinates.y - oldWindowCoordinates.y,
				   type : "move"};
	disableSelection(false);
}

// Start to resize the window
function startResizeWindow(id, event, direction)
{
	if ((event != undefined && event.button == 2) || gl_windows[id] == undefined)
		return ;
	oldMouseCoordinates = mouseCoordinates(event);
    gl_windowFocus = {id : id,
				   mouseX : oldMouseCoordinates.x,
				   mouseY : oldMouseCoordinates.y,
				   width : gl_windows[id].getWidth(),
				   height : gl_windows[id].getHeight(),
				   top : gl_windows[id].getTop(),
				   left : gl_windows[id].getLeft(),
				   type : "resize",
				   direction : direction};
	disableSelection(false);
}

// Stop to move the window
function stopWindow()
{
	disableSelection(true);
    gl_windowFocus = null;
}

// Move the window
function moveWindow(event)
{
	if (gl_windowFocus == null)
		return ;
	var id = gl_windowFocus.id;
	if (gl_windows[id] == undefined)
		return (stopWindow());
	var mouse = mouseCoordinates(event);
	if (gl_windowFocus.type == "move")
	{
		gl_windows[id].setLeft(mouse.x - gl_windowFocus.mouseX);
		gl_windows[id].setTop(mouse.y - gl_windowFocus.mouseY);
		if (mouse.x - gl_windowFocus.mouseX < C.Window.minOutX)
			gl_windows[id].setLeft(C.Window.minOutX);
		if (mouse.y - gl_windowFocus.mouseY < C.Window.minOutY)
			gl_windows[id].setTop(C.Window.minOutY);
        gl_windows[id].getPage().onResize();
	}
	else if (gl_windowFocus.type == "resize")
	{
		width = gl_windowFocus.width;
		height = gl_windowFocus.height;
		if (gl_windowFocus.direction == "nw" || gl_windowFocus.direction == "ne")
		{
			var top = gl_windowFocus.top + (mouse.y - gl_windowFocus.mouseY)
			height = gl_windowFocus.height - (mouse.y - gl_windowFocus.mouseY);
			if (top < C.Window.minOutY)
			{
				height += (top - C.Window.minOutY);
				top = C.Window.minOutY;
			}
			if (height < C.Window.minHeight)
				gl_windows[id].setTop(gl_windowFocus.top + (gl_windowFocus.height - C.Window.minHeight));
			else
				gl_windows[id].setTop(top);
		}
		if (gl_windowFocus.direction == "e" || gl_windowFocus.direction == "ne")
			width = gl_windowFocus.width + mouse.x - gl_windowFocus.mouseX;
		if (gl_windowFocus.direction == "w" || gl_windowFocus.direction == "nw" || gl_windowFocus.direction == "sw")
		{
			diff = mouse.x - gl_windowFocus.mouseX;
			left = toNumber(gl_windows[id].getLeft()) + diff;
			if (width - diff >= C.Window.minWidth && left >= C.Window.minOutX)
			{
				width -= diff;
				gl_windowFocus.width -= diff;
				gl_windows[id].setLeft(left);
				gl_windowFocus.mouseX += diff;
			}
		}
		if (gl_windowFocus.direction == "sw")
			height = gl_windowFocus.height + mouse.y - gl_windowFocus.mouseY;
		if (gl_windowFocus.direction == "s" || gl_windowFocus.direction == "sw")
			height = gl_windowFocus.height + mouse.y - gl_windowFocus.mouseY;
		if (gl_windowFocus.direction == "se")
		{
			width = gl_windowFocus.width + mouse.x - gl_windowFocus.mouseX;
			height = gl_windowFocus.height + mouse.y - gl_windowFocus.mouseY;
		}
		if (width < C.Window.minWidth)
			width = C.Window.minWidth;
		if (height < C.Window.minHeight)
			height = C.Window.minHeight;
		gl_windows[id].setWidth(width);
		gl_windows[id].setHeight(height);
        gl_windows[id].getPage().onResize();
	}
}

// Hide a window
function hideWindow(id, event)
{
	if ((event != undefined && event.button == 2) || gl_windows[id] == undefined)
		return ;
	gl_windows[id].isHidden(true);
	setNewWindowFocus();
}

// Maximize a window
function maximizeWindow(id, event)
{
	if ((event != undefined && event.button == 2) || gl_windows[id] == undefined)
		return ;
	gl_windows[id].isMaximize(!gl_windows[id].isMaximize());
}

// Hide all the other windows
function hideOtherWindow(id, event)
{
	if ((event != undefined && event.button == 2) || gl_windows[id] == undefined)
		return ;
	// Hide all the displayed windows, except the current one
	var hidden = false; // True if at least one window has been hidden
	for (var i in gl_windows)
		if (!gl_windows[i].isHidden() && i != id)
		{
			gl_windows[i].isHidden(true);
			hidden = true;
		}
	// If no window has been hidden, all the hidden windows are displayed
	if (!hidden)
		for (var i in gl_windows)
			if (gl_windows[i].isHidden())
				gl_windows[i].isHidden(false);
}

// Hide all the windows
function hideAllWindows()
{
	for (var i in gl_windows)
		if (!gl_windows[i].isHidden())
			gl_windows[i].isHidden(true);
}

// Returns true if at least one window is displayed.
function isWindowsDisplay()
{
	for (var i in gl_windows)
		if (!gl_windows[i].isHidden())
			return (true);
    return (false);
}

// Change the focused window
function setWindowFocus(id, event)
{
	if ((event != undefined && event.button == 2) || gl_windows[id] == undefined || gl_windows[id].isHidden())
		return ;
	// Blur the previous focused window
	for (var i in gl_windows)
        if (gl_windows[i].isFocus())
		{
			gl_windows[i].isFocus(false);
			break ;
		}
	// Focus on the selected window
	gl_windows[id].isFocus(true);
}

// Put the focus on the window that have the higher zIndex
function setNewWindowFocus()
{
	var zIndex = 0;
	var focus;
	for (var id in gl_windows)
        if (gl_windows[id].getZIndex() > zIndex && !gl_windows[id].isHidden())
		{
			zIndex = parseInt(gl_windows[id].getZIndex());
			focus = id;
		}
	if (focus != undefined)
		gl_windows[focus].isFocus(true);
}

// Search the id of the node of the window from one of its child.
function getWindowId(node)
{
	while (node != undefined && node.className != "window")
		node = node.parentNode;
	return (node ? node.id : null);
}

// Search the id of the window using its type.
function getWindowIdByType(type)
{
	for (var id in gl_windows)
        if (gl_windows[id].getType() == type)
			return (id);
	return (false);
}

// Search the id of the windows using their type.
function getWindowsIdByType(type)
{
	var result = new Array;
	
	for (var id in gl_windows)
        if (gl_windows[id].getType() == type)
			result.push(id);
	return (result);
}

// Saves the list of the displayed windows.
function saveWindows()
{
    gl_savedWindows = new Array();
	for (var i in gl_windows)
		if (!gl_windows[i].isHidden())
			gl_savedWindows.push(i);
}

// Restores the saved windows by displaying them.
function restoreWindows()
{
    hideAllWindows();
    if (gl_savedWindows)
        for (var i = 0; i < gl_savedWindows.length; i++)
        {
            var id = gl_savedWindows[i];
            if (gl_windows[id] && gl_windows[id].isHidden())
                gl_windows[id].isHidden(false);
        }
}

