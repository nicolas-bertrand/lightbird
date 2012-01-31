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
    this.node.preview = getElementsByClassName("preview", this.node.middle, true);
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
    // Resize the page currently displayed
    if (this.content.page)
        this.content.page.onResize();
}

// Called each time the mouse move
// @param event : The event that triggered the call
Desktop.prototype.mouseMove = function (event)
{
	this.mouse = mouseCoordinates(event);
    // If a task is being dragged we move it
    if (this.drag && getClassName(this.drag.icon, "task"))
        this.drag.mouseMoveDesktop(this.mouse);
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
        this.drag.mouseUp();
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
    if (this.drag)
        this.drag.updateTasksList(this.mouse.y);
}

// Doesn't display the page under the cursor immediatly
Desktop.prototype.mouseOverTasksList = function (event)
{
    if (!getEventRelatedTarget(event, "tasks_list", 3))
    {
        this.content.timeout = setTimeout("gl_desktop.mouseOverTasksListTimeout()", C.Desktop.pagePreviewDelay);
        // These variables must be undefined in the tasks list
        gl_desktop.addTaskPosition = undefined;
        gl_desktop.addTaskParent = undefined;
    }
}
Desktop.prototype.mouseOverTasksListTimeout = function ()
{
    delete this.content.timeout;
    if (this.content.currentPage)
        this.content.currentPage.display();
    this.content.currentPage = undefined;
    this.content.preview = true;
}

// Displays the main page
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
    gl_desktop.node.preview.style.display = "none";
}

/********************************/
/********** Tasks list **********/
/********************************/
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
    if (this.drag)
        this.drag.updateTasksList(this.mouse.y);
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
// @param position : The position of the new task in the page (n s e w)
// @param parent : If defined, the parent is cut in half and the task take a half
// @param beforeIcon : The icon before which the task will be added in the tasks list
Page.prototype.addTask = function (task, position, parent, beforeIcon)
{
    this.addNode(task, position, parent);
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
        this.removeNode(task.node);
        this.onResize();
    }
}

// Resize the tasks of the page
Page.prototype.onResize = function ()
{   
    // Get the new size of the page
    this.left = gl_desktop.content.left;
    this.top = gl_desktop.content.top;
    this.width = gl_desktop.content.width;
    this.height = gl_desktop.content.height;
    // Render the tasks tree
    if (this.tree)
        this.renderTree(this.tree, this.left, this.top, this.width, this.height);
}

// Add a node to the tree that represents the hierarchy of the tasks in the page
// @param task : The task of the node
// @param position : The position of the task relative to the parent (n s e w)
// @param parent : If defined, the parent is cut in half and both of the tasks takes one part
Page.prototype.addNode = function (task, position, parent)
{
    // The default position
    if (!position)
        position = C.Desktop.defaultPosition;
    // Create the tree
    if (!this.tree)
    {
        this.tree = { task : task };
        task.node = this.tree;
    }
    // There is only one node in the tree
    else if (!parent && this.tree.task)
        this._addNode(task, position, this.tree);
    // Add a node at the root of the tree
    else if (!parent)
    {
        // Change the root of the tree (the old tree becomes a leaf)
        if (position == "s" || position == "e")
        {
            this.tree = { first : this.tree };
            this.tree.second = { task : task };
            task.node = this.tree.second;
        }
        // The order changes depending on the position
        else
        {
            this.tree = { second : this.tree };
            this.tree.first = { task : task };
            task.node = this.tree.first;
        }
        // Set the ratio depending on the position, and the number of nodes already in that position in the tree
        this.tree.h = 1;
        this.tree.v = 1;
        if (position == "w")
            this.tree.h = 1 / (this.countNode(this.tree, "h") + 2);
        else if (position == "e")
            this.tree.h = 1 - 1 / (this.countNode(this.tree, "h") + 2);
        else if (position == "n")
            this.tree.v = 1 / (this.countNode(this.tree, "v") + 2);
        else
            this.tree.v = 1 - 1 / (this.countNode(this.tree, "v") + 2);
        // Set the parents of the leafs
        this.tree.first.parent = this.tree;
        this.tree.second.parent = this.tree;
    }
    // Add a node after the parent
    else
        this._addNode(task, position, parent.node);
}
// Helper method of Page.addNode
Page.prototype._addNode = function (task, position, node)
{
    // The horizontal and vertical ratios
    node.h = 1;
    node.v = 1;
    // Depending of the position, one is cut in half
    (position == "w" || position == "e") ? node.h = 0.5 : node.v = 0.5;
    // Build the first and second leaf of the node
    var set = function (first, second)
    {
        node.first = { task : first };
        first.node = node.first;
        node.second = { task : second };
        second.node = node.second;
    }
    // The order of the first and second leaf changes depending on the position
    if (position == "n" || position == "w")
        set(task, node.task);
    else
        set(node.task, task);
    // Set the parent node of the leafs
    node.first.parent = node;
    node.second.parent = node;
    // The node is no longer a leaf
    node.task = undefined;
}

