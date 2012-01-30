/* This file manages the desktop. */

// The desktop singleton
var gl_desktop;
// Allows to get a unique id across the current session.
// Increment the value each time an id is needed.
var gl_uid = 0;

function Desktop()
{
    // Stores the nodes of the desktop
    this.node = new Object();
    this.node.desktop = document.getElementById("desktop");
    this.node.top = getElementsByClassName("top", this.node.desktop, true);
    this.node.menu = getElementsByClassName("menu", this.node.top, true);
    this.node.tasks_list = getElementsByClassName("tasks_list", this.node.top, true);
    this.node.middle = getElementsByClassName("middle", this.node.top, true);
    this.node.player_document = getElementsByClassName("player_document", this.node.middle, true);
    this.node.pages = getElementsByClassName("pages", this.node.middle, true);
    this.node.tasks = getElementsByClassName("tasks", this.node.middle, true);
    this.node.player_media = getElementsByClassName("player_media", this.node.desktop, true);
    // Set the default values
    this.node.desktop.style.minHeight = C.Desktop.minHeight + "px";
    this.node.desktop.style.minWidth = C.Desktop.minWidth + "px";
    this.node.player_document.style.height = C.Desktop.player_document_height + "px";
    this.node.player_media.style.height = C.Desktop.player_media_height + "px";
    this.node.menu.style.width = T.Menu.width + "px";
    this.node.tasks_list.style.width = C.Desktop.tasks_list_width + "px";
    this.middle_margin_left = T.Menu.width + C.Desktop.tasks_list_width;
    this.node.middle.style.marginLeft = this.middle_margin_left + "px";
    this.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + "px";
    this.content = new Object();
    // Events
    var mouseWheelEvent = (/Firefox/i.test(navigator.userAgent)) ? "DOMMouseScroll" : "mousewheel";
    addEvent(this.node.tasks_list, mouseWheelEvent, function(event) { gl_desktop.mouseWheel(event); });
    addEvent(this.node.tasks_list, "mouseover", function(event) { gl_desktop.mouseOverTasksList(event); });
    addEvent(this.node.tasks_list, "mouseout", function(event) { gl_desktop.mouseOutTasksList(event); });
    // The task or page being dragged
    this.drag = undefined;
}

// Open a new task in a new page
// @param resource : The name of the resource that will be loaded in the task
// @param event : The event that triggered the call
Desktop.prototype.open = function (resource, event)
{
	if (getButton(event) != 0)
		return ;
    // Load the task from the resources
	gl_resources.load(resource, function(content)
    {
        // Create the page and the task
        var page = new Page();
        var task = new Task(resource, content);
        page.addTask(task);
        page.display(true);
        gl_resources.callJs(resource, task);
        // Scroll to the bottom of the tasks list
        gl_desktop.node.tasks_list.scrollTop = gl_desktop.node.tasks_list.scrollHeight - gl_desktop.top_height - C.Desktop.new_task_height;
    });
}

// Resize the desktop
Desktop.prototype.onResize = function ()
{
    var width = (gl_browserSize.width > C.Desktop.minWidth) ? gl_browserSize.width : C.Desktop.minWidth;
    var height = (gl_browserSize.height > C.Desktop.minHeight) ? gl_browserSize.height : C.Desktop.minHeight;
    this.top_height = height - C.Desktop.player_media_height;
    this.node.top.style.height = this.top_height + "px";
    this.content.left = this.middle_margin_left;
    this.content.top = C.Desktop.player_document_height;
    this.content.width = width - this.middle_margin_left;
    this.content.height = this.top_height - C.Desktop.player_document_height;
    if (this.content.page)
        this.content.page.onResize();
}

