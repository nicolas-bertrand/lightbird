/* This file manages the desktop. */

// The desktop singleton
var gl_desktop;
// Allows to get a unique id across the current session.
// Increment the value each time an id is needed.
var gl_uid = 0;

/*****************************/
/********** Desktop **********/
/*****************************/{
function Desktop()
{
    // Stores the nodes of the desktop
    this.node = new Object();
    this.node.desktop = document.getElementById("desktop");
    this.node.top = getElementsByClassName("top", this.node.desktop, true);
    this.node.menu = getElementsByClassName("menu", this.node.top, true);
    this.node.tasks_list = getElementsByClassName("tasks_list", this.node.top, true);
    this.node.resize_tasks_list = getElementsByClassName("resize_tasks_list", this.node.top, true);
    this.node.middle = getElementsByClassName("middle", this.node.top, true);
    this.node.player_document = getElementsByClassName("player_document", this.node.middle, true);
    this.node.pages = getElementsByClassName("pages", this.node.middle, true);
    this.node.tasks = getElementsByClassName("tasks", this.node.middle, true);
    this.node.preview = getElementsByClassName("preview", this.node.middle, true);
    this.node.player_media = getElementsByClassName("player_media", this.node.desktop, true);
    // Set the default values
    this.node.desktop.style.minHeight = C.Desktop.minHeight + "px";
    this.node.desktop.style.minWidth = C.Desktop.minWidth + "px";
    this.node.player_document.style.height = C.Desktop.playerDocumentHeight + "px";
    this.node.player_media.style.height = C.Desktop.playerMediaHeight + "px";
    this.node.menu.style.width = T.Menu.width + "px";
    this.node.tasks_list.style.width = C.Desktop.tasksListWidth + "px";
    this.node.resize_tasks_list.style.width = C.Desktop.resizeTasksListWidth + "px";
    this.middleMarginLeft = T.Menu.width + C.Desktop.tasksListWidth + C.Desktop.resizeTasksListWidth;
    this.node.middle.style.marginLeft = this.middleMarginLeft + "px";
    this.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + "px";
    this.taskIconHeight = (C.Desktop.tasksListWidth - C.Desktop.tasksListPadding * 2) * C.Desktop.taskHeightRatio;
    this.content = new Object();
    // Events
    var mouseWheelEvent = (/Firefox/i.test(navigator.userAgent)) ? "DOMMouseScroll" : "mousewheel";
    addEvent(this.node.desktop, mouseWheelEvent, function(event) { gl_desktop.mouseWheel(event); });
    addEvent(this.node.desktop, "mousemove", function(event) { gl_desktop.mouseMove(event); });
    addEvent(this.node.desktop, "mouseup", function(event) { gl_desktop.mouseUp(event); });
    addEvent(this.node.tasks_list, "mouseover", function(event) { gl_desktop.mouseOverTasksList(event); });
    addEvent(this.node.tasks_list, "mouseout", function(event) { gl_desktop.mouseOutTasksList(event); });
    addEvent(this.node.resize_tasks_list, "mousedown", function(event) { (new ResizeTasksList()).mouseDown(event); });
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
        gl_desktop.node.tasks_list.scrollTop = gl_desktop.node.tasks_list.scrollHeight - gl_desktop.topHeight - C.Desktop.newTaskHeight;
    });
}

// Resize the desktop
Desktop.prototype.onResize = function ()
{
    var width = (gl_browserSize.width > C.Desktop.minWidth) ? gl_browserSize.width : C.Desktop.minWidth;
    var height = (gl_browserSize.height > C.Desktop.minHeight) ? gl_browserSize.height : C.Desktop.minHeight;
    this.topHeight = height - C.Desktop.playerMediaHeight;
    this.node.top.style.height = this.topHeight + "px";
    this.content.left = this.middleMarginLeft;
    this.content.top = C.Desktop.playerDocumentHeight;
    this.content.width = width - this.middleMarginLeft;
    this.content.height = this.topHeight - C.Desktop.playerDocumentHeight;
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
    if (this.drag instanceof Task)
        this.drag.mouseMoveTask(this.mouse);
    // If a task is being resized
    else if (this.drag instanceof TaskTreeNode)
        this.drag.mouseMove(this.mouse.x, this.mouse.y);
    // The tasks list is being resized
    else if (this.drag instanceof ResizeTasksList)
        this.drag.mouseMove(this.mouse);
    // Otherwise a window may be dragged
    else
        moveWindow(event);
    // The top scrolling area of the tasks list
    if (this.mouse.x < this.middleMarginLeft && this.mouse.x > T.Menu.width && this.mouse.y <= C.Desktop.tasksListScrollHeight)
        this._scrollTasksList(1 - (this.mouse.y / C.Desktop.tasksListScrollHeight));
    // The bottom scrolling area of the tasks list
    else if (this.mouse.x < this.middleMarginLeft && this.mouse.x > T.Menu.width && this.mouse.y > this.topHeight - C.Desktop.tasksListScrollHeight && this.mouse.y <= this.topHeight)
        this._scrollTasksList((this.topHeight - C.Desktop.tasksListScrollHeight - this.mouse.y) / C.Desktop.tasksListScrollHeight);
    // Ensures that the scrolling is stopped outside of the scrolling areas
    else if (this.scroll)
        this._stopScroll();
}

