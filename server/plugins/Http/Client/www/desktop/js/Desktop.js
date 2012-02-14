/* This file manages the desktop. */

// The desktop singleton
var gl_desktop;

/********** Desktop **********/{
function Desktop()
{
    // Stores the nodes of the desktop
    this.node = new Object();
    this.node.desktop = document.getElementById("desktop");
    this.node.top = getElementsByClassName("top", this.node.desktop, true);
    this.node.menu = getElementsByClassName("menu", this.node.top, true);
    this.node.tasks_list = removeTextNodes(getElementsByClassName("tasks_list", this.node.top, true));
    this.node.resize_tasks_list = getElementsByClassName("resize_tasks_list", this.node.top, true);
    this.node.middle = getElementsByClassName("middle", this.node.top, true);
    this.node.player_document = getElementsByClassName("player_document", this.node.middle, true);
    this.node.pages = getElementsByClassName("pages", this.node.middle, true);
    this.node.tasks = getElementsByClassName("tasks", this.node.middle, true);
    this.node.preview = getElementsByClassName("preview", this.node.middle, true);
    this.node.player_media = getElementsByClassName("player_media", this.node.desktop, true);
    // Sets the default values
    this.node.desktop.style.minHeight = C.Desktop.minHeight + "px";
    this.node.desktop.style.minWidth = C.Desktop.minWidth + "px";
    this.node.player_document.style.height = C.Desktop.playerDocumentHeight + "px";
    this.node.player_media.style.height = C.Desktop.playerMediaHeight + "px";
    this.node.menu.style.width = T.Menu.width + "px";
    this.node.tasks_list.style.width = C.Desktop.tasksListWidth + "px";
    this.node.resize_tasks_list.style.width = C.Desktop.resizeTasksListWidth + "px";
    this.updateMiddleMarginLeft();
    this.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + "px";
    this.setTaskIconHeight((C.Desktop.tasksListWidth - C.Desktop.tasksListPadding * 2) * C.Desktop.taskHeightRatio);
    // Members
    gl_desktop = this;
    // Allows to drag elements
    this.drag = new Drag();
    // Manages the tasks list
    this.tasksList = new TasksList();
    // Manages the buttons on the tasks of the tasks list
    this.taskButtons = new TaskButtons();
    // True if the mouse is over the tasks list
    this.overTasksList = undefined;
    // The height of the "top" node of the desktop
    this.topHeight = undefined;
    // The position of the mouse in the desktop
    this.mouse = undefined;
    // The size of the menu + the tasks list + its resize bar
    this.middleMarginLeft;
    // The height of the tasks icons
    this.taskIconHeight;
    // The coordinates of the main area of the desktop
    this.left = undefined;
    this.top = undefined;
    this.width = undefined;
    this.height = undefined;
    // The page currently displayed by the desktop
    this.currentPage = undefined;
    // The main page is automatically displayed when the mouse move out of the tasks list
    this.mainPage = undefined;
    // When the mouse is over the tasks list for a short time, we don't display the page under it immediatly
    this.delayedPage = undefined;
    // The timeout used to delay the display of a page
    this.delayTimeout = undefined;
    // If the page display is delayed
    this.isDelayed = undefined;
    // Helps to normalize the mouse wheel delta
    this.baseWheelDelta = 2000;
    // Events
    var mouseWheelEvent = (/Firefox/i.test(navigator.userAgent)) ? "DOMMouseScroll" : "mousewheel";
    addEvent(document.body, mouseWheelEvent, function (event) { gl_desktop.mouseWheel(event); });
    addEvent(document.body, "mousemove", function (event) { gl_desktop.mouseMove(event); });
    addEvent(document.body, "mouseup", function (event) { gl_desktop.mouseUp(event); });
}

// Opens a new task in a new page.
// @param resource : The name of the resource that will be loaded in the task.
// @param event : The event that triggered the call.
Desktop.prototype.open = function (resource, event)
{
	if (getButton(event) != 0)
		return ;
    // Loads the task from the resources
	gl_resources.load(resource, function (content)
    {
        // Creates the page and the task
        var page = new Page();
        var task = new Task(resource, content);
        page.addTask(task);
        hideAllWindows();
        page.display(true);
        gl_resources.callJs(resource, task);
        // Scrolls to the bottom of the tasks list
        gl_desktop.node.tasks_list.scrollTop = gl_desktop.node.tasks_list.scrollHeight - gl_desktop.topHeight - C.Desktop.newTaskHeight;
    });
}

// Resizes the desktop.
Desktop.prototype.onResize = function ()
{
    var width = (gl_browserSize.width > C.Desktop.minWidth) ? gl_browserSize.width : C.Desktop.minWidth;
    var height = (gl_browserSize.height > C.Desktop.minHeight) ? gl_browserSize.height : C.Desktop.minHeight;
    this.topHeight = height - C.Desktop.playerMediaHeight;
    this.node.top.style.height = this.topHeight + "px";
    this.left = this.middleMarginLeft;
    this.top = C.Desktop.playerDocumentHeight;
    this.width = width - this.middleMarginLeft;
    this.height = this.topHeight - C.Desktop.playerDocumentHeight;
    // Resizes the page currently displayed
    if (this.getCurrentPage())
        this.getCurrentPage().onResize();
}

// Called each time the mouse move.
// @param event : The event that triggered the call.
Desktop.prototype.mouseMove = function (event)
{
	this.mouse = mouseCoordinates(event);
    // If we are on the tasks list
    if (this.mouse.x < this.middleMarginLeft && this.mouse.x > T.Menu.width && this.mouse.y < this.topHeight)
        this.mouseOverTasksList(this.mouse);
    else
        this.mouseOutTasksList();
    this.drag.mouseMove(event);
    moveWindow(event);
}

// Called when the mouse is up.
// @param event : The event that triggered the call.
Desktop.prototype.mouseUp = function (event)
{
    this.drag.mouseUp(event);
    stopWindow(event);
}

// Called each time the mouse wheel is used on the tasks list.
Desktop.prototype.mouseWheel = function (event)
{
    // Normalizes the delta (should be improved)
    var delta = event.detail ? -event.detail : event.wheelDelta;
    if (Math.abs(delta) < this.baseWheelDelta)
        this.baseWheelDelta = Math.abs(delta);
    delta = Math.round(delta / this.baseWheelDelta * C.Desktop.mouseWheelMultiplier);
    // Scrolls the tasks list if we are on it
    if (this.overTasksList)
    {
        this.node.tasks_list.scrollTop -= delta * C.Desktop.mouseWheelScrollSpeed;
        this.taskButtons.hideButtons();
    }
    this.drag.mouseWheel(delta);
}

// The mouse is over the task list.
Desktop.prototype.mouseOverTasksList = function (mouse)
{
    // If the mouse just came on the tasks list
    if (!this.overTasksList)
    {
        this.overTasksList = true;
        /*// We will wait pagePreviewDelay ms before displaying the page undes the cursor
        this.delayTimeout = setTimeout("gl_desktop.mouseOverTasksListTimeout()", C.Desktop.pagePreviewDelay);
        this.isDelayed = false;
        // Ensures that the preview is correctly displayed
        if (this.drag.isDragging("Task") && this.getCurrentPage())
            this.getCurrentPage().preview();*/
    }
    // The top scrolling area of the tasks list
    if (mouse.y <= C.Desktop.tasksListScrollHeight)
        this.tasksList.scroll(1 - (mouse.y / C.Desktop.tasksListScrollHeight));
    // The bottom scrolling area of the tasks list
    else if (mouse.y > this.topHeight - C.Desktop.tasksListScrollHeight && mouse.y <= this.topHeight)
        this.tasksList.scroll((this.topHeight - C.Desktop.tasksListScrollHeight - mouse.y) / C.Desktop.tasksListScrollHeight);
    // Ensures that the scrolling is stopped outside of the scrolling areas
    else
        this.tasksList.stopScroll();
}
/*Desktop.prototype.mouseOverTasksListTimeout = function ()
{
    delete this.delayTimeout;
    if (!this.isDisplayed(this.delayedPage))
        this.display(this.delayedPage);
    this.delayedPage = undefined;
    this.isDelayed = true;
}*/

// The mouse is out the tasks list.
Desktop.prototype.mouseOutTasksList = function ()
{
    if (this.overTasksList)
    {
        this.overTasksList = false;
        /*if (!this.drag.isDragging())
        {
            // Displays the main page
            if (!this.isDisplayed(this.mainPage))
                this.display(this.mainPage);
            // No preview is going to be displayed since we are out of the tasks list
            if (this.delayTimeout)
            {
                clearTimeout(this.delayTimeout);
                delete this.delayTimeout;
                this.delayedPage = undefined;
            }
            this.isDelayed = false;
        }*/
        this.tasksList.stopScroll();
    }
}

// Changes the page displayed by the desktop.
// @param page : The page to display.
// @param main : If the page displayed should be kept as the main page.
Desktop.prototype.display = function(page, main)
{
    if (!page)
        return ;
    // Sets the page as the main
    /*if (main)
        this.mainPage = page;*/
    // Hides the old page
    this.hide();
    // Displays the new page
    this.currentPage = page;
    page._display();
}

// If the current page is undefined, this method display the next page available in the desktop.
// @param page : If defined, it will search from this page.
Desktop.prototype.displayNext = function(page)
{
    var ignore = undefined;
    
    if (this.currentPage)
        return ;
    if (!page && this.node.tasks_list.firstChild.object)
        page = this.node.tasks_list.firstChild.object;
    else
        ignore = page.icon;
    // Searches the next page to display
    var found = false;
    for (var p = page.icon; p && !found; p = p.nextSibling)
        if (p != ignore && getClassName(p, "page") && p.object.getContainer() instanceof Desktop)
        {
            p.object.display(!this.mainPage);
            found = true;
        }
    // Otherwise searches the previous page
    for (var p = page.icon; p && !found; p = p.previousSibling)
        if (p != ignore && getClassName(p, "page") && p.object.getContainer() instanceof Desktop)
        {
            p.object.display(!this.mainPage);
            found = true;
        }
}

// Displays the page later if the mouse is on the tasks list for too little time.
/*Desktop.prototype.delayedDisplay = function(page)
{
    // The display of the page is delayed
    if (!this.isDelayed)
        this.delayedPage = page;
    else if (!this.isDisplayed(page))
        this.display(page);
}*/

// Hides the page currently displayed by the desktop.
Desktop.prototype.hide = function ()
{
    if (this.getCurrentPage())
    {
        var page = this.getCurrentPage();
        this.currentPage = undefined;
        page.hide();
    }
}

// Notifies the desktop that a page has been closed.
// Cleans its references and displays another page if necessary.
// @parem page : The closed page.
Desktop.prototype.close = function (page)
{
    var old = this.currentPage;
    // Removes the page of the main area
    if (this.currentPage && this.currentPage == page)
        this.currentPage = undefined;
    /*if (this.mainPage && this.mainPage == page)
        this.mainPage = undefined;
    if (this.delayedPage && this.delayedPage == page)
        this.delayedPage = undefined;*/
    /*if (page.icon.nextSibling && getClassName(page.icon.nextSibling, "page"))
        this.display(page.icon.nextSibling.object, !this.mainPage);
    else if (this.mainPage)
        this.display(this.mainPage);
    else if (page.icon.previousSibling)
        this.display(page.icon.previousSibling.object, page);*/
    // If a page was displayed, we replace it
    if (old)
        this.displayNext(page);
}

// Updates the width of the margin left of the middle div.
Desktop.prototype.updateMiddleMarginLeft = function ()
{
    this.middleMarginLeft = T.Menu.width + C.Desktop.tasksListWidth + C.Desktop.resizeTasksListWidth;
    this.node.middle.style.marginLeft = this.middleMarginLeft + "px";
}

// Returns the page currently displayed by the desktop. If no page is displayed, nothing is ruturned.
Desktop.prototype.getCurrentPage = function () { return (this.currentPage); }
// Returns true if the page in parameter is currently displayed by the desktop.
Desktop.prototype.isDisplayed = function (page) { return (page && this.currentPage && this.currentPage == page); }
// Returns true if the page in parameter is the main page
/*Desktop.prototype.isMainPage = function (page) { return (page && this.mainPage && this.mainPage == page); }*/
Desktop.prototype.setTaskIconHeight = function (height) { this.taskIconHeight = Math.round(height); }
// Returns true if the mouse is over the tasks list.
Desktop.prototype.isOverTasksList = function () { return (this.overTasksList); }
// Puts the focus on the desktop, i.e hides all the windows.
Desktop.prototype.focus = function () { hideAllWindows(); }
// Sets the desktop as the container of the page.
Desktop.prototype.setPage = function (page) {
    page.setContainer(gl_desktop);
    page.setZIndex(0);
    this.display(page, true);
}

} /*! Desktop !*/