// Called each time the mouse move
// @param event : The event that triggered the call
Desktop.prototype.mouseMove = function (event)
{
	this.mouse = mouseCoordinates(event);
    // If a task is being dragged
    if (this.drag)
    {
        var y = this.mouse.y - this.drag.mouse.y;
        // Top limit
        if (y < 0)
            this.drag.icon.style.top = "0px";
        // Bottom limit
        else if (y > this.top_height - C.Desktop.task_height)
            this.drag.icon.style.top = (this.top_height - C.Desktop.task_height) + "px";
        // The task has a little resistance before starting the drag
        else if (y >= this.drag.position.y - this.node.tasks_list.scrollTop - C.Desktop.task_magnetic_drag_y && y <= this.drag.position.y - this.node.tasks_list.scrollTop + C.Desktop.task_magnetic_drag_y && this.drag.resistance)
            this.drag.icon.style.top = this.drag.position.y - this.node.tasks_list.scrollTop + "px";
        // Start the dragging
        else if (this.drag.resistance)
            y = this.drag.startDrag(y);
        // Move the dragged task vertically
        else
            this.drag.icon.style.top = y + "px";
        // Move the dragged task horizontally if the mouse exceeds the magnetic limit
        if (this.mouse.x > this.middle_margin_left + C.Desktop.task_magnetic_drag_x)
            this.drag.icon.style.left = (this.mouse.x - this.drag.mouse.x) + "px";
        // Otherwise update the tasks list
        else if (!this.drag.resistance)
        {
            this.drag.icon.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
            this._updateTasksList(this.mouse.y);
        }
    }
    // Otherwise a window may be dragged
    else
        moveWindow(event);
    // The top scrolling area of the tasks list
    if (this.mouse.x < this.middle_margin_left && this.mouse.x > T.Menu.width && this.mouse.y <= C.Desktop.tasks_list_scroll)
        this._scrollTasksList(1 - (this.mouse.y / C.Desktop.tasks_list_scroll));
    // The bottom scrolling area of the tasks list
    else if (this.mouse.x < this.middle_margin_left && this.mouse.x > T.Menu.width && this.mouse.y > this.top_height - C.Desktop.tasks_list_scroll && this.mouse.y <= this.top_height)
        this._scrollTasksList((this.top_height - C.Desktop.tasks_list_scroll - this.mouse.y) / C.Desktop.tasks_list_scroll);
    // Ensures that the scrolling is stopped outside of the scrolling areas
    else
        this._stopScroll();
}

// Called when the mouse is up
// @param event : The event that triggered the call
Desktop.prototype.mouseUp = function (event)
{
    // If a task is being dragged, stop it
    if (this.drag)
    {
        // Move the task to its new position
        this._moveTask();
        this.drag = undefined;
        disableSelection(true);
    }
    // Otherwise stop the window
    else
        stopWindow(event);
}

// Called each time the mouse wheel is used on the tasks list
Desktop.prototype.mouseWheel = function (event)
{
    // Scroll the tasks list depending on the mouse wheel delta
    this.node.tasks_list.scrollTop -= (event.detail ? event.detail * (-28) : event.wheelDelta) / C.Desktop.wheelSpeed;
    this._updateTasksList(this.mouse.y);
}

// Doesn't display the page under the cursor immediatly
Desktop.prototype.mouseOverTasksList = function (event)
{
    if (!getEventRelatedTarget(event, "tasks_list", 3))
        this.content.timeout = setTimeout("gl_desktop.mouseOverTasksListTimeout()", C.Desktop.pagePreviewDelay);
}
Desktop.prototype.mouseOverTasksListTimeout = function ()
{
    delete this.content.timeout;
    if (this.content.currentPage)
        this.content.currentPage.display();
    this.content.currentPage = undefined;
    this.content.preview = true;
}

// Display the main page
Desktop.prototype.mouseOutTasksList = function (event)
{
    if (!this.drag && !getEventRelatedTarget(event, "tasks_list", 3))
    {
        if (gl_desktop.content.main)
            gl_desktop.content.main.display();
        if (this.content.timeout)
        {
            clearTimeout(this.content.timeout);
            delete this.content.timeout;
            this.content.currentPage = undefined;
        }
        this.content.preview = false;
    }
}