// Called when the mouse is up
// @param event : The event that triggered the call
Desktop.prototype.mouseUp = function (event)
{
    // If something in the desktop is being dragged, we stop it
    if (this.drag)
    {
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
    var delta = Math.round((event.detail ? -event.detail : event.wheelDelta / 28));
    // Scroll the tasks list
    if (this.overTasksList)
    {
        this.node.tasks_list.scrollTop -= delta * C.Desktop.mouseWheelScrollSpeed;
        if (this.drag instanceof Task)
            this.drag.updateTasksList(this.mouse.y);
    }
    // Change the displayed page
    else if (this.drag instanceof Task)
    {
        // Get the position of the ghost task
        if (delta > 0 || !this.drag.taskCache || !this.drag.taskCache.createPage)
            this.drag.moveTaskWheel = Math.max(this.drag.moveTaskWheel - delta * C.Desktop.moveTaskWheelSpeed, -this.node.tasks_list.scrollTop);
        var oldPage = this.content.page;
        // Move the ghost according to the new position
        this.drag.updateTasksList(this.drag.moveTaskWheel);
        // Display the preview of the task in the page if it has changed
        if (this.content.page && oldPage != this.content.page)
        {
            gl_desktop.addTaskPosition = C.Desktop.defaultPosition;
            gl_desktop.addTaskParent = undefined;
            this.content.page.preview();
        }
        // If the ghost is out of the displayed area, we scroll to it
        if (this.ghost)
        {
            var position = elementCoordinates(this.ghost).y;
            // Scroll up
            if (position < this.node.tasks_list.scrollTop)
                this.node.tasks_list.scrollTop = position;
            // Scroll down
            else if (position + this.taskIconHeight > this.node.tasks_list.scrollTop + this.topHeight)
                this.node.tasks_list.scrollTop = position + this.taskIconHeight - this.topHeight;
        }
        else
            this.node.tasks_list.scrollTop = this.node.tasks_list.scrollHeight;
    }
    // Resize the tasks margin
    if (this.drag instanceof TaskTreeNode)
        this.drag.mouseWheel(delta);
    // Change the padding of the tasks list
    if (this.drag instanceof ResizeTasksList)
        this.drag.mouseWheel(delta);
}

// Doesn't display the page under the cursor immediatly
Desktop.prototype.mouseOverTasksList = function (event)
{
    // Ensures that we are really on the tasks list
    var target = event.target || event.srcElement;
    if (!getEventRelatedTarget(event, "tasks_list", 5) && (!target.object || target.object != gl_desktop.drag))
    {
        this.content.timeout = setTimeout("gl_desktop.mouseOverTasksListTimeout()", C.Desktop.pagePreviewDelay);
        // These variables must be undefined in the tasks list
        gl_desktop.addTaskPosition = undefined;
        gl_desktop.addTaskParent = undefined;
        // Ensures that the preview is correctly displayed
        if (this.drag instanceof Task && this.content.page)
            this.content.page.preview();
        this.overTasksList = true;
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
    if (!getEventRelatedTarget(event, "tasks_list", 5) && this.overTasksList)
    {
        if (!this.drag)
        {
            // Display the main page
            if (gl_desktop.content.main)
                gl_desktop.content.main.display();
            // No preview is going to be displayed since we are out of the tasks list
            if (this.content.timeout)
            {
                clearTimeout(this.content.timeout);
                delete this.content.timeout;
                this.content.currentPage = undefined;
            }
            this.content.preview = false;
        }
        this.overTasksList = false;
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
} /*! Desktop !*/

/********************************/
/********** Tasks list **********/
/********************************/{
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
    var height = tasks_list.scrollHeight - this.topHeight;
    
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
    if (this.drag instanceof Task)
        this.drag.updateTasksList(this.mouse.y);
    // Scroll top finished
    if (tasks_list.scrollTop <= 0)
    {
        tasks_list.scrollTop = 0;
        clearInterval(this.scroll.id);
        delete this.scroll;
    }
    // Scroll down finished
    else if (tasks_list.scrollTop >= tasks_list.scrollHeight - this.topHeight)
    {
        tasks_list.scrollTop = tasks_list.scrollHeight - this.topHeight;
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
} /*! Tasks list !*/

/**************************/
/********** Page **********/
/**************************/{
var tmp_page_background = 0;//#######################
// A page is a container that can store multiple tasks.
// Pages are stored in the tasks list of the desktop.
function Page()
{
    this.id = gl_uid++;
    this.order = 1;
    this.number_task = 0;
    var page = this;
    
    // Create the page and insert it in the tasks list
    this.icon = document.createElement("div");
    this.icon.className = "page";
    this.icon.object = this;
    this.icon.style.padding = C.Desktop.tasksListPadding + "px";
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
    // Set the page as the main
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
    this.number_task++;
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
    this.number_task--;
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
        this.tree = new TaskTreeNode({ task : task });
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
            this.tree = new TaskTreeNode({ first : this.tree });
            this.tree.second = new TaskTreeNode({ task : task });
            task.node = this.tree.second;
        }
        // The order changes depending on the position
        else
        {
            this.tree = new TaskTreeNode({ second : this.tree });
            this.tree.first = new TaskTreeNode({ task : task });
            task.node = this.tree.first;
        }
        // Set the ratio depending on the position, and the number of nodes already in that position in the tree
        this.tree.h = -1;
        this.tree.v = -1;
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
    node.h = -1;
    node.v = -1;
    // Depending of the position, one is cut in half
    (position == "w" || position == "e") ? node.h = 0.5 : node.v = 0.5;
    // Build the first and second leaf of the node
    var set = function (first, second)
    {
        node.first = new TaskTreeNode({ task : first });
        first.node = node.first;
        node.second = new TaskTreeNode({ task : second });
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
    if (node[ratio] >= 0 && ((node.first && node.first.task) || (node.second && node.second.task)))
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
    var keep = (parent.first != node) ? parent.first : parent.second;
    // Remove the resize div
    if (node.resize)
        node.resize.parentNode.removeChild(node.resize);
    if (parent.resize)
        parent.resize.parentNode.removeChild(parent.resize);
    // The remaining branch of the parent node become the parent
    keep.parent = undefined;
    if (parent.parent)
    {
        if (parent.parent.first == parent)
            parent.parent.first = keep;
        else
            parent.parent.second = keep;
        keep.parent = parent.parent;
    }
    // Otherwise the branch to keep become the root of the tree
    else
        this.tree = keep;
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
    else if (node.first && node.second)
    {
        var margin = C.Desktop.taskMargin / 2;
        // Creates the resize node that allows to resize the tasks
        if (!node.resize)
        {
            node.resize = document.createElement("div");
            node.resize.className = "resize";
            node.resize.taskNode = node;
            addEvent(node.resize, "mousedown", function(event) { node.mouseDown(event); });
            this.content.appendChild(node.resize);
        }
        // Split vertically
        if (node.h >= 0)
        {
            // Computes the width of the two childs of the current node
            var first = Math.round(width * node.h - margin);
            var second = width - first - C.Desktop.taskMargin;
            var l = left + first + C.Desktop.taskMargin;
            // Ensures that the result is in the range of the node
            if (l < left + C.Desktop.taskMargin)
            {
                l = left + C.Desktop.taskMargin;
                second = width - C.Desktop.taskMargin;
            }
            else if (l > left + width)
            {
                l = left + width;
                first = width - C.Desktop.taskMargin;
            }
            // Render the next two branches
            this.renderTree(node.first, left, top, Math.max(first, 0), height);
            this.renderTree(node.second, l, top, Math.max(second, 0), height);
            // Positions the vertical resize bar
            setClassName(node.resize, "vertical");
            node.resize.style.left = Math.max(l - C.Desktop.taskMargin, left) + "px";
            node.resize.style.top = top + "px";
            node.resize.style.width = Math.min(C.Desktop.taskMargin, width) + "px";
            node.resize.style.height = height + "px";
        }
        // Split horizontally
        else
        {
            // Computes the height of the two childs of the current node
            var first = Math.round(height * node.v - margin);
            var second = height - first - C.Desktop.taskMargin;
            var t = top + first + C.Desktop.taskMargin;
            // Ensures that the result is in the range of the node
            if (t < top + C.Desktop.taskMargin)
            {
                t = top + C.Desktop.taskMargin;
                second = height - C.Desktop.taskMargin;
            }
            else if (t > top + height)
            {
                t = top + height;
                first = height - C.Desktop.taskMargin;
            }
            // Render the next two branches
            this.renderTree(node.first, left, top, width, Math.max(first, 0));
            this.renderTree(node.second, left, t, width, Math.max(second, 0));
            // Positions the horizontal resize bar
            setClassName(node.resize, "horizontal");
            node.resize.style.left = left + "px";
            node.resize.style.top = Math.max(t - C.Desktop.taskMargin, top) + "px";
            node.resize.style.width = width + "px";
            node.resize.style.height = Math.min(C.Desktop.taskMargin, height) + "px";
        }
    }
    // Saves the coordinates of the node
    node.left = left;
    node.top = top;
    node.width = width;
    node.height = height;
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
            preview.style.height = Math.ceil(task.height / 2) + "px";
        if (position == "s")
            preview.style.top = Math.floor(task.height / 2) + "px";
        if (position == "w" || position == "e")
            preview.style.width = Math.ceil(task.width / 2) + "px";
        if (position == "e")
            preview.style.left = Math.floor(task.width / 2) + "px";
    }
    // Display the preview on the page
    else
    {
        // Move the preview to a node in static position
        preview.parentNode.removeChild(preview);
        gl_desktop.node.middle.appendChild(preview);
        // Display the preview
        preview.style.display = "block";
        preview.style.top = this.top + "px";
        preview.style.left = this.left + "px";
        preview.style.width = this.width + "px";
        preview.style.height = this.height + "px";
        // The number of horizontal and vertical branches in the tree helps to compute the size
        var h = (this.countNode(this.tree, "h") + 2);
        var v = (this.countNode(this.tree, "v") + 2);
        // The size of the preview is based on the position of the mouse in the page
        if (position == "n" || position == "s")
            preview.style.height = this.height / v + "px";
        if (position == "s")
            preview.style.top = this.top + this.height * (v - 1) / v + "px";
        if (position == "w" || position == "e")
            preview.style.width = this.width / h + "px";
        if (position == "e")
            preview.style.left = this.left + this.width * (h - 1) / h + "px";
    }
}
} /*! Page !*/

/**************************/
/********** Task **********/
/**************************/{
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
    this.icon.style.width = C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding + "px";
    this.icon.style.height = gl_desktop.taskIconHeight + "px";
    this.icon.innerHTML = tmptoto++;//this.resource.charAt(0).toUpperCase();######################
    addEvent(this.icon, "mousedown", function(event) { task.mouseDown(event); });
    
    // Content
    this.content = document.createElement("div");
    this.content.className = "task";
    this.content.style.display = "none";
    this.content.object = this;
    this.content.innerHTML = content;
    addEvent(this.content, "mousemove", function(event) { if (gl_desktop.drag instanceof Task) task.mouseMove(event); });
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
        this.icon.style.left = (T.Menu.width + C.Desktop.tasksListPadding) + "px";
        // The place of the task icon in the tasks list
        this.place = gl_desktop._getTaskPosition(this.icon);
        // There is a little resistance before the task begins to move
        this.resistance = true;
        // This class name has a hight z-index, in order to always see the icon of the task
        setClassName(this.icon, "drag");
        // Update the size of the new page div, otherwise the tasks list may scroll
        gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + gl_desktop.taskIconHeight + "px";
        // Allows to change the task using the mouse wheel when the mouse is outside the tasks list
        this.moveTaskWheel = mouse.y;
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
    
    // If the preview has already been displayed by Task::mouseMoveTask, we do nothing
    if (gl_desktop.pagePreviewDisplayed)
        return ;
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

// Move the task according to the mouse position
Task.prototype.mouseMoveTask = function (mouse)
{
    var y = mouse.y - this.mouse.y;
    
    // Top limit
    if (y < 0)
        this.icon.style.top = "0px";
    // Bottom limit
    else if (y > gl_desktop.topHeight - gl_desktop.taskIconHeight)
        this.icon.style.top = (gl_desktop.topHeight - gl_desktop.taskIconHeight) + "px";
    // The task has a little resistance before starting the drag
    else if (y >= this.element.y - gl_desktop.node.tasks_list.scrollTop - C.Desktop.taskMagneticDragY
             && y <= this.element.y - gl_desktop.node.tasks_list.scrollTop + C.Desktop.taskMagneticDragY && this.resistance)
        this.icon.style.top = this.element.y - gl_desktop.node.tasks_list.scrollTop + "px";
    // Start the dragging
    else if (this.resistance)
        y = this.startDrag(y);
    // Move the dragged task vertically
    else
        this.icon.style.top = y + "px";
    // Move the dragged task horizontally if the mouse exceeds the magnetic limit
    if (mouse.x > gl_desktop.middleMarginLeft + C.Desktop.taskMagneticDragX)
    {
        this.icon.style.left = (mouse.x + C.Desktop.freeDragTashShift.x) + "px";
        this.icon.style.top = (mouse.y + C.Desktop.freeDragTashShift.y) + "px";
    }
    // Otherwise update the tasks list
    else if (!this.resistance)
    {
        this.icon.style.left = (T.Menu.width + C.Desktop.tasksListPadding) + "px";
        this.updateTasksList(mouse.y);
    }
    // If we are on the tasks list update the mouse y position for the wheel
    if (gl_desktop.overTasksList)
        this.moveTaskWheel = mouse.y;
    // Allows to add a task in the border of the page
    var page = this.icon.parentNode.object;
    gl_desktop.pagePreviewDisplayed = false;
    if (!gl_desktop.drag.taskCache.createPage && gl_desktop.content.page && gl_desktop.content.page.number_task > 1 &&
        mouse.x - page.left > 0 && mouse.x - page.left < page.width && mouse.y - page.top > 0 && mouse.y - page.top < page.height)
    {
        // Get the position of the mouse in the page (n s e w)
        if (mouse.x - page.left < C.Desktop.insertTaskAreaSize)
            gl_desktop.addTaskPosition = "w";
        else if (mouse.x - page.left > page.width - C.Desktop.insertTaskAreaSize)
            gl_desktop.addTaskPosition = "e";
        else if (mouse.y - page.top < C.Desktop.insertTaskAreaSize)
            gl_desktop.addTaskPosition = "n";
        else if (mouse.y - page.top > page.height - C.Desktop.insertTaskAreaSize)
            gl_desktop.addTaskPosition = "s";
        else
            return ;
        var position = gl_desktop.addTaskPosition;
        var preview = gl_desktop.node.preview;
        // Display the preview
        preview.parentNode.removeChild(preview);
        gl_desktop.node.middle.appendChild(preview);
        preview.style.display = "block";
        preview.style.top = page.top + "px";
        preview.style.left = page.left + "px";
        preview.style.width = page.width + "px";
        preview.style.height = page.height + "px";
        if (position == "n" || position == "s")
            preview.style.height = C.Desktop.insertTaskAreaSize + "px";
        if (position == "s")
            preview.style.top = page.top + (page.height - C.Desktop.insertTaskAreaSize) + "px";
        if (position == "w" || position == "e")
            preview.style.width = C.Desktop.insertTaskAreaSize + "px";
        if (position == "e")
            preview.style.left = page.left + (page.width - C.Desktop.insertTaskAreaSize) + "px";
        gl_desktop.addTaskParent = undefined;
        // Ensures that Task::mouseMove doesn't display a preview
        gl_desktop.pagePreviewDisplayed = true;
    }
}

// Place a ghost task under the cursor to show the place where the dragged task will be moved
// @param y : The y position of the mouse
Task.prototype.updateTasksList = function (y)
{
    // Change the reference y depending if the task has been dragged up or down
    y = y - this.mouse.y + gl_desktop.node.tasks_list.scrollTop;
    if (y > this.element.y)
        y += gl_desktop.taskIconHeight;
    gl_desktop._removeGhostTask();
    // Get the position of the task under the cursor
    var data = this.getCursorPosition(y);
    var page = data.page;
    var task = data.task;
    // If we want to create a page, the ghost is not required
    if (this.taskCache.createPage)
    {
        gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + gl_desktop.taskIconHeight + "px";
        gl_desktop.previewNewPage(this);
        return ;
    }
    // Create a ghost task in the current page
    gl_desktop.ghost = document.createElement("div");
    gl_desktop.ghost.className = "ghost";
    gl_desktop.ghost.style.height = gl_desktop.taskIconHeight + "px";
    // The current page is above the original page
    if (data.number_page <= this.place.page)
    {
        if (!task.nextSibling && y > data.position - C.Desktop.tasksListPadding - gl_desktop.taskIconHeight / 2)
            page.appendChild(gl_desktop.ghost);
        else if (task.nextSibling && y > data.position - gl_desktop.taskIconHeight / 2)
            page.insertBefore(gl_desktop.ghost, task.nextSibling);
        else
            page.insertBefore(gl_desktop.ghost, task);
    }
    // The current page is below the original page
    else
    {
        if (!task.nextSibling && y < data.position - C.Desktop.tasksListPadding - gl_desktop.taskIconHeight / 2)
            page.insertBefore(gl_desktop.ghost, task);
        else if (task.nextSibling && y < data.position - gl_desktop.taskIconHeight / 2)
            page.insertBefore(gl_desktop.ghost, task);
        else if (task.nextSibling)
            page.insertBefore(gl_desktop.ghost, task.nextSibling);
        else
            page.appendChild(gl_desktop.ghost);
    }
    // Display the page under which the cursor is
    if (!gl_desktop.content.page || page.object.id != gl_desktop.content.page.id)
    {
        page.object.preview();
        page.object.display();
    }
    gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + "px";
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
        position += C.Desktop.tasksListPadding;
        for (task = page.firstChild, number_task = 0; task && (position < y || !number_task); task = task.nextSibling, number_task++)
            position += gl_desktop.taskIconHeight;
        if (!task)
            position += C.Desktop.tasksListPadding;
    }
    // Get the correct nodes
    page = page.previousSibling;
    (task) ? task = task.previousSibling : task = page.lastChild;
    // Calculate the upper limit
    var limit = position - gl_desktop.taskIconHeight;
    if (!task.previousSibling && !task.nextSibling)
        limit -= C.Desktop.tasksListPadding * 2;
    else if (!task.previousSibling || !task.nextSibling)
        limit -= C.Desktop.tasksListPadding;
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
    gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + "px";
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
        this.mouse.y -= C.Desktop.taskMagneticDragY;
        y += C.Desktop.taskMagneticDragY;
    }
    else if (y > this.element.y - gl_desktop.node.tasks_list.scrollTop)
    {
        this.mouse.y += C.Desktop.taskMagneticDragY;
        y -= C.Desktop.taskMagneticDragY;
    }
    // The mouse is outside the task
    if (this.mouse.y < 0)
    {
        y -= 2 - this.mouse.y;
        this.mouse.y = 2;
    }
    else if (this.mouse.y > gl_desktop.taskIconHeight)
    {
        y += this.mouse.y - gl_desktop.taskIconHeight + 2;
        this.mouse.y = gl_desktop.taskIconHeight - 2;
    }
    // Move the task
    this.icon.style.top = y + "px";
    return (y);
}
} /*! Task !*/

/*********************************/
/********** Resize task **********/
/*********************************/{
// Represents a node of the tasks tree
// @param node : Construct the object easily
function TaskTreeNode(node)
{
    for (var key in node)
        this[key] = node[key];
}

// We clicked on a resize bar. Start to resize the node
TaskTreeNode.prototype.mouseDown = function (event)
{
    var mouse = mouseCoordinates(event);
    var element = elementCoordinates(this.resize);
    
    // The coordinates of the mouse inside the resize bar
    this.mouse = {x : mouse.x - element.x,
                  y : mouse.y - element.y};
    // The coordinates of the resize bar
    this.element = {x : element.x,
                    y : element.y};
    gl_desktop.drag = this;
    disableSelection(false);
    // Search nodes that are in the edge of the current node
    var previous, level;
    for (var node = this.parent, previous = this, level = 1; node; previous = node, node = node.parent, level++)
        if (this.h == -1 && node.h != -1 && node.second == previous && !this.leftNode)
            this.leftNode = { parent : node, h : node.h, width : node.width, level : level };
        else if (this.h == -1 && node.h != -1 && node.first == previous && !this.rightNode)
            this.rightNode = { parent : node, h : node.h, width : node.width, level : level };
        else if (this.v == -1 && node.v != -1 && node.second == previous && !this.topNode)
            this.topNode = { parent : node, v : node.v, height : node.height, level : level };
        else if (this.v == -1 && node.v != -1 && node.first == previous && !this.bottomNode)
            this.bottomNode = { parent : node, v : node.v, height : node.height, level : level };
}

// Resize the node according to the mouse position
TaskTreeNode.prototype.mouseMove = function (x, y)
{
    // Resize horizontally
    if (this.h >= 0)
    {
        // Take into account where the user clicked in the resize bar
        x += C.Desktop.taskMargin / 2 - this.mouse.x;
        // Compute the horizontal resize
        this.h = (x - this.left) / Math.max(this.width, 1);
        this.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, this.h));
        // The number of pixels the mouse moved vertically from the click position
        var diff = y - this.mouse.y - this.element.y;
        // Apply a little resistance before starting to resize vertically
        if (!this.startResize && Math.abs(diff) < C.Desktop.resizeResistance)
            diff = 0;
        else
        {
            // Start to resize vertically
            if (!this.startResize)
                this.startResize = diff;
            (this.startResize > 0) ? diff -= C.Desktop.resizeResistance : diff += C.Desktop.resizeResistance;
        }
        if (this.topNode)
            this.topNode.parent.v = this.topNode.v;
        if (this.bottomNode)
            this.bottomNode.parent.v = this.bottomNode.v;
        // Resize vertically the top node
        if (this.topNode && (!this.bottomNode || diff < 0))
        {
            var v = this.topNode.v + (1 - (this.topNode.height - diff) / Math.max(this.topNode.height, 1));
            this.topNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, v));
            // Adjust the bottom node to the new top node ratio, so that the bottom node doesn't move
            if (this.bottomNode && this.topNode.level > this.bottomNode.level && C.Desktop.stabilizeTaskResize)
            {
                // Old size of the bottom node second, divided by the new size of the bottom node
                var a = 1 - (1 - this.bottomNode.v) / ((1 - this.topNode.parent.v) / (1 - this.topNode.v));
                if (this.topNode.parent.v * this.topNode.height <= C.Desktop.taskMargin / 2)
                    a = 1 - (this.bottomNode.height * (1 - this.bottomNode.v)) / ((this.bottomNode.height / (1 - this.topNode.v)) - C.Desktop.taskMargin / 2);
                this.bottomNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
            }
        }
        // Resize vertically the bottom node
        if (this.bottomNode && (!this.topNode || diff > 0))
        {
            var v = this.bottomNode.v + (1 - (this.bottomNode.height - diff) / Math.max(this.bottomNode.height, 1));
            this.bottomNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, v));
            // Adapt the top node to the new bottom node ratio, so that the top node doesn't move
            if (this.topNode && this.bottomNode.level > this.topNode.level && C.Desktop.stabilizeTaskResize)
            {
                // Old size of the top node first, divided by the new size of the top node
                var a = this.topNode.v / (this.bottomNode.parent.v / this.bottomNode.v);
                if ((1 - this.bottomNode.parent.v) * this.bottomNode.height <= C.Desktop.taskMargin / 2)
                    a = (this.topNode.height * this.topNode.v) / ((this.topNode.height / this.bottomNode.v) - C.Desktop.taskMargin / 2);
                this.topNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
            }
        }
    }
    // Resize vertically
    else
    {
        // Take into account where the user clicked in the resize bar
        y += C.Desktop.taskMargin / 2 - this.mouse.y;
        // Compute the vertical resize
        this.v = (y - this.top) / Math.max(this.height, 1);
        this.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, this.v));
        // The number of pixels the mouse moved horizontally from the click position
        var diff = x - this.mouse.x - this.element.x;
        // Apply a little resistance before starting to resize horizontally
        if (!this.startResize && Math.abs(diff) < C.Desktop.resizeResistance)
            diff = 0;
        else
        {
            // Start to resize horizontally
            if (!this.startResize)
                this.startResize = diff;
            (this.startResize > 0) ? diff -= C.Desktop.resizeResistance : diff += C.Desktop.resizeResistance;
        }
        if (this.leftNode)
            this.leftNode.parent.h = this.leftNode.h;
        if (this.rightNode)
            this.rightNode.parent.h = this.rightNode.h;
        // Resize horitontally the left node
        if (this.leftNode && (!this.rightNode || diff < 0))
        {
            var h = this.leftNode.h + (1 - (this.leftNode.width - diff) / Math.max(this.leftNode.width, 1));
            this.leftNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, h));
            // Adjust the right node to the new left node ratio, so that the right node doesn't move
            if (this.rightNode && this.leftNode.level > this.rightNode.level && C.Desktop.stabilizeTaskResize)
            {
                // Old size of the right node second, divided by the new size of the right node
                var a = 1 - (1 - this.rightNode.h) / ((1 - this.leftNode.parent.h) / (1 - this.leftNode.h));
                if (this.leftNode.parent.h * this.leftNode.width <= C.Desktop.taskMargin / 2)
                    a = 1 - (this.rightNode.width * (1 - this.rightNode.h)) / ((this.rightNode.width / (1 - this.leftNode.h)) - C.Desktop.taskMargin / 2);
                this.rightNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
            }
        }
        // Resize horitontally the right node
        if (this.rightNode && (!this.leftNode || diff > 0))
        {
            var h = this.rightNode.h + (1 - (this.rightNode.width - diff) / Math.max(this.rightNode.width, 1));
            this.rightNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, h));
            // Adapt the left node to the new right node ratio, so that the left node doesn't move
            if (this.leftNode && this.rightNode.level > this.leftNode.level && C.Desktop.stabilizeTaskResize)
            {
                // Old size of the left node first, divided by the new size of the left node
                var a = this.leftNode.h / (this.rightNode.parent.h / this.rightNode.h);
                if ((1 - this.rightNode.parent.h) * this.rightNode.width <= C.Desktop.taskMargin / 2)
                    a = (this.leftNode.width * this.leftNode.h) / ((this.leftNode.width / this.rightNode.h) - C.Desktop.taskMargin / 2);
                this.leftNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
            }
        }
    }
    // Display the modifications
    var page = gl_desktop.content.page;
    if (page)
        page.renderTree(page.tree, page.left, page.top, page.width, page.height);
}