/********** Page **********/{
// A page is a container that can store multiple tasks.
function Page()
{
    // The number of tasks in the page
    this.numberTasks = 0;
    // The icon of the page in the tasks list
    this.icon = undefined;
    // The content of the page. Stores some element of the page, like the resize bars.
    this.content = undefined;
    // The coordinates of the page
    this.left = undefined;
    this.top = undefined;
    this.width = undefined;
    this.height = undefined;
    // The tree allows to render the tasks of the page. It consists of TaskTreeNode objects.
    this.tree = undefined;
    // The container that displays the page.
    this.container = gl_desktop;
    var page = this;
    
    // Creates the icon and insert it in the tasks list
    this.icon = document.createElement("div");
    this.icon.className = "page";
    this.icon.object = this;
    this.icon.style.padding = C.Desktop.tasksListPadding + "px";
    gl_desktop.node.tasks_list.insertBefore(this.icon, gl_desktop.node.tasks_list.lastChild);
    addEvent(this.icon, "mousedown", function (event) { page.mouseDown(event); });
    addEvent(this.icon, "mouseover", function (event) { page.mouseOver(event); });
    this.updateOddEven();
    
    // Creates the content
    this.content = document.createElement("div");
    this.content.className = "page";
    this.content.style.display = "none";
    this.content.object = this;
    // Puts the focus on the page when we click on the content (the resize bars)
    addEvent(this.content, "mousedown", function (event) { page.mouseDownContent(event); });
    gl_desktop.node.pages.appendChild(this.content);
}

// The mouse goes down on a page icon.
Page.prototype.mouseDown = function (event)
{
    // The mouse have to be directly on the page icon
    if (!getClassName((event.target || event.srcElement), "page"))
        return ;
    // Sets the page as the main
    if (getButton(event) == 0/* && !gl_desktop.isMainPage(this)*/)
        this.display(true);
    // Closes the page
    else if (getButton(event) == 1)
        this.close();
}

// Displays the page when the mouse is over it.
Page.prototype.mouseOver = function (event)
{
    // If we are dragging something, do nothing
    /*if (gl_desktop.drag.isDragging())
        return ;
    gl_desktop.delayedDisplay(this);*/
}

// Puts the focus on the page when we click on the content (the resize bars)
Page.prototype.mouseDownContent = function ()
{
    this.container.focus();
}

// Asks the container of the page to display it.
// @param main : If the page is the main.
Page.prototype.display = function (main)
{
    this.container.display(this, main);
}

// Displays the page and its tasks.
Page.prototype._display = function ()
{
    // Updates the coordinates
    this.onResize();
    // Display the page
    this.content.style.display = "block";
    // Displays the tasks of the page
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        if (getClassName(task, "task"))
            task.object.display();
}

// Hides the page and its tasks.
Page.prototype.hide = function ()
{
    this.content.style.display = "none";
    // Hides the tasks of the page.
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        if (getClassName(task, "task"))
            task.object.hide();
}

// Closes the page.
Page.prototype.close = function ()
{
    // Notifies the desktop that the page has been closed
    this.container.close(this);
    // Removes the tasks of the page
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        if (getClassName(task, "task"))
            task.object.close();
    // Removes the page nodes
    this.icon.parentNode.removeChild(this.icon);
    this.content.parentNode.removeChild(this.content);
    this.updateOddEven();
}

// Adds a task in the page.
// @param task : The task to add.
// @param position : The position of the new task in the page or the parent (n s e w).
// @param parent : If defined, the parent task is cut in half and the task take a half.
// @param beforeIcon : The icon before which the task will be added in the tasks list.
Page.prototype.addTask = function (task, position, parent, beforeIcon)
{
    this.addNode(task, position, parent);
    task.content.style.zIndex = this.content.style.zIndex;
    // Puts the new task in the tasks list
    if (beforeIcon)
        beforeIcon.parentNode.insertBefore(task.icon, beforeIcon);
    else
        this.icon.appendChild(task.icon);
    this.numberTasks++;
}

// Removes a task from the page.
// @param task : The task to remove.
// @param move : If the task is moved its content is not removed.
Page.prototype.removeTask = function (task, move)
{
    task.icon.parentNode.removeChild(task.icon);
    if (!move)
        task.close();
    this.numberTasks--;
    // If the page is empty we close it
    if (this.numberTasks == 0)
        this.close();
    // Otherwise we resize its tasks
    else
    {
        this.removeNode(task.node);
        this.onResize();
    }
}

// Resizes the tasks of the page.
Page.prototype.onResize = function ()
{
    this.left = this.container.left;
    this.top = this.container.top;
    this.width = this.container.width;
    this.height = this.container.height;
    // Renders the tasks tree
    this.renderTree();
}

// When a task is dragged over a page or an other task, this method displays a preview of its future position.
// @param task : The task on which the preview is applied. If undefined, the preview is shown on the page.
// @param position : The future position of the task (n s e w).
Page.prototype.preview = function (task, position)
{
    var drag = gl_desktop.drag.getObject();
    
    // We don't display a preview when we are on a new page
    if (drag.isOnNewPage())
        return ;
    // Hides the content of the dragged task if we are not on its page
    if (drag.getPage() != this && this.container instanceof Desktop && drag.getPage() instanceof Desktop)
        drag.hide();
    // The default position
    if (!position)
        position = C.Desktop.defaultPosition;
    var preview = gl_desktop.node.preview;
    // No preview is needed if the task or the page are the same
    if ((task && drag == task) || (!task && drag.getPage() == this))
        preview.style.display = "none";
    // Displays the preview on a task
    else if (task)
    {
        // Moves the preview in the task in order to receive the mousemove events
        preview.parentNode.removeChild(preview);
        task.content.appendChild(preview);
        // Displays the preview based on the position of the mouse in the task
        preview.style.top = 0 + "px";
        preview.style.left = 0 + "px";
        preview.style.width = task.width + "px";
        preview.style.height = task.height + "px";
        preview.style.zIndex = task.content.style.zIndex;
        preview.style.display = "block";
        if (position == "n" || position == "s")
            preview.style.height = Math.ceil(task.height / 2) + "px";
        if (position == "s")
            preview.style.top = Math.floor(task.height / 2) + "px";
        if (position == "w" || position == "e")
            preview.style.width = Math.ceil(task.width / 2) + "px";
        if (position == "e")
            preview.style.left = Math.floor(task.width / 2) + "px";
    }
    // Displays the preview on the page
    else
    {
        // Moves the preview to a node in static position
        preview.parentNode.removeChild(preview);
        gl_desktop.node.middle.appendChild(preview);
        // Displays the preview
        preview.style.top = this.top + "px";
        preview.style.left = this.left + "px";
        preview.style.width = this.width + "px";
        preview.style.height = this.height + "px";
        preview.style.zIndex = this.content.style.zIndex;
        preview.style.display = "block";
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

// Adds a node to the tree that represents the hierarchy of the tasks in the page.
// @param task : The task of the node.
// @param position : The position of the task relative to the parent (n s e w).
// @param parent : If defined, the parent is cut in half and both of the tasks takes one part.
Page.prototype.addNode = function (task, position, parent)
{
    // The default position
    if (!position)
        position = C.Desktop.defaultPosition;
    // Creates the tree
    if (!this.tree)
    {
        this.tree = new TaskTreeNode({ task : task });
        task.node = this.tree;
    }
    // There is only one node in the tree
    else if (!parent && this.tree.task)
        this._addNode(task, position, this.tree);
    // Adds a node at the root of the tree
    else if (!parent)
    {
        // Changes the root of the tree (the old tree becomes a leaf)
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
        // Sets the ratio depending on the position, and the number of nodes already in that position in the tree
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
        // Sets the parents of the leafs
        this.tree.first.parent = this.tree;
        this.tree.second.parent = this.tree;
    }
    // Adds a node after the parent
    else
        this._addNode(task, position, parent.node);
}
// Helper method of Page.addNode.
Page.prototype._addNode = function (task, position, node)
{
    // The horizontal and vertical ratios
    node.h = -1;
    node.v = -1;
    // Depending of the position, one is cut in half
    (position == "w" || position == "e") ? node.h = 0.5 : node.v = 0.5;
    // Builds the first and second leaf of the node
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
    // Sets the parent node of the leafs
    node.first.parent = node;
    node.second.parent = node;
    // The node is no longer a leaf
    node.task = undefined;
}

// Counts recursively the number of leafs that are horizontal or vertical.
// @param node : The current node of the tree.
// @param ratio : "h" or "v".
Page.prototype.countNode = function (node, ratio)
{
    var result = 0;
    
    // If the node has one leaf and the good ratio
    if (node[ratio] >= 0 && ((node.first && node.first.task) || (node.second && node.second.task)))
        result = 1;
    // Continues the counting on the first branch
    if (node.first)
        result += this.countNode(node.first, ratio);
    // Same on the second branch
    if (node.first)
        result += this.countNode(node.second, ratio);
    return (result);
}

// Removes a node from the tasks tree.
// @param node : The node to remove.
Page.prototype.removeNode = function (node)
{
    var parent = node.parent;
    // Gets the branch to keep
    var keep = (parent.first != node) ? parent.first : parent.second;
    // Removes the resize div
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

// Displays the tasks tree recursively.
// @param node : The current node.
// @param left, top, width, height : The remaining area to be filled by the tasks.
Page.prototype.renderTree = function (node, left, top, width, height)
{
    if (!this.tree)
        return ;
    // If all the parameters are empty, we are at the root of the tree
    if (!node && !left && !top && !width && !height)
    {
        node = this.tree;
        left = this.left;
        top = this.top;
        width = this.width;
        height = this.height;
    }
    // The node is a leaf, so we display its task
    if (node.task)
        node.task.setCoordinates(left, top, width, height);
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
            addEvent(node.resize, "mousedown", function (event) { node.mouseDown(event); });
            this.content.appendChild(node.resize);
        }
        // Splits vertically
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
            // Renders the next two branches
            this.renderTree(node.first, left, top, Math.max(first, 0), height);
            this.renderTree(node.second, l, top, Math.max(second, 0), height);
            // Positions the vertical resize bar
            setClassName(node.resize, "vertical");
            node.resize.style.left = Math.max(l - C.Desktop.taskMargin, left) + "px";
            node.resize.style.top = top + "px";
            node.resize.style.width = Math.min(C.Desktop.taskMargin, width) + "px";
            node.resize.style.height = height + "px";
        }
        // Splits horizontally
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

// Updates the odd/even class name of the pages icons.
Page.prototype.updateOddEven = function ()
{
    var i;

    for (var page = gl_desktop.node.tasks_list.firstChild, i = 1; page; page = page.nextSibling, i++)
        if (getClassName(page, "page"))
        {
            removeClassName(page, "odd");
            removeClassName(page, "even");
            if (i % 2)
                setClassName(page, "odd");
            else
                setClassName(page, "even");
        }
}

// Returns the number of tasks in the page.
Page.prototype.getNumberTasks = function () { return (this.numberTasks); }
Page.prototype.getContainer = function () { return (this.container); }
Page.prototype.setContainer = function (container) { this.container = container; }
Page.prototype.setZIndex = function (zIndex) {
    this.content.style.zIndex = zIndex;
    for (var task = this.icon.firstChild; task; task = task.nextSibling)
        if (getClassName(task, "task"))
            task.object.content.style.zIndex = zIndex;
}

} /*! Page !*/

/********** Task **********/{
var tmptoto = 1;
// A task holds a resource and is stored in a page.
// @param resource : The name of the resource loaded in the task.
// @param content : The HTML content of the resource.
function Task(resource, content)
{
    // The icon element of the task in the tasks list, inside the icon of the page of the task
    this.icon = undefined;
    // The content of the task, which stores its resource
    this.content = undefined;
    // The coordinates of the content of the task
    this.left = undefined;
    this.top = undefined;
    this.width = undefined;
    this.height = undefined;
    // The following members are used when the task icon is being dragged
    // The position of the mouse in the element {x, y}
    this.mouse = undefined;
    // The initial position of the element {x, y}
    this.element = undefined;
    // The position of the task's page in tha tasks list
    this.page = undefined;
    // Applies a resistance before starting to drag a task icon
    this.resistance = undefined;
    // Keeps various data in a cache so that we dont have to calculate them each time the cursor moves
    // {position, limit, number_page, number_task, page, task, createPage}
    this.taskCache = undefined;
    // Allows to change the page using the mouse wheel when the mouse is outside the tasks list and a task is being dragged.
    // Contains the current y position of the ghost in the tasks list.
    this.movePageWheel = undefined;
    // True if the preview in the border of a page has already been displayed.
    // Prevents the preview to be displayed inside a task in this case.
    this.pagePreviewDisplayed = undefined;
    // If defined, this is the task that will be splitted to host the dragged task.
    // Otherwise the dragged task will be inserted in the border of the page.
    this.addTaskParent;
    // The position (n s e w) in which the task being dragged will be placed in the parent
    this.addTaskPosition;
    // The ghost is an empty div with the same size as a task that allows to show where the dragged task will be landed when the mouse up.
    this.ghost = undefined;
    // The last page on which the preview has been displayed. Ensures that we don't display the preview on the same page.
    this.lastPagePreview = undefined;
    // The page displayed on the desktop when we start to drag the task icon.
    this.originalDesktopPage = undefined;
    var task = this;
    
    // Creates the tasks list icon
    this.icon = document.createElement("div");
    this.icon.className = "task";
    this.icon.object = this;
    this.icon.style.width = C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding + "px";
    this.icon.style.height = gl_desktop.taskIconHeight + "px";
    this.icon.innerHTML = tmptoto++;//resource.charAt(0).toUpperCase();######################
    // Adds the buttons system on the task icon
    gl_desktop.taskButtons.addTask(this);
    // This event start to move the task icon or close it
    addEvent(this.icon, "mousedown", (this.mouseDownEvent = function (event) { task.mouseDown(event); }));
    
    // Creates the content of the task
    this.content = document.createElement("div");
    this.content.className = "task";
    this.content.style.display = "none";
    this.content.object = this;
    this.content.innerHTML = content;
    // This event displays a preview of the position of a task dragged over the content of the current task
    addEvent(this.content, "mousemove", (this.mouseMoveEvent = function (event) { if (gl_desktop.drag.isDragging("Task")) task.mouseMovePreview(event); }));
    // Puts the focus on the page of the task
    addEvent(this.content, "mousedown", (this.mouseDownContentEvent = function (event) { task.mouseDownContent(event); }));
    gl_desktop.node.tasks.appendChild(this.content);
}

// Starts to move the task icon or close it, depending on the mouse button pressed.
Task.prototype.mouseDown = function (event)
{
    var mouse = mouseCoordinates(event);
    
    // Ensures that we didn't click on a task icon button
    if (getEventTarget(event, "task_buttons", 5))
        return ;
    // Starts to drag the task icon
    if (getButton(event) == 0)
    {
        // Displays the page if it is hidden
        if (!this.getPage().getContainer().isDisplayed() || this.getPage().getContainer() instanceof Window)
            this.getPage().getContainer().display(this.getPage(), true);
        gl_desktop.drag.start(event, this.icon, this, "mouseMoveTask", "mouseWheel", "mouseUp");
        this.mouse = gl_desktop.drag.getMouse();
        this.mouse.y += gl_desktop.node.tasks_list.scrollTop;
        this.element = gl_desktop.drag.getElement();
        // Moves the task icon to the position of the mouse
        this.icon.style.position = "absolute";
        this.icon.style.top = mouse.y - this.mouse.y + "px";
        this.icon.style.left = (T.Menu.width + C.Desktop.tasksListPadding) + "px";
        // Gets the position of the task's page in the tasks list
        this.page = 0;
        for (var page = this.icon.parentNode; page; page = page.previousSibling)
            this.page++;
        // There is a little resistance before the task begins to move
        this.resistance = true;
        // This class name has a hight z-index, in order to always see the icon of the task
        setClassName(this.icon, "drag");
        // Updates the size of the new page div, otherwise the tasks list may scroll
        gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + gl_desktop.taskIconHeight + "px";
        this.lastPagePreview = undefined;
        // If there is several tasks in the page and it is on the desktop, hides all the windows
        if (this.getPage().getNumberTasks() > 1 && this.getPage().getContainer() instanceof Desktop)
            hideAllWindows();
        // Saves the windows states in order to restore them if necessary
        saveWindows();
        this.originalDesktopPage = gl_desktop.getCurrentPage();
        this.movePageWheel = mouse.y;
        this.updateTasksList(mouse.y);
    }
    // Closes the task
    else if (getButton(event) == 1)
        this.getPage().removeTask(this);
}

// The mouse moved over the content of a task, while a task icon is being dragged.
// Displays a preview of the future position of the dragged task in the content.
Task.prototype.mouseMovePreview = function (event)
{
    var mouse = mouseCoordinates(event);
    var x = mouse.x - this.left;
    var y = mouse.y - this.top;
    var ratio = this.width / this.height;
    var task = gl_desktop.drag.getObject();
    var oldPosition = task.addTaskPosition;
    var oldParent = task.addTaskParent;
    
    // If the preview has already been displayed by Task::mouseMoveTask, we do nothing
    if (task.pagePreviewDisplayed)
        return ;
    // In the corner of the task we use the diagonal to get the position
    if (!task.addTaskPosition || y < this.height / 3 && x < this.width / 3 || y < this.height / 3 && x > this.width * 2 / 3
                                    || y > this.height * 2 / 3 && x < this.width / 3 || y > this.height * 2 / 3 && x > this.width * 2 / 3)
    {
        if (x > y * ratio && (this.width - x) > y * ratio)
            task.addTaskPosition = "n";
        else if (x > y * ratio)
            task.addTaskPosition = "e";
        else if ((this.width - x) > y * ratio)
            task.addTaskPosition = "w";
        else
            task.addTaskPosition = "s";
    }
    // Facilitates the transition east/west
    else if (task.addTaskPosition == "e" || task.addTaskPosition == "w")
    {
        if (y >= this.height / 3 && y <= this.height * 2 / 3 && x <= this.width / 2)
            task.addTaskPosition = "w";
        else if (y >= this.height / 3 && y <= this.height * 2 / 3 && x >= this.width / 2)
            task.addTaskPosition = "e";
        else if (y < this.height / 2)
            task.addTaskPosition = "n";
        else
            task.addTaskPosition = "s";
    }
    // Facilitates the transition north/south
    else
    {
        if (x >= this.width / 3 && x <= this.width * 2 / 3 && y <= this.height / 2)
            task.addTaskPosition = "n";
        else if (x >= this.width / 3 && x <= this.width * 2 / 3 && y >= this.height / 2)
            task.addTaskPosition = "s";
        else if (x < this.width / 2)
            task.addTaskPosition = "w";
        else
            task.addTaskPosition = "e";
    }
    task.addTaskParent = this;
    // If the position has changed, the preview is displayed
    if (oldPosition != task.addTaskPosition || oldParent != task.addTaskParent)
        this.getPage().preview(task.addTaskParent, task.addTaskPosition);
}

// Moves the task according to the mouse position.
Task.prototype.mouseMoveTask = function (event)
{
    var mouse = mouseCoordinates(event);
    var y = mouse.y - this.mouse.y;
    
    // Top limit
    if (y < 0)
        this.icon.style.top = "0px";
    // Bottom limit
    else if (y > gl_desktop.topHeight - gl_desktop.taskIconHeight)
        this.icon.style.top = (gl_desktop.topHeight - gl_desktop.taskIconHeight) + "px";
    // The task has a little resistance before starting the drag
    else if (y >= this.element.y - gl_desktop.node.tasks_list.scrollTop - C.Desktop.taskResistance
             && y <= this.element.y - gl_desktop.node.tasks_list.scrollTop + C.Desktop.taskResistance && this.resistance)
        this.icon.style.top = this.element.y - gl_desktop.node.tasks_list.scrollTop + "px";
    // Starts the dragging
    else if (this.resistance)
        y = this.startDrag(y);
    // Moves the dragged task vertically
    else
        this.icon.style.top = y + "px";
    // Moves the dragged task horizontally if the mouse is outside of the tasks list
    if (mouse.x >= gl_desktop.middleMarginLeft)
    {
        this.icon.style.left = (mouse.x + C.Desktop.freeDragTashShift.x) + "px";
        this.icon.style.top = Math.min(gl_desktop.topHeight - gl_desktop.taskIconHeight, Math.max(C.Desktop.playerDocumentHeight, mouse.y + C.Desktop.freeDragTashShift.y)) + "px";
    }
    // Otherwise updates the tasks list
    else
    {
        this.icon.style.left = (T.Menu.width + C.Desktop.tasksListPadding) + "px";
        if (!this.resistance)
            this.updateTasksList(mouse.y);
    }
    // These variables must be undefined in the tasks list
    if (gl_desktop.isOverTasksList() && (this.addTaskParent || this.addTaskPosition))
    {
        this.addTaskPosition = undefined;
        this.addTaskParent = undefined;
    }
    // If we are on the tasks list update the mouse y position for the wheel
    if (gl_desktop.isOverTasksList())
        this.movePageWheel = mouse.y;
    // Allows to add a task in the border of the page
    var page = this.taskCache.page.object;
    this.pagePreviewDisplayed = false;
    if (!this.taskCache.createPage && page.getNumberTasks() > 1 && mouse.x - page.left > 0
        && mouse.x - page.left < page.width && mouse.y - page.top > 0 && mouse.y - page.top < page.height)
    {
        // Gets the position of the mouse in the page (n s e w)
        if (mouse.x - page.left < C.Desktop.insertTaskAreaSize)
            this.addTaskPosition = "w";
        else if (mouse.x - page.left > page.width - C.Desktop.insertTaskAreaSize)
            this.addTaskPosition = "e";
        else if (mouse.y - page.top < C.Desktop.insertTaskAreaSize)
            this.addTaskPosition = "n";
        else if (mouse.y - page.top > page.height - C.Desktop.insertTaskAreaSize)
            this.addTaskPosition = "s";
        else
            return ;
        var position = this.addTaskPosition;
        var preview = gl_desktop.node.preview;
        // Displays the preview
        preview.parentNode.removeChild(preview);
        gl_desktop.node.middle.appendChild(preview);
        preview.style.top = page.top + "px";
        preview.style.left = page.left + "px";
        preview.style.width = page.width + "px";
        preview.style.height = page.height + "px";
        preview.style.zIndex = page.content.style.zIndex;
        preview.style.display = "block";
        if (position == "n" || position == "s")
            preview.style.height = C.Desktop.insertTaskAreaSize + "px";
        if (position == "s")
            preview.style.top = page.top + (page.height - C.Desktop.insertTaskAreaSize) + "px";
        if (position == "w" || position == "e")
            preview.style.width = C.Desktop.insertTaskAreaSize + "px";
        if (position == "e")
            preview.style.left = page.left + (page.width - C.Desktop.insertTaskAreaSize) + "px";
        this.addTaskParent = undefined;
        // Ensures that Task::mouseMove doesn't display a preview
        this.pagePreviewDisplayed = true;
    }
}

// Called when the mouse wheel and we drag a task.
Task.prototype.mouseWheel = function (delta)
{
    // Changes the displayed page if we are not on the tasks list
    if (gl_desktop.mouse.x >= gl_desktop.middleMarginLeft)
    {
        var tasksList = gl_desktop.node.tasks_list;
        // Gets the position of the ghost task
        if (delta > 0 || !this.taskCache.createPage)
            this.movePageWheel = Math.max(this.movePageWheel - delta * C.Desktop.movePageWheelSpeed, -tasksList.scrollTop);
        // Scrolls the tasks list up to ensure that the ghost of the task icon is always visible
        if (this.movePageWheel < gl_desktop.taskIconHeight && tasksList.scrollTop)
        {
            tasksList.scrollTop += this.movePageWheel - gl_desktop.taskIconHeight;
            this.movePageWheel = gl_desktop.taskIconHeight;
        }
        // Scrolls the tasks list down to ensure that the ghost of the task icon is always visible
        else if (this.movePageWheel > gl_desktop.topHeight - gl_desktop.taskIconHeight * 2 && tasksList.scrollHeight - tasksList.scrollTop > gl_desktop.topHeight)
        {
            tasksList.scrollTop += this.movePageWheel - gl_desktop.topHeight + gl_desktop.taskIconHeight * 2;
            this.movePageWheel = gl_desktop.topHeight - gl_desktop.taskIconHeight * 2;
        }
        var oldPage = gl_desktop.getCurrentPage();
        // Moves the ghost according to the new position
        this.updateTasksList(this.movePageWheel);
        // Displays the preview of the task in the page if it has changed
        if (gl_desktop.getCurrentPage() && !gl_desktop.isDisplayed(oldPage))
        {
            this.addTaskPosition = C.Desktop.defaultPosition;
            this.addTaskParent = undefined;
            gl_desktop.getCurrentPage().preview();
        }
    }
    else
        this.updateTasksList(gl_desktop.mouse.y);
}

// Puts the focus on the page of the task.
Task.prototype.mouseDownContent = function ()
{
    this.getPage().getContainer().focus();
}

// Puts a ghost task under the cursor to show the place where the dragged task will be moved.
// @param y : The y position of the mouse.
Task.prototype.updateTasksList = function (y)
{
    // Changes the reference y depending if the task has been dragged up or down
    y = y - this.mouse.y + gl_desktop.node.tasks_list.scrollTop;
    if (y > this.element.y)
        y += gl_desktop.taskIconHeight;
    this.removeGhostTask();
    // Gets the position of the task under the cursor
    var data = this.getCursorPosition(y);
    var page = data.page;
    var task = data.task;
    // If we want to create a page, the ghost is not required
    if (this.taskCache.createPage)
    {
        gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + gl_desktop.taskIconHeight + "px";
        this.previewNewPage();
        return ;
    }
    // Creates a ghost task in the current page
    this.ghost = document.createElement("div");
    this.ghost.className = "ghost";
    this.ghost.style.height = gl_desktop.taskIconHeight + "px";
    // The current page is above the original page
    if (data.number_page <= this.page)
    {
        if (!task.nextSibling && y > data.position - C.Desktop.tasksListPadding - gl_desktop.taskIconHeight / 2)
            page.appendChild(this.ghost);
        else if (task.nextSibling && y > data.position - gl_desktop.taskIconHeight / 2)
            page.insertBefore(this.ghost, task.nextSibling);
        else
            page.insertBefore(this.ghost, task);
    }
    // The current page is below the original page
    else
    {
        if (!task.nextSibling && y < data.position - C.Desktop.tasksListPadding - gl_desktop.taskIconHeight / 2)
            page.insertBefore(this.ghost, task);
        else if (task.nextSibling && y < data.position - gl_desktop.taskIconHeight / 2)
            page.insertBefore(this.ghost, task);
        else if (task.nextSibling)
            page.insertBefore(this.ghost, task.nextSibling);
        else
            page.appendChild(this.ghost);
    }
    // Displays the page under which the cursor is
    // If the page is different or the mouse just came over the tasks list
    if (this.lastPagePreview != page.object || (this.addTaskParent || this.addTaskPosition))
    {
        // Hides the task displayed in previewNewPage
        if (!this.lastPagePreview)
            this.hide();
        // Hides all the windows if we are on an other page
        if (this.getPage() != page.object)
            hideAllWindows();
        // Otherwise displays the windows displayed at the beginning of the drag
        else
            restoreWindows();
        // Displays the preview
        page.object.preview();
        // Displays the page
        page.object.display();
        // Ensures that a page is always displayed on the desktop
        if (!this.originalDesktopPage && data.number_page == this.page)
            gl_desktop.hide();
        else
            gl_desktop.displayNext(page.object);
        // Remembers the page on which the preview is so we don't display it twice
        this.lastPagePreview = page.object;
    }
    gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + "px";
}

// Gets the position of the task under the cursor and some other data.
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
    // Runs through the tasks in order to find the position of the task under the cursor
    for (page = gl_desktop.node.tasks_list.firstChild; page && getClassName(page, "page") && (position < y || !number_page); page = page.nextSibling, number_page++)
    {
        position += C.Desktop.tasksListPadding;
        for (task = page.firstChild, number_task = 0; task && (position < y || !number_task); task = task.nextSibling, number_task++)
            position += gl_desktop.taskIconHeight;
        if (!task)
            position += C.Desktop.tasksListPadding;
    }
    // Gets the correct nodes
    page = page.previousSibling;
    (task) ? task = task.previousSibling : task = page.lastChild;
    // Calculates the upper limit
    var limit = position - gl_desktop.taskIconHeight;
    if (!task.previousSibling && !task.nextSibling)
        limit -= C.Desktop.tasksListPadding * 2;
    else if (!task.previousSibling || !task.nextSibling)
        limit -= C.Desktop.tasksListPadding;
    // Saves the data in a cache, so that we dont have to calculate it each time the cursor moves
    this.taskCache = {position : position, limit : limit, number_page : number_page, number_task : number_task, page : page, task : task, createPage : (y > position)}
    return (this.taskCache);
}

// Moves the dragged task to the page under the cursor.
Task.prototype.mouseUp = function ()
{
    // Ensures that a new page is only created if necessary
    if (!this.icon.nextSibling && !this.icon.previousSibling && !getClassName(this.icon.parentNode.nextSibling, "page"))
        this.taskCache.createPage = false;
    // If the dragged task has been moved in a new page
    if (this.taskCache.number_page != this.page || this.taskCache.createPage || (this.addTaskParent && this.addTaskParent.getPage() != this.getPage()))
    {
        // Removes the old task
        this.getPage().removeTask(this, true);
        // Puts the task in a new page
        if (this.taskCache.createPage)
            (new Page()).addTask(this);
        // Adds the task to the its new page
        else if (this.addTaskParent)
            this.addTaskParent.getPage().addTask(this, this.addTaskPosition, this.addTaskParent, this.addTaskParent.icon);
        else
            this.ghost.parentNode.object.addTask(this, this.addTaskPosition, this.addTaskParent, this.ghost);
        hideAllWindows();
        this.getPage().display(true);
    }
    // If the page hasn't changed
    else if (this.taskCache.number_page == this.page && this.ghost)
    {
        var page = this.getPage();
        if (this.addTaskParent)
            page = this.addTaskParent.getPage();
        // Changes the position of the task in the tree
        if (gl_desktop.node.preview.style.display == "block")
        {
            page.removeNode(this.node);
            page.addNode(this, this.addTaskPosition, this.addTaskParent);
            page.onResize();
        }
        // Moves the task in the tasks list
        this.icon.parentNode.removeChild(this.icon);
        this.ghost.parentNode.insertBefore(this.icon, this.ghost);
        // If the page is on the desktop and the task has not been dragged, hides all the windows (its a simple click)
        if (page.getContainer() instanceof Desktop && this.resistance)
            hideAllWindows();
    }
    this.removeGhostTask();
    gl_desktop.node.tasks_list.lastChild.style.height = C.Desktop.newTaskHeight + "px";
    this.taskCache = undefined;
    this.icon.style.position = "static";
    removeClassName(this.icon, "drag");
    gl_desktop.node.preview.style.display = "none";
}

// Removes the resistance and start to drag the task.
Task.prototype.startDrag = function (y)
{
    this.resistance = false;
    // Updates the position of the mouse in the task
    if (y < this.element.y - gl_desktop.node.tasks_list.scrollTop)
    {
        this.mouse.y -= C.Desktop.taskResistance;
        y += C.Desktop.taskResistance;
    }
    else if (y > this.element.y - gl_desktop.node.tasks_list.scrollTop)
    {
        this.mouse.y += C.Desktop.taskResistance;
        y -= C.Desktop.taskResistance;
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
    // Moves the task
    this.icon.style.top = y + "px";
    return (y);
}

// Displays a preview of a new page.
// @param task : The task of the future new page.
Task.prototype.previewNewPage = function ()
{
    // Hides the page currently displayed and all the windows
    gl_desktop.hide();
    hideAllWindows();
    // The new task is displayed as a full page
    this.setCoordinates(gl_desktop.left, gl_desktop.top, gl_desktop.width, gl_desktop.height);
    this.display();
    // Hides the preview
    gl_desktop.node.preview.style.display = "none";
    // Ensures that the preview of the page before the new page will be displayed again when we come back on it
    this.lastPagePreview = undefined;
}

// Displays the content of the task.
Task.prototype.display = function ()
{
    this.content.style.display = "block";
}

// Hides the content of the task.
Task.prototype.hide = function ()
{
    this.content.style.display = "none";
}

// Closes the task.
Task.prototype.close = function ()
{
    if (this.content.parentNode)
        this.content.parentNode.removeChild(this.content);
    removeEvent(this.icon, "mousedown", this.mouseDownEvent);
    removeEvent(this.content, "mousemove", this.mouseMoveEvent);
    removeEvent(this.content, "mousedown", this.mouseDownContentEvent);
}

// Removes the ghost task of the tasks list.
Task.prototype.removeGhostTask = function ()
{
    if (this.ghost)
    {
        this.ghost.parentNode.removeChild(this.ghost);
        this.ghost = undefined;
    }
}

// Sets the coordinates of the content of the task.
Task.prototype.setCoordinates = function(left, top, width, height)
{
    this.left = left;
    this.content.style.left = left + "px";
    this.top = top;
    this.content.style.top = top + "px";
    this.width = width;
    this.content.style.width = width + "px";
    this.height = height;
    this.content.style.height = height + "px";
}

// Returns true if the task dragged is over the new page area (the bottom of the tasks list).
Task.prototype.isOnNewPage = function() { return (this.taskCache && this.taskCache.createPage); }
// Returns the page of the task.
Task.prototype.getPage = function () { return (this.icon.parentNode.object); }

} /*! Task !*/

/********** TaskTreeNode **********/{
// Represents a node of the tasks tree that allows to display the tasks in a page.
// @param node : Construct the object's properties.
// h : If positive the node is split horizontally (the separation bar is vertical).
// Represents the ratio between 0 and 1 of the space taken by its first child.
// Thus the space taken by the second child is (1 - h).
// v : If positive the node is split vertically (the separation bar is horizontal).
// The details are the same as h. h and v can't be positive at the same time.
// task : If defined, the node is a leaf of the tree (it has no child) and this
// property represents the Task to display in the node. h and v are ignored in the leafs.
// first : The first child of the node (left is h, top if v).
// second : The second child of the node (right if h, bottom if v).
// left top width height : The coordinates of the node the last time it has been rendered.
function TaskTreeNode(node)
{
    for (var key in node)
        this[key] = node[key];
    this.mouse = undefined;
    this.element = undefined;
    this.leftNode = undefined;
    this.rightNode = undefined;
    this.topNode = undefined;
    this.bottomNode = undefined;
    this.startResize = undefined;
}

// Starts to resize the node.
TaskTreeNode.prototype.mouseDown = function (event)
{
    gl_desktop.drag.start(event, this.resize, this, "mouseMove", "mouseWheel", "mouseUp");
    this.mouse = gl_desktop.drag.getMouse();
    this.element = gl_desktop.drag.getElement();
    // Searches nodes that are in the edge of the current node and save their properties
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

// Resizes the node according to the mouse position.
TaskTreeNode.prototype.mouseMove = function (event)
{
    var m = mouseCoordinates(event);
    var x = m.x;
    var y = m.y;

    // Resizes horizontally
    if (this.h >= 0)
    {
        // Takes into account where the user clicked in the resize bar
        x += C.Desktop.taskMargin / 2 - this.mouse.x;
        // Computes the horizontal resize
        this.h = (x - this.left) / Math.max(this.width, 1);
        this.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, this.h));
        // The number of pixels the mouse moved vertically from the click position
        var diff = y - this.mouse.y - this.element.y;
        // Applies a little resistance before starting to resize vertically
        if (!this.startResize && Math.abs(diff) < C.Desktop.resizeResistance)
            diff = 0;
        else
        {
            // Starts to resize vertically
            if (!this.startResize)
                this.startResize = diff;
            (this.startResize > 0) ? diff -= C.Desktop.resizeResistance : diff += C.Desktop.resizeResistance;
        }
        if (this.topNode)
            this.topNode.parent.v = this.topNode.v;
        if (this.bottomNode)
            this.bottomNode.parent.v = this.bottomNode.v;
        // Resizes vertically the top node
        if (this.topNode && (!this.bottomNode || diff < 0))
        {
            var v = this.topNode.v + (1 - (this.topNode.height - diff) / Math.max(this.topNode.height, 1));
            this.topNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, v));
            // Adjusts the bottom node to the new top node ratio, so that the bottom node doesn't move
            if (this.bottomNode && this.topNode.level > this.bottomNode.level && C.Desktop.stabilizeTaskResize)
            {
                // Old size of the bottom node second, divided by the new size of the bottom node
                var a = 1 - (1 - this.bottomNode.v) / ((1 - this.topNode.parent.v) / (1 - this.topNode.v));
                if (this.topNode.parent.v * this.topNode.height <= C.Desktop.taskMargin / 2)
                    a = 1 - (this.bottomNode.height * (1 - this.bottomNode.v)) / ((this.bottomNode.height / (1 - this.topNode.v)) - C.Desktop.taskMargin / 2);
                this.bottomNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
            }
        }
        // Resizes vertically the bottom node
        if (this.bottomNode && (!this.topNode || diff > 0))
        {
            var v = this.bottomNode.v + (1 - (this.bottomNode.height - diff) / Math.max(this.bottomNode.height, 1));
            this.bottomNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, v));
            // Adapts the top node to the new bottom node ratio, so that the top node doesn't move
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
    // Resizes vertically
    else
    {
        // Takes into account where the user clicked in the resize bar
        y += C.Desktop.taskMargin / 2 - this.mouse.y;
        // Computes the vertical resize
        this.v = (y - this.top) / Math.max(this.height, 1);
        this.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, this.v));
        // The number of pixels the mouse moved horizontally from the click position
        var diff = x - this.mouse.x - this.element.x;
        // Applies a little resistance before starting to resize horizontally
        if (!this.startResize && Math.abs(diff) < C.Desktop.resizeResistance)
            diff = 0;
        else
        {
            // Starts to resize horizontally
            if (!this.startResize)
                this.startResize = diff;
            (this.startResize > 0) ? diff -= C.Desktop.resizeResistance : diff += C.Desktop.resizeResistance;
        }
        if (this.leftNode)
            this.leftNode.parent.h = this.leftNode.h;
        if (this.rightNode)
            this.rightNode.parent.h = this.rightNode.h;
        // Resizes horitontally the left node
        if (this.leftNode && (!this.rightNode || diff < 0))
        {
            var h = this.leftNode.h + (1 - (this.leftNode.width - diff) / Math.max(this.leftNode.width, 1));
            this.leftNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, h));
            // Adjusts the right node to the new left node ratio, so that the right node doesn't move
            if (this.rightNode && this.leftNode.level > this.rightNode.level && C.Desktop.stabilizeTaskResize)
            {
                // Old size of the right node second, divided by the new size of the right node
                var a = 1 - (1 - this.rightNode.h) / ((1 - this.leftNode.parent.h) / (1 - this.leftNode.h));
                if (this.leftNode.parent.h * this.leftNode.width <= C.Desktop.taskMargin / 2)
                    a = 1 - (this.rightNode.width * (1 - this.rightNode.h)) / ((this.rightNode.width / (1 - this.leftNode.h)) - C.Desktop.taskMargin / 2);
                this.rightNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
            }
        }
        // Resizes horitontally the right node
        if (this.rightNode && (!this.leftNode || diff > 0))
        {
            var h = this.rightNode.h + (1 - (this.rightNode.width - diff) / Math.max(this.rightNode.width, 1));
            this.rightNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, h));
            // Adapts the left node to the new right node ratio, so that the left node doesn't move
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
    // Retreives the page of the node
    for (var node = this.first; node; node = node.first)
        if (node.task)
            // Displays the modifications
            node.task.getPage().renderTree();
}