// Hides all the pages of the desktop 
Desktop.prototype.hide = function ()
{
    if (this.content.page)
    {
        this.content.page.content.style.display = "none";
        for (var task = this.content.page.icon.firstChild; task; task = task.nextSibling)
            if (getClassName(task, "task"))
                task.object.content.style.display = "none";
        this.content.page = undefined;
    }
}

// Display a preview of a new page
// @param task : The task of the future new page
Desktop.prototype.previewNewPage = function (task)
{
    // Hide the page currently displayed
    this.hide();
    // Display the task
    task.content.style.left = this.content.left + "px";
    task.content.style.top = this.content.top + "px";
    task.content.style.width = this.content.width + "px";
    task.content.style.height = this.content.height + "px";
    task.content.style.display = "block";
}

/********************************/
/********** Tasks list **********/
/********************************/
// Place a ghost task under the cursor to show the place where the dragged task will be moved
// @param y : The y position of the mouse
Desktop.prototype._updateTasksList = function (y)
{
    if (!this.drag)
        return ;
    // Change the reference y depending if the task has been dragged up or down
    y = y - this.drag.mouse.y + this.node.tasks_list.scrollTop;
    if (y > this.drag.position.y)
        y += C.Desktop.task_height;
    this._removeGhostTask();
    // Get the position of the task under the cursor
    var data = this._getCursorPosition(y);
    var page = data.page;
    var task = data.task;
    // If we want to create a page, the ghost is not required
    if (this.taskCache.createPage)
    {
        this.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + C.Desktop.task_height + "px";
        this.previewNewPage(this.drag);
        return ;
    }
    // Create a ghost task in the current page
    this.ghost = document.createElement("div");
    this.ghost.className = "ghost";
    // The current page is above the original page
    if (data.number_page <= this.drag.place.page)
    {
        if (!task.nextSibling && y > data.position - C.Desktop.page_padding - C.Desktop.task_height / 2)
            page.appendChild(this.ghost);
        else if (task.nextSibling && y > data.position - C.Desktop.task_height / 2)
            page.insertBefore(this.ghost, task.nextSibling);
        else
            page.insertBefore(this.ghost, task);
    }
    // The current page is below the original page
    else
    {
        if (!task.nextSibling && y < data.position - C.Desktop.page_padding - C.Desktop.task_height / 2)
            page.insertBefore(this.ghost, task);
        else if (task.nextSibling && y < data.position - C.Desktop.task_height / 2)
            page.insertBefore(this.ghost, task);
        else if (task.nextSibling)
            page.insertBefore(this.ghost, task.nextSibling);
        else
            page.appendChild(this.ghost);
    }
    // Display the page under which the cursor is
    if (!this.content.page || page.object.id != this.content.page.id)
        page.object.preview(this.drag);
    this.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + "px";
}

// Get the position of the task under the cursor and some other data
// @param y : The y position of the mouse
Desktop.prototype._getCursorPosition = function (y)
{
    var position = 0;
    var number_page = 0;
    var number_task = 0;
    var page, task;
    
    // If the position have already been calculated for this interval, we return the cache
    if (this.taskCache && y < this.taskCache.position && y > this.taskCache.limit && !this.taskCache.createPage)
        return (this.taskCache);
    // Run through the tasks in order to find the position of the task under the cursor
    for (page = this.node.tasks_list.firstChild; page && getClassName(page, "page") && (position < y || !number_page); page = page.nextSibling, number_page++)
    {
        position += C.Desktop.page_padding;
        for (task = page.firstChild, number_task = 0; task && (position < y || !number_task); task = task.nextSibling, number_task++)
            position += C.Desktop.task_height;
        if (!task)
            position += C.Desktop.page_padding;
    }
    // Get the correct nodes
    page = page.previousSibling;
    (task) ? task = task.previousSibling : task = page.lastChild;
    // Calculate the upper limit
    var limit = position - C.Desktop.task_height;
    if (!task.previousSibling && !task.nextSibling)
        limit -= C.Desktop.page_padding * 2;
    else if (!task.previousSibling || !task.nextSibling)
        limit -= C.Desktop.page_padding;
    // Save the data in a cache, so that we dont have to calculate it each time the cursor moves
    this.taskCache = {position : position, limit : limit, number_page : number_page, number_task : number_task, page : page, task : task, createPage : (y > position)}
    return (this.taskCache);
}