// Resize the tasks margin and the resize bar
TaskTreeNode.prototype.mouseWheel = function (delta)
{
    // Change the margin between the tasks
    C.Desktop.taskMargin += delta;
    C.Desktop.taskMargin = Math.max(C.Desktop.taskMargin, 1);
    // Take into account the position of the mouse in the bar
    if (this.h != -1)
        this.mouse.x = C.Desktop.taskMargin / 2;
    else
        this.mouse.y = C.Desktop.taskMargin / 2;
    // Display the modifications
    var page = gl_desktop.content.page;
    if (page)
        page.renderTree(page.tree, page.left, page.top, page.width, page.height);
}

// Stop the resize
TaskTreeNode.prototype.mouseUp = function ()
{
    delete this.leftNode;
    delete this.rightNode;
    delete this.topNode;
    delete this.bottomNode;
    delete this.startResize;
}
} /*! Resize task !*/

/***************************************/
/********** Resize tasks list **********/
/***************************************/{
function ResizeTasksList()
{
}

// Start to resize the tasks list
ResizeTasksList.prototype.mouseDown = function(event)
{
    var mouse = mouseCoordinates(event);
    var element = elementCoordinates(gl_desktop.node.resize_tasks_list);
    
    // The coordinates of the mouse inside the resize bar
    this.mouse = {x : mouse.x - element.x,
                  y : mouse.y - element.y};
    // Start the resize
    gl_desktop.drag = this;
    disableSelection(false);
}

// Resize the tasks list
ResizeTasksList.prototype.mouseMove = function(mouse)
{
    C.Desktop.tasksListWidth = Math.max(C.Desktop.resizeTasksListMin, Math.min(C.Desktop.resizeTasksListMax, mouse.x - this.mouse.x - T.Menu.width));
    gl_desktop.node.tasks_list.style.width = C.Desktop.tasksListWidth + "px";
    gl_desktop.middleMarginLeft = T.Menu.width + C.Desktop.tasksListWidth + C.Desktop.resizeTasksListWidth;
    gl_desktop.node.middle.style.marginLeft = gl_desktop.middleMarginLeft + "px";
    if (C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding < C.Desktop.taskIconMinWidth)
        C.Desktop.tasksListPadding = Math.round((C.Desktop.tasksListWidth - C.Desktop.taskIconMinWidth) / 2);
    var width = C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding;
    gl_desktop.taskIconHeight = Math.round(width * C.Desktop.taskHeightRatio);
    // Update the tasks list
    for (var page = gl_desktop.node.tasks_list.firstChild; page; page = page.nextSibling)
    {
        page.style.padding = C.Desktop.tasksListPadding + "px";
        for (var task = page.firstChild; task; task = task.nextSibling)
        {
            task.style.width = width + "px";
            task.style.height = gl_desktop.taskIconHeight + "px";
        }
    }
    // Resize the desktop
    gl_desktop.onResize();
}

// Change the padding of the tasks list
ResizeTasksList.prototype.mouseWheel = function (delta)
{
    // Minimum width of a task icon
    if (C.Desktop.tasksListWidth - 2 * (C.Desktop.tasksListPadding + delta) < C.Desktop.taskIconMinWidth)
        return ;
    // Change the padding
    C.Desktop.tasksListPadding = Math.max(1, C.Desktop.tasksListPadding + delta);
    var width = C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding;
    gl_desktop.taskIconHeight = Math.round(width * C.Desktop.taskHeightRatio);
    // Update the tasks list
    for (var page = gl_desktop.node.tasks_list.firstChild; page; page = page.nextSibling)
    {
        page.style.padding = C.Desktop.tasksListPadding + "px";
        for (var task = page.firstChild; task; task = task.nextSibling)
        {
            task.style.width = width + "px";
            task.style.height = gl_desktop.taskIconHeight + "px";
        }
    }
    gl_desktop.onResize();
}

// Resize the tasks list
ResizeTasksList.prototype.mouseUp = function(event)
{
}
} /*! Resize tasks list !*/
