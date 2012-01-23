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
    this.node.tasks = getElementsByClassName("tasks", this.node.top, true);
    this.node.middle = getElementsByClassName("middle", this.node.top, true);
    this.node.player_document = getElementsByClassName("player_document", this.node.middle, true);
    this.node.content = getElementsByClassName("content", this.node.middle, true);
    this.node.player_media = getElementsByClassName("player_media", this.node.desktop, true);
    // Set the default values
    this.node.desktop.style.minHeight = C.Desktop.minHeight + "px";
    this.node.player_document.style.height = C.Desktop.player_document_height + "px";
    this.node.player_media.style.height = C.Desktop.player_media_height + "px";
    this.node.menu.style.width = T.Menu.width + "px";
    this.node.tasks.style.width = C.Desktop.tasks_width + "px";
    this.middle_margin_left = T.Menu.width + C.Desktop.tasks_width;
    this.node.middle.style.marginLeft = this.middle_margin_left + "px";
    this.node.tasks.lastChild.style.height = C.Desktop.new_task_height + "px";
    // Set the onmousewheel event
    var mouseWheelEvent = (/Firefox/i.test(navigator.userAgent)) ? "DOMMouseScroll" : "mousewheel";
    addEvent(this.node.tasks, mouseWheelEvent, function(event) { gl_desktop.mouseWheel(event) });
    // The task or page being dragged
    this.drag = undefined;
}

// Resize the desktop
Desktop.prototype.onResize = function ()
{
    var height = (gl_browserSize.height > C.Desktop.minHeight) ? gl_browserSize.height : C.Desktop.minHeight;
    this.top_height = height - C.Desktop.player_media_height;
    this.node.top.style.height = this.top_height + "px";
    this.node.content.style.height = (this.top_height - C.Desktop.player_document_height) + "px";
}

// Open a new task in a new page
// @param resource : The name of the resource that will be loaded in the task
// @param event : The event that triggered the call
Desktop.prototype.open = function (resource, event)
{
	if (getButton(event) != 0)
		return ;
    var desktop = this;
    // Load the task from the resources
	gl_resources.load(resource, function(content)
    {
        // Create the page and the task
        var page = new Page();
        var task = new Task(resource, page);
        // Display the new task
        desktop.node.content.innerHTML = content;
        gl_resources.callJs(resource, task);
    });
}

// Called each time the mouse move
// @param event : The event that triggered the call
Desktop.prototype.mouseMove = function (event)
{
	this.mouse = mouseCoordinates(event);
    // If a task is being dragged
    if (this.drag)
    {
        // Move the dragged task vertically
        var y = this.mouse.y - this.drag.mouse.y;
        if (y < 0)
            this.drag.div.style.top = "0px";
        else if (y > this.top_height - C.Desktop.task_height)
            this.drag.div.style.top = (this.top_height - C.Desktop.task_height) + "px";
        else
            this.drag.div.style.top = y + "px";
        // Move the dragged task horizontally if the mouse exceeds the magnetic limit
        if (this.mouse.x > this.middle_margin_left + C.Desktop.task_magnetic_drag)
        {
            this.drag.div.style.left = (this.mouse.x - this.drag.mouse.x) + "px";
            this._setOriginalGhost();
        }
        // Otherwise update the tasks list
        else
        {
            this.drag.div.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
            this._updateTasks(this.mouse.y);
        }
    }
    // Otherwise a window may be dragged
    else
        moveWindow(event);
    // The top scrolling area of the tasks list
    if (this.mouse.x < this.middle_margin_left && this.mouse.x > T.Menu.width && this.mouse.y <= C.Desktop.tasks_scroll)
        this._scrollTasks(1 - (this.mouse.y / C.Desktop.tasks_scroll));
    // The bottom scrolling area of the tasks list
    else if (this.mouse.x < this.middle_margin_left && this.mouse.x > T.Menu.width && this.mouse.y > this.top_height - C.Desktop.tasks_scroll && this.mouse.y <= this.top_height)
        this._scrollTasks((this.top_height - C.Desktop.tasks_scroll - this.mouse.y) / C.Desktop.tasks_scroll);
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
    this.node.tasks.scrollTop -= (event.detail ? event.detail * (-28) : event.wheelDelta) / C.Desktop.wheelSpeed;
    this._updateTasks(this.mouse.y);
}

// Put the ghost at its original place
Desktop.prototype._setOriginalGhost = function ()
{
    if (!this.ghost || !this.ghost.original)
    {
        if (this.ghost)
            this.ghost.parentNode.removeChild(this.ghost);
        this.ghost = document.createElement("div");
        this.ghost.className = "ghost";
        this.drag.div.parentNode.insertBefore(this.ghost, this.drag.div);
        this.ghost.original = true;
    }
}