// Count recursively the number of leafs that are horizontal or vertical
// @param node : The current node of the tree
// @param ratio : "h" or "v"
Page.prototype.countNode = function (node, ratio)
{
    var result = 0;
    
    // If the node has one leaf and the good ratio
    if (node[ratio] && node[ratio] != 1 && ((node.first && node.first.task) || (node.second && node.second.task)))
        result = 1;
    // Continue the counting on the first branch
    if (node.first)
        result += this.countNode(node.first, ratio);
    // Same on the second branch
    if (node.first)
        result += this.countNode(node.second, ratio);
    return (result);
}

// Remove a node from the tasks tree
// @param node : The node to remove
Page.prototype.removeNode = function (node)
{
    var parent = node.parent;
    // Get the branch to keep
    if (parent.first !== node)
        var keep = parent.first;
    else
        var keep = parent.second;
    // The remaining branch of the parent node become the parent
    parent.task = keep.task;
    parent.h = keep.h;
    parent.v = keep.v;
    parent.first = keep.first;
    parent.second = keep.second;
    if (parent.task)
        parent.task.node = parent;
    if (parent.first)
        parent.first.parent = parent;
    if (parent.second)
        parent.second.parent = parent;
}

// Displays the tasks tree recursively
// @param node : The current node
// @param left, top, width, height : The remaining area to be filled by the tasks
Page.prototype.renderTree = function (node, left, top, width, height)
{
    // The node is a leaf, so we display its task
    if (node.task)
    {
        // Keep the size of the area in the task
        node.task.left = left;
        node.task.top = top;
        node.task.width = width;
        node.task.height = height;
        // Update the size of the task
        node.task.content.style.left = left + "px";
        node.task.content.style.top = top + "px";
        node.task.content.style.width = width + "px";
        node.task.content.style.height = height + "px";
    }
    // Otherwise we split the area based on the ratios, and we dive deeper into the tree
    else
    {
        // Split horizontally
        if (node.h != 1)
        {
            this.renderTree(node.first, left, top, width * node.h - C.Desktop.task_margin, height);
            this.renderTree(node.second, left + width * node.h + C.Desktop.task_margin, top, width * (1 - node.h) - C.Desktop.task_margin, height);
        }
        // Split vertically
        else
        {
            this.renderTree(node.first, left, top, width, height * node.v - C.Desktop.task_margin);
            this.renderTree(node.second, left, top + height * node.v + C.Desktop.task_margin, width, height * (1 - node.v) - C.Desktop.task_margin);
        }
    }
}