// Resizes the tasks margin and the resize bar.
TaskTreeNode.prototype.mouseWheel = function (delta)
{
    // Changes the margin between the tasks
    C.Desktop.taskMargin += delta;
    C.Desktop.taskMargin = Math.max(C.Desktop.taskMargin, 1);
    // Takes into account the position of the mouse in the bar
    if (this.h != -1)
        this.mouse.x = C.Desktop.taskMargin / 2;
    else
        this.mouse.y = C.Desktop.taskMargin / 2;
    // Retreives the page of the node
    for (var node = this.first; node; node = node.first)
        if (node.task)
            // Displays the modifications
            node.task.getPage().renderTree();
}

// Stops the resize.
TaskTreeNode.prototype.mouseUp = function ()
{
    delete this.leftNode;
    delete this.rightNode;
    delete this.topNode;
    delete this.bottomNode;
    delete this.startResize;
}

} /*! TaskTreeNode !*/

/********** TasksList **********/{
// Manages the tasks list.
function TasksList()
{
    this.mouse = undefined;
    this.interval = undefined;
    this.delta = undefined;
    var tasksList = this;
    addEvent(gl_desktop.node.resize_tasks_list, "mousedown", function (event) { tasksList.mouseDown(event); });
}

// Starts to resize the tasks list.
TasksList.prototype.mouseDown = function (event)
{
    gl_desktop.drag.start(event, gl_desktop.node.resize_tasks_list, this, "mouseMove", "mouseWheel");
    this.mouse = gl_desktop.drag.getMouse();
}

// Resizes the tasks list.
TasksList.prototype.mouseMove = function (event)
{
    var mouse = mouseCoordinates(event);
    
    // Updates the width of the tasks list according to the position of the mouse
    C.Desktop.tasksListWidth = Math.max(C.Desktop.resizeTasksListMin, Math.min(C.Desktop.resizeTasksListMax, mouse.x - this.mouse.x - T.Menu.width));
    gl_desktop.node.tasks_list.style.width = C.Desktop.tasksListWidth + "px";
    gl_desktop.updateMiddleMarginLeft();
    // Resizes the tasks and pages icons
    this._resize();
    // Applies the changes on the desktop
    gl_desktop.onResize();
}

// Changes the padding of the pages in the tasks list using the mouse wheel.
TasksList.prototype.mouseWheel = function (delta)
{
    // Changes the padding
    C.Desktop.tasksListPadding = Math.max(1, C.Desktop.tasksListPadding + delta);
    // Resizes the tasks and pages icons
    this._resize();
    // Applies the changes on the desktop
    gl_desktop.onResize();
}

// Updates the pages and tasks icons sizes.
TasksList.prototype._resize = function ()
{
    // Ensures that the padding respect the minimum width of a task
    if (C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding < C.Desktop.taskIconMinWidth)
        C.Desktop.tasksListPadding = Math.round((C.Desktop.tasksListWidth - C.Desktop.taskIconMinWidth) / 2);
    // The width is based on the tasks list width and the padding of the pages
    var width = C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding;
    // The height is computed using the width and a ratio
    gl_desktop.setTaskIconHeight(width * C.Desktop.taskHeightRatio);
    for (var page = gl_desktop.node.tasks_list.firstChild; page; page = page.nextSibling)
        if (getClassName(page, "page"))
        {
            page.style.padding = C.Desktop.tasksListPadding + "px";
            for (var task = page.firstChild; task; task = task.nextSibling)
            {
                task.style.width = width + "px";
                task.style.height = gl_desktop.taskIconHeight + "px";
            }
        }
}

// Scrolls the tasks list up or down, depending on the delta.
// @param delta : The speed of the scroll. Scroll top if positive, down if negative.
// Should be between 1 and -1.
TasksList.prototype.scroll = function (delta)
{
    var tasks_list = gl_desktop.node.tasks_list;
    var height = tasks_list.scrollHeight - gl_desktop.topHeight;
    
    this.delta = delta;
    // Nothing to scroll
    if (!height || !delta || (delta > 0 && tasks_list.scrollTop == 0) || (delta < 0 && tasks_list.scrollTop == height))
        this.stopScroll();
    // Creates the interval that will scroll the tasks
    else if (!this.interval)
        this.interval = setInterval("gl_desktop.tasksList.scrollInterval()", 1000 / 30);
    gl_desktop.taskButtons.startScroll();
}
// This method is called by setInterval and scrolls the tasks list according to the delta.
TasksList.prototype.scrollInterval = function ()
{
    var tasks_list = gl_desktop.node.tasks_list;
    
    if (!this.interval)
        return ;
    // Calculate the number of pixels to scroll
    if (this.delta > 0)
        tasks_list.scrollTop -= (Math.exp(Math.abs(this.delta)) - 1) * C.Desktop.scrollSpeed;
    else
        tasks_list.scrollTop += (Math.exp(Math.abs(this.delta)) - 1) * C.Desktop.scrollSpeed;
    if (gl_desktop.drag.isDragging("Task"))
        gl_desktop.drag.getObject().updateTasksList(gl_desktop.mouse.y);
    // Scroll top finished
    if (tasks_list.scrollTop <= 0)
    {
        tasks_list.scrollTop = 0;
        clearInterval(this.interval);
        delete this.interval;
        gl_desktop.taskButtons.stopScroll();
    }
    // Scroll down finished
    else if (tasks_list.scrollTop >= tasks_list.scrollHeight - gl_desktop.topHeight)
    {
        tasks_list.scrollTop = tasks_list.scrollHeight - gl_desktop.topHeight;
        this.stopScroll();
    }
}

// Stops the scrolling of the tasks list.
TasksList.prototype.stopScroll = function ()
{
    if (this.interval)
    {
        clearInterval(this.interval);
        delete this.interval;
        delete this.delta;
    }
    gl_desktop.taskButtons.stopScroll();
}

} /*! TasksList !*/