// Move the dragged task to the page under the cursor
Desktop.prototype._moveTask = function ()
{
    var display = true;
    
    // Ensures that a new page is only created if necessary
    if (!this.drag.icon.nextSibling && !this.drag.icon.previousSibling && !getClassName(this.drag.icon.parentNode.nextSibling, "page"))
        this.taskCache.createPage = false;
    // If the dragged task has been moved in a new page
    if (this.taskCache.number_page != this.drag.place.page || this.taskCache.createPage)
    {
        // Remove the old task
        this.drag.icon.parentNode.object.removeTask(this.drag, true);
        // Put the task in a new page
        if (this.taskCache.createPage)
            (new Page()).addTask(this.drag);
        // Otherwise we add the task to the ghost's page
        else
            this.ghost.parentNode.object.addTask(this.drag, this.ghost);
        this.drag.icon.parentNode.object.display(true);
    }
    // If the page hasn't changed
    else if (this.taskCache.number_page == this.drag.place.page && this.ghost)
    {
        this.drag.icon.parentNode.removeChild(this.drag.icon);
        this.ghost.parentNode.insertBefore(this.drag.icon, this.ghost);
    }
    // The page hasn't moved
    else
        display = false;
    if (display)
        this.drag.icon.parentNode.object.display(true);
    this._removeGhostTask();
    this.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + "px";
    this.taskCache = undefined;
    this.drag.icon.style.position = "static";
}

// Remove the ghost task
Desktop.prototype._removeGhostTask = function ()
{
    if (this.ghost)
    {
        this.ghost.parentNode.removeChild(this.ghost);
        this.ghost = undefined;
    }
}

// Get the position of the task in the tasks list
Desktop.prototype._getTaskPosition = function (task)
{
    var number_page = 0;
    var number_task = 0;
    
    for (var page = task.parentNode; page; page = page.previousSibling)
        number_page++;
    for (; task; task = task.previousSibling)
        number_task++;
    return {page : number_page,
            task : number_task};
}

// Scroll the tasks list up or down, depending on the delta
// @param delta : The speed of the scroll. Scroll top if positive, down if negative.
// Should be between 1 and -1.
Desktop.prototype._scrollTasksList = function (delta)
{
    var tasks_list = this.node.tasks_list;
    var height = tasks_list.scrollHeight - this.top_height;
    
    // Nothing to scroll
    if (!height || !delta || (delta > 0 && tasks_list.scrollTop == 0) || (delta < 0 && tasks_list.scrollTop == height))
    {
        this._stopScroll();
        return ;
    }
    // Create the interval that will scroll the tasks
    if (!this.scroll)
    {
        var id = setInterval("gl_desktop._scrollTasksListDo()", 1000 / 30);
        this.scroll = {id : id, delta : delta}
    }
    else
        this.scroll.delta = delta;
}

// This method is called by setInterval, and scroll the tasks list depending on the delta
Desktop.prototype._scrollTasksListDo = function ()
{
    var tasks_list = this.node.tasks_list;
    
    if (!this.scroll)
        return ;
    // Calculate the number of pixels to scroll
    if (this.scroll.delta > 0)
        tasks_list.scrollTop -= (Math.exp(Math.abs(this.scroll.delta)) - 1) * C.Desktop.scrollSpeed;
    else
        tasks_list.scrollTop += (Math.exp(Math.abs(this.scroll.delta)) - 1) * C.Desktop.scrollSpeed;
    this._updateTasksList(this.mouse.y);
    // Scroll top finished
    if (tasks_list.scrollTop <= 0)
    {
        tasks_list.scrollTop = 0;
        clearInterval(this.scroll.id);
        delete this.scroll;
    }
    // Scroll down finished
    else if (tasks_list.scrollTop >= tasks_list.scrollHeight - this.top_height)
    {
        tasks_list.scrollTop = tasks_list.scrollHeight - this.top_height;
        this._stopScroll();
    }
}