// When a task is dragged over a page or an other task, this method display a preview of its future position
// @param task : The task on which the preview is applied. If undefined, the preview is shown on the page.
// @param position : The future position of the task (n s e w)
Page.prototype.preview = function(task, position)
{
    // Don't display a preview when we are on a new page
    if (gl_desktop.drag.taskCache.createPage)
        return ;
    // Hide the content of the dragged task if we are not on its page
    if (gl_desktop.drag.icon.parentNode.object != this)
        gl_desktop.drag.content.style.display = "none";
    // The default position
    if (!position)
        position = C.Desktop.defaultPosition;
    var preview = gl_desktop.node.preview;
    // No preview is needed if the task or the page are the same
    if ((task && gl_desktop.drag == task) || (!task && gl_desktop.drag.icon.parentNode.object == this))
        preview.style.display = "none";
    // Display the preview on a task
    else if (task)
    {
        // Move the preview in the task in order to receive the mousemove events
        preview.parentNode.removeChild(preview);
        task.content.appendChild(preview);
        // Display the preview based on the position of the mouse in the task
        preview.style.display = "block";
        preview.style.top = 0 + "px";
        preview.style.left = 0 + "px";
        preview.style.width = task.width + "px";
        preview.style.height = task.height + "px";
        if (position == "n" || position == "s")
            preview.style.height = task.height / 2 + "px";
        if (position == "s")
            preview.style.top = task.height / 2 + "px";
        if (position == "w" || position == "e")
            preview.style.width = task.width / 2 + "px";
        if (position == "e")
            preview.style.left = task.width / 2 + "px";
    }
    // Display the preview on the page
    else
    {
        // Move the preview in a node in static position
        preview.parentNode.removeChild(preview);
        gl_desktop.node.middle.appendChild(preview);
        // Display the preview based on the position of the mouse in the page
        preview.style.display = "block";
        preview.style.top = this.top + "px";
        preview.style.left = this.left + "px";
        preview.style.width = this.width + "px";
        preview.style.height = this.height + "px";
        if (position == "n" || position == "s")
            preview.style.height = this.height / 2 + "px";
        if (position == "s")
            preview.style.top = this.height / 2 + "px";
        if (position == "w" || position == "e")
            preview.style.width = this.width / 2 + "px";
        if (position == "e")
            preview.style.left = this.left + this.width / 2 + "px";
    }
    this.display();
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
    this.position = "right";
    var task = this;
    
    // Tasks list
    this.icon = document.createElement("div");
    this.icon.className = "task";
    this.icon.object = this;
    this.icon.innerHTML = tmptoto++;//this.resource.charAt(0).toUpperCase();######################
    addEvent(this.icon, "mousedown", function(event) { task.mouseDown(event); });
    
    // Content
    this.content = document.createElement("div");
    this.content.className = "task";
    this.content.style.display = "none";
    this.content.object = this;
    this.content.innerHTML = content;
    addEvent(this.content, "mousemove", function(event) { if (gl_desktop.drag) task.mouseMove(event); });
    gl_desktop.node.tasks.appendChild(this.content);
}

// We clicked on the task
Task.prototype.mouseDown = function (event)
{
    // Prepare the dragging of the page
    if (getButton(event) == 0)
    {
        gl_desktop.drag = this;
        // The selection is disabled during the drag
        disableSelection(false);
        var mouse = mouseCoordinates(event);
        var element = elementCoordinates(this.icon);
        // The coordinates of the mouse inside the task icon
        this.mouse = {x : mouse.x - element.x,
                      y : mouse.y - element.y + gl_desktop.node.tasks_list.scrollTop};
        // The coordinates of the task icon
        this.element = {x : element.x,
                        y : element.y};
        this.icon.style.position = "absolute";
        this.icon.style.top = mouse.y - this.mouse.y + "px";
        this.icon.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
        // The place of the task icon in the tasks list
        this.place = gl_desktop._getTaskPosition(this.icon);
        // There is a little resistance before the task begins to move
        this.resistance = true;
        // This class name has a hight z-index, in order to always see the icon of the task
        setClassName(this.icon, "drag");
        // Update the size of the new page div, otherwise the tasks list may scroll
        gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + C.Desktop.task_height + "px";
        this.updateTasksList(mouse.y);
    }
    // Close the task
    else if (getButton(event) == 1)
        this.icon.parentNode.object.removeTask(this);
}