/********** TaskButtons **********/{
// Manages the buttons on the task icons.
function TaskButtons()
{
    // The task buttons
    this.buttons = removeTextNodes(getElementsByClassName("task_buttons", gl_desktop.node.top, true));
    // The buttons
    this.close = getElementsByClassName("close", this.buttons, true);
    this.hide = getElementsByClassName("hide", this.buttons, true);
    this.window = getElementsByClassName("window", this.buttons, true);
    this.full_screen = getElementsByClassName("full_screen", this.buttons, true);
    this.buttons.firstChild.style.left = -C.Desktop.taskButtonsWidth * 2 + "px";
    // Events
    var object = this;
    addEvent(document.body, "mouseup", function (event) { object.mouseUp(event); });
}

// Register the events that allows to display the buttons of the task.
TaskButtons.prototype.addTask = function (task)
{
    var object = this;
    addEvent(task.icon, "mouseover", function (event) { object.mouseOver(event, task.icon); });
    addEvent(task.icon, "mouseout", function (event) { object.mouseOut(event, task.icon); });
    addEvent(task.icon, "mousedown", function (event) { object.mouseDown(event, task); });
}

// Displays the buttons while we are on the task icon.
TaskButtons.prototype.mouseOver = function (event, task)
{
    if (getEventRelatedTarget(event, "task", 5) != task)
    {
        // Puts the buttons in the task icon
        this.buttons.parentNode.removeChild(this.buttons);
        task.insertBefore(this.buttons, task.firstChild);
        // Update the size of the buttons
        var taskWidth = C.Desktop.tasksListWidth - 2 * C.Desktop.tasksListPadding;
        this.buttons.style.width = (C.Desktop.taskButtonsWidth * 4 + taskWidth) + "px";
        this.buttons.firstChild.style.top = -gl_desktop.node.tasks_list.scrollTop + "px";
        this.close.style.height = gl_desktop.taskIconHeight + "px";
        this.hide.style.height = gl_desktop.taskIconHeight + "px";
        this.window.style.height = gl_desktop.taskIconHeight + "px";
        this.full_screen.style.height = gl_desktop.taskIconHeight + "px";
    }
}

// Hides the buttons.
TaskButtons.prototype.mouseOut = function (event, task)
{
    if (getEventRelatedTarget(event, "task", 5) != task)
    {
        this.buttons.parentNode.removeChild(this.buttons);
        gl_desktop.node.top.appendChild(this.buttons);
    }
}

// Execute the action of a button
TaskButtons.prototype.mouseDown = function (event, task)
{
    var button = (event.target || event.srcElement);
    var page = task.getPage();
    
    // Closes the task
    if (button == this.close)
    {
        this.buttons.parentNode.removeChild(this.buttons);
        gl_desktop.node.top.appendChild(this.buttons);
        if (task.icon.nextSibling)
            task.icon.nextSibling.insertBefore(this.buttons, task.icon.nextSibling.firstChild);
        task.getPage().removeTask(task);
    }
    // Hides / displays the page
    else if (button == this.hide)
    {
        if (page.getContainer().isDisplayed(page) && (page.getContainer() instanceof Window || !isWindowsDisplay()))
            page.getContainer().hide();
        else
        {
            page.getContainer().display(page, true);
            if (page.getContainer() instanceof Desktop)
                hideAllWindows();
        }
    }
    // Moves the page to the desktop / window
    else if (button == this.window)
    {
        page.getContainer().close(page);
        if (page.getContainer() instanceof Desktop)
            openWindow(page);
        else
        {
            gl_desktop.setPage(page);
            hideAllWindows();
        }
    }
    // Displays the page in full screen
    else if (button == this.full_screen)
        console.log("full_screen");
    // Otherwise we hide the buttons because the task is going to be dragged
    else
        setClassName(this.buttons, "hide");
}

// Displays the buttons if the mouse is still on the icon.
TaskButtons.prototype.mouseUp = function (event)
{
    removeClassName(this.buttons, "hide");
}

// These methods ensure that the buttons are not displayed while the tasks list is scrolling.
TaskButtons.prototype.hideButtons = function ()
{
    var tasks_list = gl_desktop.node.tasks_list;
    if (tasks_list.scrollTop > 0 && tasks_list.scrollTop < tasks_list.scrollHeight - gl_desktop.topHeight)
    {
        this.buttons.parentNode.removeChild(this.buttons);
        gl_desktop.node.top.appendChild(this.buttons);
    }
}

TaskButtons.prototype.startScroll = function ()
{
    var tasks_list = gl_desktop.node.tasks_list;
    if (tasks_list.scrollTop > 0 && tasks_list.scrollTop < tasks_list.scrollHeight - gl_desktop.topHeight)
        setClassName(this.buttons, "scroll");
    else
        this.stopScroll();
}

TaskButtons.prototype.stopScroll = function ()
{
    removeClassName(this.buttons, "scroll");
    this.buttons.firstChild.style.top = -gl_desktop.node.tasks_list.scrollTop + "px";
}

} /*! TaskButtons !*/