// Stop the scrolling of the tasks list
Desktop.prototype._stopScroll = function ()
{
    if (this.scroll)
    {
        clearInterval(this.scroll.id);
        delete this.scroll;
    }
}

/**************************/
/********** Page **********/
/**************************/
var tmp_page_background = 0;//#######################
// A page is a container that can store multiple tasks.
// Pages are stored in the tasks list of the desktop.
function Page()
{
    this.id = gl_uid++;
    this.order = 1;
    var page = this;
    
    // Create the page and insert it in the tasks list
    this.icon = document.createElement("div");
    this.icon.className = "page";
    this.icon.object = this;
    if (tmp_page_background++ % 2)
        this.icon.style.backgroundColor = "#88CCFF";
    else
        this.icon.style.backgroundColor = "#88AAFF";
    gl_desktop.node.tasks_list.insertBefore(this.icon, gl_desktop.node.tasks_list.lastChild);
    addEvent(this.icon, "mousedown", function(event) { page.mouseDown(event); });
    addEvent(this.icon, "mouseover", function(event) { page.mouseOver(event); });
    
    // Display the page in the main area of the desktop
    this.content = document.createElement("div");
    this.content.className = "page";
    this.content.style.display = "none";
    this.content.object = this;
    gl_desktop.node.pages.appendChild(this.content);
    this.onResize();
}

// We clicked on a page
Page.prototype.mouseDown = function (event)
{
    // The current page become the main page
    if (getButton(event) == 0)
        this.display(true);
    // Close the page
    else if (getButton(event) == 1 && getClassName((event.target || event.srcElement), "page"))
        this.close();
}

// Display the page when the mouse is over it
Page.prototype.mouseOver = function (event)
{
    // If we are not dragging something
    if (gl_desktop.drag)
        return ;
    // The display of the page is delayed
    if (!gl_desktop.content.preview)
        gl_desktop.content.currentPage = this;
    // If we are on an other page, display it
    else if (!gl_desktop.content.page || gl_desktop.content.page.id != this.id)
        this.display();
}

// Display the page in the main area of the desktop
// @param main : If the page displayed should be kept as the main page
Page.prototype.display = function (main)
{
    if (main)
        gl_desktop.content.main = this;
    // If the page we try to display is the current page
    if (gl_desktop.content.page && this.id == gl_desktop.content.page.id)
        return ;
    // Hide the old page
    if (gl_desktop.content.page)
    {
        gl_desktop.content.page.content.style.display = "none";
        for (var task = gl_desktop.content.page.icon.firstChild; task; task = task.nextSibling)
            if (getClassName(task, "task"))
                task.object.content.style.display = "none";
    }
    // Display the page
    gl_desktop.content.page = this;
    this.content.style.display = "block";
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        if (getClassName(task, "task"))
            task.object.content.style.display = "block";
    this.onResize();
}

// Close the page
Page.prototype.close = function ()
{
    // Remove the tasks of the page
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        task.object.content.parentNode.removeChild(task.object.content);
    // Remove the page of the tasks list
    this.icon.parentNode.removeChild(this.icon);
    // Remove the page of the main area
    if (gl_desktop.content.page && gl_desktop.content.page.id == this.id)
        gl_desktop.content.page = undefined;
    if (gl_desktop.content.main && gl_desktop.content.main.id == this.id)
        gl_desktop.content.main = undefined;
    if (gl_desktop.content.currentPage && gl_desktop.content.currentPage.id == this.id)
        gl_desktop.content.currentPage = undefined;
    // Change the displayed node
    if (this.content.nextSibling)
        this.content.nextSibling.object.display(!gl_desktop.content.main);
    else if (gl_desktop.content.main)
        gl_desktop.content.main.display();
    else if (this.content.previousSibling)
        this.content.previousSibling.object.display(true);
    this.content.parentNode.removeChild(this.content);
}