// The mouse moved over the content of a task
// Get the position of the mouse in the task (north south east west)
Task.prototype.mouseMove = function (event)
{
    var mouse = mouseCoordinates(event);
    var x = mouse.x - this.left;
    var y = mouse.y - this.top;
    var ratio = this.width / this.height;
    var oldPosition = gl_desktop.addTaskPosition;
    var oldParent = gl_desktop.addTaskParent;
    
    // In the corner of the task we use the diagonal to get the position
    if (!gl_desktop.addTaskPosition || y < this.height / 3 && x < this.width / 3 || y < this.height / 3 && x > this.width * 2 / 3
                                    || y > this.height * 2 / 3 && x < this.width / 3 || y > this.height * 2 / 3 && x > this.width * 2 / 3)
    {
        if (x > y * ratio && (this.width - x) > y * ratio)
            gl_desktop.addTaskPosition = "n";
        else if (x > y * ratio)
            gl_desktop.addTaskPosition = "e";
        else if ((this.width - x) > y * ratio)
            gl_desktop.addTaskPosition = "w";
        else
            gl_desktop.addTaskPosition = "s";
    }
    // Facilites the transition east/west
    else if (gl_desktop.addTaskPosition == "e" || gl_desktop.addTaskPosition == "w")
    {
        if (y > this.height / 3 && y < this.height * 2 / 3 && x < this.width / 2)
            gl_desktop.addTaskPosition = "w";
        else if (y > this.height / 3 && y < this.height * 2 / 3 && x > this.width / 2)
            gl_desktop.addTaskPosition = "e";
        else if (y < this.height / 2)
            gl_desktop.addTaskPosition = "n";
        else
            gl_desktop.addTaskPosition = "s";
    }
    // Facilites the transition north/south
    else
    {
        if (x > this.width / 3 && x < this.width * 2 / 3 && y < this.height / 2)
            gl_desktop.addTaskPosition = "n";
        else if (x > this.width / 3 && x < this.width * 2 / 3 && y > this.height / 2)
            gl_desktop.addTaskPosition = "s";
        else if (x < this.width / 2)
            gl_desktop.addTaskPosition = "w";
        else
            gl_desktop.addTaskPosition = "e";
    }
    gl_desktop.addTaskParent = this;
    // If the position has changed, the preview is displayed
    if (oldPosition != gl_desktop.addTaskPosition || oldParent != gl_desktop.addTaskParent)
        this.icon.parentNode.object.preview(gl_desktop.addTaskParent, gl_desktop.addTaskPosition);
}

// Move the task
// @param mouse : The position of the mouse
Task.prototype.mouseMoveDesktop = function (mouse)
{
    var y = mouse.y - this.mouse.y;
    
    // Top limit
    if (y < 0)
        this.icon.style.top = "0px";
    // Bottom limit
    else if (y > gl_desktop.top_height - C.Desktop.task_height)
        this.icon.style.top = (gl_desktop.top_height - C.Desktop.task_height) + "px";
    // The task has a little resistance before starting the drag
    else if (y >= this.element.y - gl_desktop.node.tasks_list.scrollTop - C.Desktop.task_magnetic_drag_y
             && y <= this.element.y - gl_desktop.node.tasks_list.scrollTop + C.Desktop.task_magnetic_drag_y && this.resistance)
        this.icon.style.top = this.element.y - gl_desktop.node.tasks_list.scrollTop + "px";
    // Start the dragging
    else if (this.resistance)
        y = this.startDrag(y);
    // Move the dragged task vertically
    else
        this.icon.style.top = y + "px";
    // Move the dragged task horizontally if the mouse exceeds the magnetic limit
    if (mouse.x > gl_desktop.middle_margin_left + C.Desktop.task_magnetic_drag_x)
    {
        this.icon.style.left = (mouse.x + C.Desktop.free_drag_shift_x) + "px";
        this.icon.style.top = (mouse.y + C.Desktop.free_drag_shift_y) + "px";
    }
    // Otherwise update the tasks list
    else if (!this.resistance)
    {
        this.icon.style.left = (T.Menu.width + C.Desktop.page_padding) + "px";
        this.updateTasksList(mouse.y);
    }
}