/********** Drag **********/{
// Helps to drag an object (only one at a time).
function Drag()
{
    // The dragged object
    this.object = undefined;
    // The name of its class
    this.objectName = undefined;
    // Callbacks
    this.mouseMoveCallback = undefined;
    this.mouseUpCallback = undefined;
    this.mouseWheelCallback = undefined;
    // The position of the mouse in the element
    this.mouse = { x : 0, y : 0 };
    // The initial position of the element
    this.element = { x : 0, y : 0 };
}

// Starts to drag an object.
// @param event : The event that triggered the drag.
// @param node : The html node to drag.
// @param object : The object to drag.
// @param mouseMove : The name of a method in the object to call when the mouse moves.
// @param mouseWheel : The name of a method in the object to call when the mouse wheel clicks. Optional.
// @param mouseUp : The name of a method in the object to call when the drag is finished. Optional.
Drag.prototype.start = function (event, node, object, mouseMove, mouseWheel, mouseUp)
{
    var mouse = mouseCoordinates(event);
    var element = elementCoordinates(node);
    
    // If something was already being dragged, we stop it
    if (this.isDragging())
        this._stop();
    // The coordinates of the mouse inside the node
    this.mouse = {x : mouse.x - element.x,
                  y : mouse.y - element.y};
    // The absolute coordinates of the node
    this.element = {x : element.x,
                    y : element.y};
    this.object = object;
    this.objectName = getObjectName(object);
    this.mouseMoveCallback = mouseMove;
    this.mouseUpCallback = mouseUp;
    this.mouseWheelCallback = mouseWheel;
    // The selection is disabled during the drag
    disableSelection(false);
}

// Called when the mouse moves.
Drag.prototype.mouseMove = function (event)
{
    if (this.object && this.mouseMoveCallback)
        this.object[this.mouseMoveCallback](event);
}

// Called when the mouse wheel clicks.
Drag.prototype.mouseWheel = function (delta)
{
    if (this.object && this.mouseWheelCallback)
        this.object[this.mouseWheelCallback](delta);
}

// Called when the mouse is up (the drag is finished).
Drag.prototype.mouseUp = function (event)
{
    if (this.object && this.mouseUpCallback)
        this.object[this.mouseUpCallback](event);
    this._stop();
}

// Stops the dragging of the object.
Drag.prototype._stop = function ()
{
    this.object = undefined;
    this.objectName = undefined;
    this.mouseMoveCallback = undefined;
    this.mouseUpCallback = undefined;
    this.mouseWheelCallback = undefined;
    this.mouse = { x : 0, y : 0 };
    this.element = { x : 0, y : 0 };
    disableSelection(true);
}

// True if the object is being dragged
// @param object : If object is a string, returns true if the dragged object is the same type.
// If it is an object, returns true if the dragged object is the same.
// Otherwise returns true if something is being dragged.
Drag.prototype.isDragging = function (object)
{
    if (!object && this.object)
        return (true);
    else if (!this.object)
        return (false);
    else if (this.object === object)
        return (true);
    else if (this.objectName === object)
        return (true);
    return (false);
}
// The position of the mouse inside the dragged element.
// @return : { x, y }
Drag.prototype.getMouse = function () { return (this.mouse); }
// The initial position of the dragged element.
// @return : { x, y }
Drag.prototype.getElement = function () { return (this.element); }
// Returns the instance of the object being dragged.
Drag.prototype.getObject = function () { return (this.object); }

} /*! Drag !*/