// Add a task in the page
// @param task : The task to add
// @param beforeIcon : The icon before which the task will be added in the tasks list
Page.prototype.addTask = function (task, beforeIcon)
{
    this.updateRatios();
    this.number_tasks++;
    // Update the ratio of the tasks
    var ratio_rest = (1 - 1 / this.number_tasks);
    for (var t = this.icon.firstChild; t; t = t.nextSibling)
        if (getClassName(t, "task"))
            t.object.ratio *= ratio_rest;
    task.order = this.order++;
    task.ratio = 1 / this.number_tasks;
    // Put the new task in the tasks list
    if (beforeIcon)
        beforeIcon.parentNode.insertBefore(task.icon, beforeIcon);
    else
        this.icon.appendChild(task.icon);
    // Update the display of the page
    task.content.style.display = "block";
    this.onResize();
}

// Remove a task from the page
// @param task : The task to remove
// @param move : If the task is moved its content is not removed
Page.prototype.removeTask = function (task, move)
{
    task.icon.parentNode.removeChild(task.icon);
    if (!move)
        task.content.parentNode.removeChild(task.content);
    // If the page is empty we close it
    if (!this.icon.firstChild)
        this.close();
    // Otherwise we resize its tasks
    else
    {
        this.updateRatios();
        this.onResize();
    }
}

// When a task is dragged over a page, this method display a preview of its future position
// @param task : The task to preview
Page.prototype.preview = function(task)
{
    this.display();
    // Ensures that we are on an other page
    if (task.icon.parentNode.object.id == this.id)
        return ;
    // Change temporarily the ratio of the task
    var old_ratio = task.ratio;
    task.ratio = 1 / (this.number_tasks + 1);
    // Take into account the new page in the ratios
    var ratio_rest = 1 - (1 / (this.number_tasks + 1));
    for (var t = this.icon.firstChild; t; t = t.nextSibling)
        if (getClassName(t, "task"))
            t.object.ratio *= ratio_rest;
    var tasks = new Object();
    // Build a sorted list of the tasks
    for (var t = this.icon.firstChild; t; t = t.nextSibling)
        if (getClassName(t, "task"))
            tasks[t.object.order] = t.object;
    tasks[this.order + 1] = task;
    // Update the size of the page
    this.left = gl_desktop.content.left;
    this.top = gl_desktop.content.top;
    this.width = gl_desktop.content.width;
    this.height = gl_desktop.content.height;
    // Update the position and size of the tasks
    var i = 0;
    var left = this.left;
    for (var c = 0; c <= this.number_tasks; i++)
        if (tasks[i])
        {
            var t = tasks[i].content;
            t.style.left = left + "px";
            var width = Math.floor((this.width + C.Desktop.task_margin) * tasks[i].ratio);
            left += width;
            if (c == this.number_tasks)
                width += (this.left + this.width + C.Desktop.task_margin - left);
            t.style.width = (width - C.Desktop.task_margin) + "px";
            t.style.top = this.top + "px";
            t.style.height = this.height + "px";
            c++;
        }
    // Set the real ratio of the page back
    task.ratio = old_ratio;
    task.content.style.display = "block";
    // Restore the ratios of the current page
    this.updateRatios();
}

// Update the ratios of the page
Page.prototype.updateRatios = function ()
{
    // Get the missing ratio of the page
    var ratio_missing = 0;
    this.number_tasks = 0;
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        if (getClassName(task, "task"))
        {
            ratio_missing += task.object.ratio;
            this.number_tasks++;
        }
    // Update the ratio of the tasks
    if (ratio_missing <= 0 || ratio_missing >= 1)
        return ;
    ratio_missing = (1 / ratio_missing);
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        if (getClassName(task, "task"))
            task.object.ratio *= ratio_missing;
}

