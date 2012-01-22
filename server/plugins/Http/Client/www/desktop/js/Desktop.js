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
	if (event != undefined && event.button == 2)
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
	var mouse = mouseCoordinates(event);
    // If a task is being dragged
    if (this.drag)
    {
        // Move the dragged task vertically
        var y = mouse.y - this.drag.mouse.y
        if (y < 0)
            this.drag.div.style.top = "0px";
        else if (y > this.top_height - C.Desktop.task_height)
            this.drag.div.style.top = (this.top_height - C.Desktop.task_height) + "px";
        else
            this.drag.div.style.top = y + "px";
        // Move the dragged task horizontally if the mouse exceeds the magnetic limit
        if (mouse.x > this.middle_margin_left + C.Desktop.task_magnetic_drag)
        {
            this.drag.div.style.left = (mouse.x - this.drag.mouse.x) + "px";
            this._setOriginalGhost();
        }
        // Otherwise update the tasks list
        else
        {
            this.drag.div.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
            this._updateTasks(mouse.y);
        }
    }
    // Otherwise a window may be dragged
    else
        moveWindow(event);
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
    // Change the reference y depending if the task has been dragged up or down
    y = y - this.drag.mouse.y;
    if (y > this.drag.position.y)
        y += C.Desktop.task_height;
    this._removeGhostTask();
    // Get the position of the task under the cursor
    var data = this._getCursorPosition(y);
    var page = data.page;
    var task = data.task;
    // If we want to create a page, the ghost is not required
    if (this.taskCache.createPage)
        return ;
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
    if (this.taskCache && y < this.taskCache.position && y > this.taskCache.limit)
        return (this.taskCache);
    // Run through the tasks in order to find the position of the task under the cursor
    for (page = this.node.tasks.firstChild; page && (position < y || !number_page); page = page.nextSibling, number_page++)
    {
        position += C.Desktop.page_padding;
        for (task = page.firstChild, number_task = 0; task && (position < y || !number_task); task = task.nextSibling, number_task++)
            position += C.Desktop.task_height;
        if (!task)
            position += C.Desktop.page_padding;
    }
    // Get the correct nodes
    (page) ? page = page.previousSibling : page = this.node.tasks.lastChild;
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
    if ((this.ghost && this.ghost.original) || (!this.drag.div.nextSibling && !this.drag.div.previousSibling && !this.drag.div.parentNode.nextSibling))
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
        {
            var div = document.createElement("div");
            div.className = "page";
            if (tmp_page_background++ % 2)
                div.style.backgroundColor = "#88CCFF";
            else
                div.style.backgroundColor = "#88AAFF";
            div.appendChild(this.drag.div);
            this.node.tasks.appendChild(div);
        }
        this._removeGhostTask();
        // If the old page is empty, we remove it
        if (!parent.firstChild)
            parent.parentNode.removeChild(parent);
    }
    else
        this._removeGhostTask();
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

/********** Page **********/
var tmp_page_background = 0;//#######################
// A page is a container that can store multiple tasks.
// Pages are stored in the tasks list of the desktop.
function Page()
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
    gl_desktop.node.tasks.appendChild(this.div);
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
        gl_desktop.drag = task;
        disableSelection(false);
        var mouse = mouseCoordinates(event);
        var element = elementCoordinates(task.div);
        task.mouse = {x : mouse.x - element.x,
                      y : mouse.y - element.y};
        task.position = {x : element.x,
                         y : element.y};
        task.div.style.position = "absolute";
        task.div.style.top = element.y + "px";
        task.div.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
        task.place = gl_desktop._getTaskPosition(task.div);
        gl_desktop._updateTasks(mouse.y);
    });
    // Put the task in a page
    if (page)
        page.div.appendChild(this.div);
}