// Place a ghost task under the cursor to show the place where the dragged task will be moved
// @param y : The y position of the mouse
Desktop.prototype._updateTasks = function (y)
{
    if (!this.drag)
        return ;
    // Change the reference y depending if the task has been dragged up or down
    y = y - this.drag.mouse.y + this.node.tasks.scrollTop;
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
        this.node.tasks.lastChild.style.height = C.Desktop.new_task_height + C.Desktop.task_height + "px";
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
    this.node.tasks.lastChild.style.height = C.Desktop.new_task_height + "px";
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
    for (page = this.node.tasks.firstChild; page && page.className == "page" && (position < y || !number_page); page = page.nextSibling, number_page++)
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
    // Ensures that a new page is only created if necessary
    if ((this.ghost && this.ghost.original) || (!this.drag.div.nextSibling && !this.drag.div.previousSibling && this.drag.div.parentNode.nextSibling.className != "page"))
        this.taskCache.createPage = false;
    // If the dragged task has been moved
    if (this.taskCache.number_page != this.drag.place.page || this.taskCache.number_task != this.drag.place.task || this.taskCache.createPage)
    {
        // Remove the old task
        var parent = this.drag.div.parentNode;
        this.drag.div.parentNode.removeChild(this.drag.div);
        // Replace the ghost by the new task
        if (!this.taskCache.createPage)
            this.ghost.parentNode.insertBefore(this.drag.div, this.ghost);
        // Put the task in a new page
        else
            new Page(this.drag);
        this._removeGhostTask();
        // If the old page is empty, we remove it
        if (!parent.firstChild)
            parent.parentNode.removeChild(parent);
    }
    else
        this._removeGhostTask();
    this.node.tasks.lastChild.style.height = C.Desktop.new_task_height + "px";
    this.taskCache = undefined;
    this.drag.div.style.position = "static";
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
Desktop.prototype._scrollTasks = function (delta)
{
    var tasks = this.node.tasks;
    var height = tasks.scrollHeight - this.top_height;
    
    // Nothing to scroll
    if (!height || !delta || (delta > 0 && tasks.scrollTop == 0) || (delta < 0 && tasks.scrollTop == height))
    {
        this._stopScroll();
        return ;
    }
    // Create the interval that will scroll the tasks
    if (!this.scroll)
    {
        var id = setInterval("gl_desktop._scrollTasksDo()", 1000 / C.animationFPS);
        this.scroll = {id : id, delta : delta}
    }
    else
        this.scroll.delta = delta;
}

// This method is called by setInterval, and scroll the tasks list depending on the delta
Desktop.prototype._scrollTasksDo = function ()
{
    var tasks = this.node.tasks;
    
    if (!this.scroll)
        return ;
    // Calculate the number of pixels to scroll
    if (this.scroll.delta > 0)
        tasks.scrollTop -= (Math.exp(Math.abs(this.scroll.delta)) - 1) * 15;
    else
        tasks.scrollTop += (Math.exp(Math.abs(this.scroll.delta)) - 1) * 15;
    this._updateTasks(this.mouse.y);
    // Scroll top finished
    if (tasks.scrollTop <= 0)
    {
        tasks.scrollTop = 0;
        clearInterval(this.scroll.id);
        delete this.scroll;
    }
    // Scroll down finished
    else if (tasks.scrollTop >= tasks.scrollHeight - this.top_height)
    {
        tasks.scrollTop = tasks.scrollHeight - this.top_height;
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

/********** Page **********/
var tmp_page_background = 0;//#######################
// A page is a container that can store multiple tasks.
// Pages are stored in the tasks list of the desktop.
// @param task : The first task of the page (optional)
function Page(task)
{
    // Create the page and insert it in the tasks list
    this.id = gl_uid++;
    this.div = document.createElement("div");
    this.div.object = this;
    this.div.className = "page";
    if (tmp_page_background++ % 2)
        this.div.style.backgroundColor = "#88CCFF";
    else
        this.div.style.backgroundColor = "#88AAFF";
    var page = this.div;
    // Close the page
    addEvent(this.div, "mousedown", function(event)
    {
        if (getButton(event) == 1 && (event.target || event.srcElement).className == "page")
            page.parentNode.removeChild(page);
    });
    if (task)
        this.div.appendChild(task.div);
    gl_desktop.node.tasks.insertBefore(this.div, gl_desktop.node.tasks.lastChild);
}

/********** Task **********/
var tmp_number = 1;//#######################
// A task holds a resource and can be stored in a page
// @param resource : The name of the resource loaded in the task
// @param page : The page of the task (optional)
function Task(resource, page)
{
    // Create the task
    this.id = gl_uid++;
    this.resource = resource;
    this.div = document.createElement("div");
    this.div.object = this;
    this.div.className = "task";
    //task.div.innerHTML = task.resource.charAt(0).toUpperCase();
    this.div.innerHTML = tmp_number++;
    var task = this;
    // This event allows the task to be moved in other pages by being dragged
    addEvent(this.div, "mousedown", function(event)
    {
        // Move the task
        if (getButton(event) == 0)
        {
            gl_desktop.drag = task;
            disableSelection(false);
            var mouse = mouseCoordinates(event);
            var element = elementCoordinates(task.div);
            element.y = element.y;
            task.mouse = {x : mouse.x - element.x,
                          y : mouse.y - element.y + gl_desktop.node.tasks.scrollTop};
            task.position = {x : element.x,
                             y : element.y};
            task.div.style.position = "absolute";
            task.div.style.top = mouse.y - task.mouse.y + "px";
            task.div.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
            task.place = gl_desktop._getTaskPosition(task.div);
            gl_desktop.node.tasks.lastChild.style.height = C.Desktop.new_task_height + C.Desktop.task_height + "px";
            gl_desktop._updateTasks(mouse.y);
        }
        // Close the task
        else if (getButton(event) == 1)
        {
            var page = task.div.parentNode;
            task.div.parentNode.removeChild(task.div);
            if (!page.firstChild)
                page.parentNode.removeChild(page);
        }
    });
    // Put the task in a page
    if (page)
    {
        page.div.appendChild(this.div);
        // Scroll to the bottom of the list
        gl_desktop.node.tasks.scrollTop = gl_desktop.node.tasks.scrollHeight - gl_desktop.top_height - C.Desktop.new_task_height;
    }
}