// Place a ghost task under the cursor to show the place where the dragged task will be moved
// @param y : The y position of the mouse
Task.prototype.updateTasksList = function (y)
{
    // Change the reference y depending if the task has been dragged up or down
    y = y - this.mouse.y + gl_desktop.node.tasks_list.scrollTop;
    if (y > this.element.y)
        y += C.Desktop.task_height;
    gl_desktop._removeGhostTask();
    // Get the position of the task under the cursor
    var data = this.getCursorPosition(y);
    var page = data.page;
    var task = data.task;
    // If we want to create a page, the ghost is not required
    if (this.taskCache.createPage)
    {
        gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + C.Desktop.task_height + "px";
        gl_desktop.previewNewPage(this);
        return ;
    }
    // Create a ghost task in the current page
    gl_desktop.ghost = document.createElement("div");
    gl_desktop.ghost.className = "ghost";
    // The current page is above the original page
    if (data.number_page <= this.place.page)
    {
        if (!task.nextSibling && y > data.position - C.Desktop.page_padding - C.Desktop.task_height / 2)
            page.appendChild(gl_desktop.ghost);
        else if (task.nextSibling && y > data.position - C.Desktop.task_height / 2)
            page.insertBefore(gl_desktop.ghost, task.nextSibling);
        else
            page.insertBefore(gl_desktop.ghost, task);
    }
    // The current page is below the original page
    else
    {
        if (!task.nextSibling && y < data.position - C.Desktop.page_padding - C.Desktop.task_height / 2)
            page.insertBefore(gl_desktop.ghost, task);
        else if (task.nextSibling && y < data.position - C.Desktop.task_height / 2)
            page.insertBefore(gl_desktop.ghost, task);
        else if (task.nextSibling)
            page.insertBefore(gl_desktop.ghost, task.nextSibling);
        else
            page.appendChild(gl_desktop.ghost);
    }
    // Display the page under which the cursor is
    if (!gl_desktop.content.page || page.object.id != gl_desktop.content.page.id)
        page.object.preview();
    gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + "px";
}

// Get the position of the task under the cursor and some other data
// @param y : The y position of the mouse
Task.prototype.getCursorPosition = function (y)
{
    var position = 0;
    var number_page = 0;
    var number_task = 0;
    var page, task;
    
    // If the position have already been calculated for this interval, we return the cache
    if (this.taskCache && y < this.taskCache.position && y > this.taskCache.limit && !this.taskCache.createPage)
        return (this.taskCache);
    // Run through the tasks in order to find the position of the task under the cursor
    for (page = gl_desktop.node.tasks_list.firstChild; page && getClassName(page, "page") && (position < y || !number_page); page = page.nextSibling, number_page++)
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
Task.prototype.mouseUp = function ()
{
    // Ensures that a new page is only created if necessary
    if (!this.icon.nextSibling && !this.icon.previousSibling && !getClassName(this.icon.parentNode.nextSibling, "page"))
        this.taskCache.createPage = false;
    // If the dragged task has been moved in a new page
    if (this.taskCache.number_page != this.place.page || this.taskCache.createPage)
    {
        // Remove the old task
        this.icon.parentNode.object.removeTask(this, true);
        // Put the task in a new page
        if (this.taskCache.createPage)
            (new Page()).addTask(this);
        // Otherwise we add the task to the ghost's page
        else
            gl_desktop.ghost.parentNode.object.addTask(this, gl_desktop.addTaskPosition, gl_desktop.addTaskParent, gl_desktop.ghost);
        this.icon.parentNode.object.display(true);
    }
    // If the page hasn't changed
    else if (this.taskCache.number_page == this.place.page && gl_desktop.ghost)
    {
        // Change the position of the task in the tree
        if (gl_desktop.node.preview.style.display == "block")
        {
            var page = this.icon.parentNode.object;
            page.removeNode(this.node);
            page.addNode(this, gl_desktop.addTaskPosition, gl_desktop.addTaskParent);
            page.onResize();
        }
        // Move the task in the tasks list
        this.icon.parentNode.removeChild(this.icon);
        gl_desktop.ghost.parentNode.insertBefore(this.icon, gl_desktop.ghost);
    }
    this.icon.parentNode.object.display(true);
    gl_desktop._removeGhostTask();
    gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.new_task_height + "px";
    this.taskCache = undefined;
    this.icon.style.position = "static";
    removeClassName(this.icon, "drag");
    gl_desktop.node.preview.style.display = "none";
}

// Remove the resistance and start to drag the task
Task.prototype.startDrag = function (y)
{
    this.resistance = false;
    // Update the position of the mouse in the task
    if (y < this.element.y - gl_desktop.node.tasks_list.scrollTop)
    {
        this.mouse.y -= C.Desktop.task_magnetic_drag_y;
        y += C.Desktop.task_magnetic_drag_y;
    }
    else if (y > this.element.y - gl_desktop.node.tasks_list.scrollTop)
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