// Resize the tasks of the page
Page.prototype.onResize = function ()
{
    var tasks = new Object();
    
    // Get the new size of the page
    this.left = gl_desktop.content.left;
    this.top = gl_desktop.content.top;
    this.width = gl_desktop.content.width;
    this.height = gl_desktop.content.height;
    // Build a sorted list of the tasks
    for (var t = this.icon.firstChild; t; t = t.nextSibling)
        if (getClassName(t, "task"))
            tasks[t.object.order] = t.object;
    // Update the position and size of the tasks
    var i = 0;
    var left = this.left;
    for (var c = 0; c < this.number_tasks; i++)
        if (tasks[i])
        {
            var task = tasks[i].content;
            task.style.left = left + "px";
            var width = Math.floor((this.width + C.Desktop.task_margin) * tasks[i].ratio);
            left += width;
            if (c + 1 == this.number_tasks)
                width += (this.left + this.width + C.Desktop.task_margin - left);
            task.style.width = (width - C.Desktop.task_margin) + "px";
            task.style.top = this.top + "px";
            task.style.height = this.height + "px";
            c++;
        }
}

/**************************/
/********** Task **********/
/**************************/
var tmptoto = 1;
// A task holds a resource and can be stored in a page
// @param resource : The name of the resource loaded in the task
// @param content : The HTML content of the resource
function Task(resource, content)
{
    this.id = gl_uid++;
    this.resource = resource;
    var task = this;
    
    // Tasks list
    this.icon = document.createElement("div");
    this.icon.className = "task";
    this.icon.object = this;
    this.icon.innerHTML = tmptoto++;//this.resource.charAt(0).toUpperCase();######################
    // This event allows the task to be moved in other pages by being dragged
    addEvent(this.icon, "mousedown", function(event)
    {
        // Move the task
        if (getButton(event) == 0)
        {
            gl_desktop.drag = task;
            disableSelection(false);
            var mouse = mouseCoordinates(event);
            var element = elementCoordinates(task.icon);
            element.y = element.y;
            task.mouse = {x : mouse.x - element.x,
                          y : mouse.y - element.y + gl_desktop.node.tasks_list.scrollTop};
            task.position = {x : element.x,
                             y : element.y};
            task.icon.style.position = "absolute";
            task.icon.style.top = mouse.y - task.mouse.y + "px";
            task.icon.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
            task.place = gl_desktop._getTaskPosition(task.icon);
            task.resistance = true;
            gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + C.Desktop.task_height + "px";
            gl_desktop._updateTasksList(mouse.y);
        }
        // Close the task
        else if (getButton(event) == 1)
            task.icon.parentNode.object.removeTask(task);
    });
    
    // Main area
    this.content = document.createElement("div");
    this.content.className = "task";
    this.content.style.display = "none";
    this.content.object = this;
    this.content.innerHTML = content;
    gl_desktop.node.tasks.appendChild(this.content);
}

// Remove the resistance and start to drag the task
Task.prototype.startDrag = function (y)
{
    this.resistance = false;
    // Update the position of the mouse in the task
    if (y < this.position.y - gl_desktop.node.tasks_list.scrollTop)
    {
        this.mouse.y -= C.Desktop.task_magnetic_drag_y;
        y += C.Desktop.task_magnetic_drag_y;
    }
    else if (y > this.position.y - gl_desktop.node.tasks_list.scrollTop)
    {
        this.mouse.y += C.Desktop.task_magnetic_drag_y;
        y -= C.Desktop.task_magnetic_drag_y;
    }
    // The mouse is outside the task
    if (this.mouse.y < 0)
    {
        y -= 2 - this.mouse.y;
        this.mouse.y = 2;
    }
    else if (this.mouse.y > C.Desktop.task_height)
    {
        y += this.mouse.y - C.Desktop.task_height + 2;
        this.mouse.y = C.Desktop.task_height - 2;
    }
    // Move the task
    this.icon.style.top = y + "px";
    return (y);
}
