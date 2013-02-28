// Manages the desktop.
var gl_desktop;

function Desktop(task)
{
    var self = this;
    gl_desktop = self;
    
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.desktop = $("#desktop");
        self.node.top = $("#desktop>.top")[0];
        self.node.middle = $("#desktop>.middle")[0];
        self.node.tasks_list = $(self.node.middle).children(".tasks_list").children(".icons")[0];
        self.node.tasks_list.container = $(self.node.middle).children(".tasks_list")[0];
        self.node.tasks_list.top = $(self.node.tasks_list).children(".top")[0];
        self.node.tasks_list.bottom = $(self.node.tasks_list).children(".bottom")[0];
        self.node.pages = $(self.node.middle).children(".pages")[0];
        self.node.tasks = $(self.node.middle).children("#tasks")[0];
        self.node.resize_layer = $(self.node.middle).children(".resize_layer")[0];
        self.node.preview = $("#desktop>#preview")[0];

        // Members
        self.middleHeight; // The height of the middle area
        self.drag = new Drag(); // Allows to drag elements
        self.taskButtons = new TaskButtons(); // Manages the buttons of the tasks
        self.tasksList = new TasksList(); // Manages the scroll of the tasks list
        self.taskPreview = new TaskPreview(); // Displays a preview while a task icon is being dragged
        self.sessions = new Sessions(); // Manages the sessions
        self.mouse = { x : 0, y : 0 }; // The position of the mouse on the desktop
        self.currentPage = undefined; // The page currently displayed by the desktop
        // The coordinates of the main area of the desktop
        self.left;
        self.top;
        self.width;
        self.height;
        
        // Sets the default values
        $(self.node.top).height(C.Desktop.topHeight);
        
        // Events
        $(document.body).mousewheel(function (e, delta) { self.mouseWheel(e, delta); });
        $(document.body).mousemove(function (e) { self.mouseMove(e); });
        $(document.body).mouseup(function (e) { self.mouseUp(e); });
    }
    
    // Called when the browser is resized. Updates the size of the desktop.
    self.onResize = function ()
    {
        var width = gl_browserSize.width;
        var height = gl_browserSize.height;
        self.middleHeight = height - C.Desktop.topHeight - C.Desktop.bottomHeight;
        $(self.node.middle).height(self.middleHeight);
        self.left = C.Desktop.tasksListWidth;
        self.top = C.Desktop.topHeight;
        self.width = width - C.Desktop.tasksListWidth;
        self.height = self.middleHeight;
        self.node.resize_layer.style.left = self.left + "px";
        self.node.resize_layer.style.top = self.top + "px";
        self.node.resize_layer.style.width = self.width + "px";
        self.node.resize_layer.style.height = self.middleHeight + "px";
        if (self.currentPage)
            self.currentPage.onResize();
        gl_player.onResize(width, height);
    }
    
    // Opens a task in a new page.
    // @param resource : The name of the resource that will be loaded in the task.
    // @param parameter : An optional parameter for the resource.
    self.openPage = function (resource, parameter, event)
    {
        if (event && $.Event(event).originalEvent.which != 1)
            return ;
        // Loads the task from the resources
        gl_resources.load(resource, function (html)
        {
            // Creates the page and the task
            var page = new Page();
            var task = new Task(resource, html);
            page.addTask(task);
            gl_resources.callJs(resource, task, parameter);
            page.display();
            // Ensures that the tasks list is displayed
            $(self.node.tasks_list.container).addClass("display");
            // Scrolls to the bottom of the tasks list
            self.node.tasks_list.scrollTop = gl_desktop.node.tasks_list.scrollHeight - gl_desktop.middleHeight - C.Desktop.pageMargin;
        });
    }
    
    // Closes all the pages and their tasks.
    self.disconnect = function ()
    {
        var pages = $(gl_desktop.node.tasks_list).children(".page");
        pages.each(function () { this.object.close(); });
    }
    
    // Called each time the mouse wheel is used.
    // @param delta : The direction and velocity of the movement.
    self.mouseWheel = function(e, delta)
    {
        delta = Math.round(delta * C.Desktop.mouseWheelMultiplier);
        self.tasksList.mouseWheel(delta);
        self.drag.mouseWheel(e, delta);
    }
    
    // Called each time the mouse moves over the desktop.
    self.mouseMove = function (e)
    {
        self.mouse = { pageX : e.pageX, pageY : e.pageY }
        self.drag.mouseMove(e);
    }

    // Called when the mouse is up.
    self.mouseUp = function (e)
    {
        self.drag.mouseUp(e);
    }
    
// Container interface
    
    // Changes the page displayed by the desktop.
    // @param page : The page to display.
    self.display = function(page)
    {
        // Hides the old page and the windows
        self.hide();
        gl_windows.hide();
        // Sets the new page
        self.currentPage = page;
        $(self.currentPage.icon).removeClass("window");
        self.currentPage.setZIndex(0);
        $(self.node.resize_layer).addClass("display");
    }

    // Hides the page currently displayed by the desktop.
    self.hide = function ()
    {
        if (self.currentPage)
        {
            if (self.currentPage.isDisplayed())
                self.currentPage.hide();
            self.currentPage = undefined;
            $(self.node.resize_layer).removeClass("display");
        }
    }
    
    // Notifies the desktop that a page has been closed.
    // @parem page : The page closed.
    self.close = function (page)
    {
        if (self.currentPage && self.currentPage == page)
        {
            self.currentPage = undefined;
            $(self.node.resize_layer).removeClass("display");
        }
    }
    
    // Notifies that the desktop is no longer the container of the page.
    // @parem page : The page concerned.
    self.containerChanged = function(page)
    {
        if (self.currentPage && self.currentPage == page)
        {
            self.currentPage = undefined;
            $(self.node.resize_layer).removeClass("display");
        }
    }
    
    self.init();
    return (self);
}

// A page is a container that can store multiple tasks.
function Page()
{
    var self = this;
    
    self.init = function ()
    {
        self.numberTasks = 0; // The number of tasks in the page
        self.icon; // The icon of the page in the tasks list
        self.tree; // The tree allows to render the tasks of the page. It consists of TaskTreeNode objects.
        self.container = gl_desktop; // The container that displays the page
        self.zIndex = 0; // THe z-index of the content of the page
        self.justFocused = false; // True if the page was not focused before the mouse down event
        // Drag properties
        self.element; // The initial position of the icon {x, y}
        self.mouse; // The position of the mouse in the icon {x, y}
        self.ghost; // A div that shows the future position of the page while it is dragged
        // The coordinates of the page
        self.left;
        self.top;
        self.width;
        self.height;
    
        // Creates the icon and insert it in the tasks list
        self.icon = $("<div></div>")[0];
        $(self.icon).addClass("page");
        self.icon.object = self;
        self.icon.height; // The height of the icon
        $(self.icon).mousedown(function (e) { self.mouseDown(e); });
        $(self.icon).mouseup(function (e) { self.mouseUpClose(e) });
        $(self.icon).insertBefore(gl_desktop.node.tasks_list.bottom);
        
        // Creates the content of the page
        self.content = $("<div></div>")[0];
        $(self.content).addClass("page");
        self.content.object = self;
        $(self.content).appendTo(gl_desktop.node.pages);
    }
    
    // Adds a task in the page.
    // @param task : The task to add.
    // @param position : The position of the new task in the page or the parent (n s e w).
    // @param parent : If defined, the parent task is cut in half and the task take a half.
    // @param beforeIcon : The icon before which the task will be added in the tasks list.
    self.addTask = function (task, position, parent, beforeIcon)
    {
        self.addNode(task, position, parent);
        if (beforeIcon)
            $(task.icon).insertBefore(beforeIcon);
        else
            $(self.icon).append(task.icon);
        self.numberTasks++;
        self.icon.height = C.Desktop.taskIconHeight * self.numberTasks;
        task.setContainer(self.container);
        task.setZIndex(self.zIndex);
    }
    
    // Displays the page and its tasks.
    self.display = function ()
    {
        // Displays the page
        self.container.display(self);
        if (!$(self.content).hasClass("display"))
        {
            // Display the page
            $(self.content).addClass("display");
            // Displays the tasks of the page
            $(self.icon).children(".task").each(function () { this.object.display(); });
            // Updates the coordinates
            self.onResize();
        }
        // Puts the focus on it
        var pages = $(gl_desktop.node.tasks_list).children(".page.focus").removeClass("focus");
        pages.each(function () { if (this != self.icon) $(this).children(".task").each(function () { this.object.focus(false); }); });
        $(self.icon).addClass("focus");
        $(self.icon).children(".task").each(function () { this.object.focus(true); });
    }

    // Hides the page and its tasks.
    self.hide = function ()
    {
        $(self.content).removeClass("display");
        $(self.icon).removeClass("focus");
        // Hides the tasks of the page.
        $(self.icon).children(".task").each(function () { this.object.hide(); });
        self.container.hide(self);
    }
    
    // Removes a task from the page.
    // @param task : The task to remove.
    // @param move : If the task is moved its content is not removed.
    self.removeTask = function (task, move)
    {
        var taskNode = task.node; // Saves the node before the task is closed
        if (!move)
            task.close();
        else
            $(task.icon).detach();
        self.numberTasks--;
        self.icon.height = C.Desktop.taskIconHeight * self.numberTasks;
        // If the page is empty we close it
        if (self.numberTasks == 0)
            self.close();
        // Otherwise we resize its tasks
        else
        {
            self.removeNode(taskNode);
            self.onResize();
        }
    }
    
    // Closes the page.
    self.close = function ()
    {
        // Notifies the desktop that the page has been closed
        self.container.close(self);
        // Removes the tasks of the page
        $(self.icon).children(".task").each(function () { this.object.close(); });
        // Removes the page nodes
        $(self.icon).remove();
        $(self.content).remove();
        // Hides the tasks list if there is no page remaining
        if (!$(gl_desktop.node.tasks_list).children(".page").length)
            $(gl_desktop.node.tasks_list.container).removeClass("display");
    }
    
    // Puts the focus on the page when we click on it, and starts to drag it if necessary.
    self.mouseDown = function (e)
    {
        if (e.which == 1)
        {
            // Displays the page
            if (!$(self.icon).hasClass("focus"))
            {
                self.justFocused = true;
                self.display();
            }
            else
                self.justFocused = false;
            // Starts to drag the icon if we clicked directly on it (not on the task)
            if ($(e.target).hasClass("page") && !gl_desktop.drag.isDragging())
            {
                gl_desktop.drag.start(e, self.icon, self, "mouseMove", "mouseWheel", "mouseUp");
                self.element = gl_desktop.drag.getElement();
                self.mouse = gl_desktop.drag.getMouse();
                // Takes the scroll into account
                self.element.y +=  gl_desktop.node.tasks_list.scrollTop;
                // Moves the icon to the mouse position
                $(self.icon).addClass("drag");
                self.icon.style.top = (e.pageY - self.mouse.y) + "px";
                // Creates the ghost
                self.ghost = $("<div></div>");
                self.ghost.addClass("ghost");
                self.ghost.height(self.icon.height);
                self.ghost.insertBefore($(self.icon));
            }
        }
    }
    
    // Moves the page icon while it is dragged.
    // @parem mouseY : If "e" is not defined, mouseY is used instead of e.pageY.
    self.mouseMove = function (e, mouseY)
    {
        // Moves the icon
        if (e)
            mouseY = e.pageY;
        self.icon.style.top = mouseY - self.mouse.y + "px";
        // The reference y changes if we are above or below the original position of the page
        var y = mouseY - self.mouse.y - C.Desktop.topHeight + gl_desktop.node.tasks_list.scrollTop - C.Desktop.pageMargin / 2;
        if (self.element.y < y)
            y += self.icon.height + C.Desktop.pageMargin;
        var h = C.Desktop.pageMargin;
        var element;
        var pages = $(gl_desktop.node.tasks_list).children(".page");
        // Searches the page under which we are
        for (var i = 0; i < pages.length; ++i)
        {
            h += pages[i].height / 2;
            element = pages[i];
            if (y < h)
                break ;
            h += pages[i].height / 2 + C.Desktop.pageMargin;
        }
        // And moves the ghost accordingly
        if (i == pages.length)
            self.ghost.insertAfter(element);
        else
            self.ghost.insertBefore(element);
    }
    
    // Updates the position of the icon.
    self.mouseWheel = function (e)
    {
        self.mouseMove(e);
    }
    
    // Drops the page icon at the ghost position.
    self.mouseUp = function (e)
    {
        $(self.icon).removeClass("drag");
        $(self.icon).insertBefore(self.ghost);
        self.ghost.remove();
    }
    
    // Closes the page with the middle mouse.
    self.mouseUpClose = function (e)
    {
        if (e.which == 2 && e.target == self.icon && !gl_desktop.drag.isDragging())
            self.close();
    }

    // Resizes the tasks of the page.
    self.onResize = function ()
    {
        self.left = self.container.left;
        self.top = self.container.top;
        self.width = Math.max(self.container.width, 0);
        self.height = Math.max(self.container.height, 0);
        // Renders the tasks tree
        self.renderTree();
    }

    // Adds a node to the tree that represents the hierarchy of the tasks in the page.
    // @param task : The task of the node.
    // @param position : The position of the task relative to the parent (n s e w).
    // @param parent : If defined, the parent is cut in half and both of the tasks takes one part.
    self.addNode = function (task, position, parent)
    {
        // The default position
        if (!position)
            position = C.Desktop.defaultPosition;
        // Creates the tree
        if (!self.tree)
        {
            self.tree = new TaskTreeNode({ task : task });
            task.node = self.tree;
        }
        // There is only one node in the tree
        else if (!parent && self.tree.task)
            self._addNode(task, position, self.tree);
        // Adds a node at the root of the tree
        else if (!parent)
        {
            // Changes the root of the tree (the old tree becomes a leaf)
            if (position == "s" || position == "e")
            {
                self.tree = new TaskTreeNode({ first : self.tree });
                self.tree.second = new TaskTreeNode({ task : task });
                task.node = self.tree.second;
            }
            // The order changes depending on the position
            else
            {
                self.tree = new TaskTreeNode({ second : self.tree });
                self.tree.first = new TaskTreeNode({ task : task });
                task.node = self.tree.first;
            }
            // Sets the ratio depending on the position, and the number of nodes already in that position in the tree
            self.tree.h = -1;
            self.tree.v = -1;
            if (position == "w")
                self.tree.h = 1 / (self.countNode(self.tree, "h") + 2);
            else if (position == "e")
                self.tree.h = 1 - 1 / (self.countNode(self.tree, "h") + 2);
            else if (position == "n")
                self.tree.v = 1 / (self.countNode(self.tree, "v") + 2);
            else
                self.tree.v = 1 - 1 / (self.countNode(self.tree, "v") + 2);
            // Sets the parents of the leafs
            self.tree.first.parent = self.tree;
            self.tree.second.parent = self.tree;
        }
        // Adds a node after the parent
        else
            self._addNode(task, position, parent.node);
    }
    // Helper method of Page.addNode.
    self._addNode = function (task, position, node)
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
    self.countNode = function (node, ratio)
    {
        var result = 0;
        
        // If the node has one leaf and the good ratio
        if (node[ratio] >= 0 && ((node.first && node.first.task) || (node.second && node.second.task)))
            result = 1;
        // Continues the counting on the first branch
        if (node.first)
            result += self.countNode(node.first, ratio);
        // Same on the second branch
        if (node.second)
            result += self.countNode(node.second, ratio);
        return (result);
    }

    // Removes a node from the tasks tree.
    // @param node : The node to remove.
    self.removeNode = function (node)
    {
        var parent = node.parent;
        // Gets the branch to keep
        var keep = (parent.first != node) ? parent.first : parent.second;
        // Removes the resize div
        if (node.resize)
            $(node.resize).remove();
        if (parent.resize)
            $(parent.resize).remove();
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
            self.tree = keep;
    }
    
    // Displays the tasks tree recursively.
    // @param node : The current node.
    // @param left, top, width, height : The remaining area to be filled by the tasks.
    self.renderTree = function (node, left, top, width, height)
    {
        if (!self.tree)
            return ;
        // If all the parameters are empty, we are at the root of the tree
        if (!node && !left && !top && !width && !height)
        {
            node = self.tree;
            left = self.left;
            top = self.top;
            width = self.width;
            height = self.height;
        }
        // The node is a leaf, so we display its task
        if (node.task)
            node.task.setCoordinates(left, top, width, height);
        // Otherwise we split the area based on the ratios, and we dive deeper into the tree
        else if (node.first && node.second)
        {
            var taskMargin = C.Desktop.taskMargin;
            var margin = taskMargin / 2;
            // Creates the resize bar that allows to resize the tasks
            if (!node.resize)
                node.createResize(self.content);
            // Splits vertically
            if (node.h >= 0)
            {
                // Computes the width of the two childs of the current node
                var first = Math.round(width * node.h - margin);
                var second = width - first - taskMargin;
                var l = left + first + taskMargin;
                // Ensures that the result is in the range of the node
                if (l < left + taskMargin)
                {
                    l = left + taskMargin;
                    second = width - taskMargin;
                }
                else if (l > left + width)
                {
                    l = left + width;
                    first = width - taskMargin;
                }
                // Renders the next two branches
                self.renderTree(node.first, left, top, Math.max(first, 0), height);
                self.renderTree(node.second, l, top, Math.max(second, 0), height);
                // Positions the vertical resize bar
                $(node.resize).addClass("vertical");
                node.resize.style.left = Math.max(l - taskMargin, left) + "px";
                node.resize.style.top = top + "px";
                node.resize.style.width = Math.max(Math.min(taskMargin, width), 0) + "px";
                node.resize.style.height = (height > 0 ? height : 0) + "px";
            }
            // Splits horizontally
            else
            {
                // Computes the height of the two childs of the current node
                var first = Math.round(height * node.v - margin);
                var second = height - first - taskMargin;
                var t = top + first + taskMargin;
                // Ensures that the result is in the range of the node
                if (t < top + taskMargin)
                {
                    t = top + taskMargin;
                    second = height - taskMargin;
                }
                else if (t > top + height)
                {
                    t = top + height;
                    first = height - taskMargin;
                }
                // Render the next two branches
                self.renderTree(node.first, left, top, width, Math.max(first, 0));
                self.renderTree(node.second, left, t, width, Math.max(second, 0));
                // Positions the horizontal resize bar
                $(node.resize).addClass("horizontal");
                node.resize.style.left = left + "px";
                node.resize.style.top = Math.max(t - taskMargin, top) + "px";
                node.resize.style.width = (width > 0 ? width : 0) + "px";
                node.resize.style.height = Math.max(Math.min(taskMargin, height), 0) + "px";
            }
        }
        // Saves the coordinates of the node
        node.left = left;
        node.top = top;
        node.width = width;
        node.height = height;
    }
    
    // Changes the container that displays the page.
    self.setContainer = function (container)
    {
        self.container.containerChanged(self);
        self.container = container;
        $(self.icon).children(".task").each(function () { this.object.setContainer(container); });
        if (container != gl_desktop)
            $(self.content).addClass("window");
        else
            $(self.content).removeClass("window");
    }
    
    // Puts the focus on the page elements.
    self.setZIndex = function (zIndex)
    {
        self.content.style.zIndex = zIndex;
        $(self.icon).children(".task").each(function () { this.object.setZIndex(zIndex); });
        self.zIndex = zIndex;
    }
    
    // Returns true is the page is displayed.
    self.isDisplayed = function ()
    {
        return ($(self.content).hasClass("display"));
    }
    
    // Returns the container of the page.
    self.getContainer = function () { return (self.container); }

    self.init();
    return (self);
}

// A task holds a resource and is stored in a page.
// @param resource : The name of the resource displayed by the task.
// @param content : The HTML content of the resource.
function Task(resource, html)
{
    var self = this;
    
    self.init = function ()
    {
        self.icon; // The icon element of the task in the tasks list, inside the icon of the page
        self.content; // The content of the task, which stores its resource
        self.resource = new Object(); // The instance of the resource that manages the content
        self.lastFocusDate = 0; // The date of the last time the task was focused. Used by isFocus to tell if the task has the focus regardless of the events timing.
        // The following members are used when the task icon is being dragged
        self.mouse; // The position of the mouse in the icon {x, y}
        self.element; // The initial position of the element {x, y}
        self.resistance; // Applies a resistance before starting to drag the task icon
        self.ghost; // A div that shows the future position of the task while it is dragged
        self.page; // The position of the task's page in tha tasks list
        self.node; // The node of the task in the tree
        self.lastBackgroundSet; // The last task background set using setBackground()
        self.overflow = true; // If the overflow of the content is disabled
        // Keeps various data in a cache so that we dont have to calculate them each time the cursor moves
        // {position, limit, numberPage, number_task, page, task, createPage}
        self.taskCache;
        // The coordinates of the content of the task
        self.left;
        self.top;
        self.width;
        self.height;
        
        // Creates the tasks list icon
        self.icon = $("<div></div>")[0];
        $(self.icon).addClass("task");
        self.icon.object = self;
        $(self.icon).mousedown(function (e) { self.mouseDown(e); });
        $(self.icon).mouseup(function (e) { self.mouseUpClose(e); });
        // Adds the content of the icon
        var content = $("<div></div>")[0];
        $(content).addClass("content");
        content.innerHTML = T.Tasks[resource];
        $(content).appendTo(self.icon);
        // Register the task in order to display its buttons
        gl_desktop.taskButtons.addTask(self);
        
        // Creates the content of the task
        self.content = $("<div></div>")[0];
        $(self.content).addClass("task");
        self.content.object = self;
        self.content.innerHTML = html;
        translate(self.content);
        // Events
        $(self.content).mousedown(function (e) { self.mouseDownContent(e); });
        $(self.content).mouseenter(function (e) { self.mouseEnterContent(e); });
        $(self.content).mouseleave(function (e) { self.mouseLeaveContent(e); });
        $(self.content).appendTo(gl_desktop.node.tasks);
        gl_desktop.taskPreview.addTask(self);
    }
    
    // Starts to move the task icon
    self.mouseDown = function (e)
    {
        if (e.which == 1)
        {
            // Handles the case where the mouse is released outside the browser
            if (gl_desktop.drag.isDragging())
                return ;
            gl_desktop.drag.start(e, self.icon, self, "mouseMove", "mouseWheel", "mouseUp");
            self.mouse = gl_desktop.drag.getMouse();
            self.element = gl_desktop.drag.getElement();
            // Takes the scroll into account
            self.mouse.y += gl_desktop.node.tasks_list.scrollTop;
            self.element.y += gl_desktop.node.tasks_list.scrollTop;
            // There is a little resistance before the task begins to move
            self.resistance = true;
            // Gets the position of the task's page in the tasks list
            self.page = 0;
            for (var page = self.icon.parentNode; page; page = page.previousSibling)
                if ($(page).hasClass("page"))
                    self.page++;
            // Saves all the pages displayed
            gl_desktop.sessions.save("task");
        }
    }
    
    // Moves the task icon according to the mouse position.
    self.mouseMove = function (e)
    {
        // The new position of the task icon
        var y = e.pageY - self.mouse.y;
        var x = e.pageX - self.mouse.x;
        var elementY = self.element.y - gl_desktop.node.tasks_list.scrollTop;
        
        // Top limit
        if (y < C.Desktop.topHeight && !self.resistance)
            self.icon.style.top = C.Desktop.topHeight + "px";
        // Bottom limit
        else if (y > C.Desktop.topHeight + gl_desktop.middleHeight - C.Desktop.taskIconHeight && !self.resistance)
            self.icon.style.top = (C.Desktop.topHeight + gl_desktop.middleHeight - C.Desktop.taskIconHeight) + "px";
        // If the resistance is broken, we start to drag the task
        else if (self.resistance && (y < elementY - C.Desktop.taskResistance || y > elementY + C.Desktop.taskResistance
                 || x < self.element.x - C.Desktop.taskResistance || x > self.element.x + C.Desktop.taskResistance))
            self.startDrag(y);
        // Moves the task vertically
        else if (!self.resistance)
            self.icon.style.top = y + "px";
        // Moves the dragged task horizontally if the mouse is outside of the tasks list
        if (e.pageX >= C.Desktop.tasksListWidth)
        {
            self.icon.style.left = e.pageX + C.Desktop.taskDragShift.x + "px";
            self.icon.style.top = Math.min(Math.max(e.pageY + C.Desktop.taskDragShift.y, C.Desktop.topHeight), C.Desktop.topHeight + gl_desktop.middleHeight - C.Desktop.taskIconHeight) + "px";
            // Ensures that the cache exists
            if (!self.taskCache)
                self.updateTasksList(e.pageY);
        }
        // Otherwise updates the tasks list
        else
        {
            self.icon.style.left = C.Desktop.tasksListMargin + "px";
            if (!self.resistance)
                self.updateTasksList(e.pageY);
        }
    }
        
    // Puts a ghost task under the cursor to show the place where the dragged task will be moved.
    // @param y : The y position of the mouse.
    self.updateTasksList = function (y)
    {
        // Changes the reference y depending if the task has been dragged up or down
        y = y - self.mouse.y + gl_desktop.node.tasks_list.scrollTop;
        if (y > self.element.y)
            y += C.Desktop.taskIconHeight;
        
        $(self.ghost).detach();
        // Gets the position of the task under the cursor
        var data = self.getCursorPosition(y);
        var page = data.page;
        var task = data.task;
        var position = data.position;
        // If we want to create a page, the ghost is hidden and the new task is displayed as a full page
        if (data.createPage)
        {
            $(gl_desktop.node.tasks_list.bottom).height(C.Desktop.taskIconHeight);
            $(gl_desktop.node.pages).children(".display").each(function () { this.object.hide(); });
            self.setCoordinates(gl_desktop.left, gl_desktop.top, gl_desktop.width, gl_desktop.height);
            self.display();
            $(self.icon.parentNode).addClass("focus");
            gl_desktop.taskPreview.hide();
            return ;
        }
        // The current page is above the original page
        if (data.numberPage <= self.page)
        {
            if (data.numberPage == self.page && $(self.icon.parentNode.nextSibling).hasClass("page"))
                position -= C.Desktop.pageMargin + 1;
            if (!task.nextSibling && y > position - C.Desktop.taskIconHeight / 2)
                page.appendChild(self.ghost);
            else if (task.nextSibling && y > position - C.Desktop.taskIconHeight / 2)
                page.insertBefore(self.ghost, task.nextSibling);
            else
                page.insertBefore(self.ghost, task);
        }
        // The current page is below the original page
        else
        {
            if (!$(data.page.nextSibling).hasClass("page"))
                position += C.Desktop.pageMargin;
            if (!task.nextSibling && y < position - C.Desktop.pageMargin - C.Desktop.taskIconHeight / 2)
                page.insertBefore(self.ghost, task);
            else if (task.nextSibling && y < position - C.Desktop.pageMargin - C.Desktop.taskIconHeight / 2)
                page.insertBefore(self.ghost, task);
            else if (task.nextSibling)
                page.insertBefore(self.ghost, task.nextSibling);
            else
                page.appendChild(self.ghost);
        }
        // Displays the current page
        if (!page.object.isDisplayed() || (page.object != self.getPage() && self.getPage().isDisplayed()))
        {
            // Hides all the pages
            self.hide();
            $(gl_desktop.node.pages).children(".display").each(function() { this.object.hide(); });
            // Displays the current page but keeps the focus on the original page of the task
            if (page.object != self.getPage())
            {
                page.object.display();
                $(page).removeClass("focus");
                $(self.icon.parentNode).addClass("focus");
            }
            // Restore the state of the session when we clicked on the task icon
            else
            {
                gl_desktop.sessions.load("task").each(function () { this.object.display(); });
                page.object.display();
            }
            // Displays the preview
            gl_desktop.taskPreview.display(undefined, undefined, page.object);
        }
        // If the original page of the task is empty, we ensure that the bottom margin is displayed
        if (self.getPage().numberTasks == 1)
            if (self.ghost.parentNode != self.icon.parentNode)
                $(self.icon.parentNode).addClass("empty");
            else
                $(self.icon.parentNode).removeClass("empty");
        $(gl_desktop.node.tasks_list.bottom).height(0);
    }
    
    // Gets the position of the task under the cursor and some other data.
    // @param y : The y position of the mouse
    self.getCursorPosition = function (y)
    {
        var position;
        var pages, tasks;
        var p, t;
        var margin;
        var createPage = false;
        
        // If the position have already been calculated for this interval, we return the cache
        if (self.taskCache && y < self.taskCache.position && y > self.taskCache.limit && !self.taskCache.createPage)
            return (self.taskCache);
        // Runs through the tasks in order to find the position of the task under the cursor
        pages = $(gl_desktop.node.tasks_list).children(".page");
        position = C.Desktop.topHeight;
        for (p = 0; p < pages.length && (position <= y || !p); p++)
        {
            position += C.Desktop.pageMargin;
            tasks = $(pages[p]).children(".task");
            for (t = 0; t < tasks.length && (position <= y || !t); t++)
                position += C.Desktop.taskIconHeight;
            if (self.page == p + 1)
                position += C.Desktop.pageMargin + 1;
        }
        // Gets the correct nodes
        var page = pages[p - 1];
        var task = tasks[t - 1];
        // Calculates the upper limit
        var limit = position - C.Desktop.taskIconHeight;
        if (!task.previousSibling && !task.nextSibling)
            limit -= C.Desktop.pageMargin;
        else if (!task.previousSibling || !task.nextSibling)
            limit -= C.Desktop.pageMargin / 2;
        if (p == pages.length)
            position -= (C.Desktop.pageMargin + 1);
        // Determines if a new page have to be created
        if (pages.length == p && tasks.length == t && y > position && (p != self.page || self.getPage().numberTasks != 1))
        {
            createPage = true;
            $(self.content).addClass("create_page");
        }
        else
            $(self.content).removeClass("create_page");
        // Saves the result in a cache, so that we dont have to calculate it each time the cursor moves
        self.taskCache = {position : position, limit : limit, numberPage : p, number_task : t, page : page, task : task, createPage : createPage}
        return (self.taskCache);
    }
    
    // Changes the displayed page if we are not on the tasks list and a page is being dragged.
    self.mouseWheel = function (e, delta)
    {
        // We are outside the tasks list
        if (e.pageX >= C.Desktop.tasksListWidth)
        {
            var numberTasks = self.getPage().numberTasks;
            // Moves the ghost up
            if (delta > 0)
            {
                if (!self.ghost.parentNode)
                    $(self.ghost).appendTo($(gl_desktop.node.tasks_list).children(".page:last"));
                else if (self.ghost.previousSibling && self.ghost.previousSibling != self.icon)
                    $(self.ghost).insertBefore(self.ghost.previousSibling);
                else if (self.ghost.previousSibling && self.ghost.previousSibling.previousSibling && numberTasks != 1)
                    $(self.ghost).insertBefore(self.ghost.previousSibling.previousSibling);
                else if ($(self.ghost.parentNode.previousSibling).hasClass("page"))
                    $(self.ghost).appendTo(self.ghost.parentNode.previousSibling);
            }
            // Down
            else
            {
                if (self.ghost.nextSibling && self.ghost.nextSibling != self.icon)
                    $(self.ghost).insertAfter(self.ghost.nextSibling);
                else if (self.ghost.nextSibling && self.ghost.nextSibling.nextSibling && numberTasks != 1)
                    $(self.ghost).insertAfter(self.ghost.nextSibling.nextSibling);
                else if (self.ghost.parentNode && $(self.ghost.parentNode.nextSibling).hasClass("page"))
                    $(self.ghost).prependTo(self.ghost.parentNode.nextSibling);
                else if (self.ghost.parentNode != self.icon.parentNode || numberTasks != 1)
                    $(self.ghost).detach();
            }
            // If the original page of the task is empty, we ensure that the bottom margin is displayed
            if (self.ghost.parentNode != self.icon.parentNode && numberTasks == 1)
                $(self.icon.parentNode).addClass("empty");
            else
                $(self.icon.parentNode).removeClass("empty");
             // Updates the cache
            self.taskCache.numberPage = 0;
            for (var page = self.ghost.parentNode; page; page = page.previousSibling)
                if ($(page).hasClass("page"))
                    self.taskCache.numberPage++;
            if (self.ghost.parentNode)
            {
                self.taskCache.createPage = false;
                $(self.content).removeClass("create_page");
            }
            // New page
            else
            {
                self.taskCache.createPage = true;
                $(self.content).addClass("create_page");
                $(gl_desktop.node.tasks_list.bottom).height(C.Desktop.taskIconHeight);
                $(gl_desktop.node.pages).children(".display").each(function () { this.object.hide(); });
                self.setCoordinates(gl_desktop.left, gl_desktop.top, gl_desktop.width, gl_desktop.height);
                self.display();
                $(self.icon.parentNode).addClass("focus");
                gl_desktop.taskPreview.hide();
                gl_desktop.node.tasks_list.scrollTop = gl_desktop.node.tasks_list.scrollHeight;
                return ;
            }
            // Displays the current page
            var page = self.ghost.parentNode;
            if (!page.object.isDisplayed() || (page.object != self.getPage() && self.getPage().isDisplayed()))
            {
                // Hides all the pages
                self.hide();
                $(gl_desktop.node.pages).children(".display").each(function() { this.object.hide(); });
                // Displays the current page but keeps the focus on the original page of the task
                if (page.object != self.getPage())
                {
                    page.object.display();
                    $(page).removeClass("focus");
                    $(self.icon.parentNode).addClass("focus");
                }
                // Restore the state of the session when we clicked on the task icon
                else
                {
                    gl_desktop.sessions.load("task").each(function () { this.object.display(); });
                    page.object.display();
                }
                // Displays the preview
                gl_desktop.taskPreview.display(undefined, undefined, page.object);
            }
            // Scrolls to the ghost if necessary
            if ($(self.ghost).offset().top < C.Desktop.topHeight)
                gl_desktop.node.tasks_list.scrollTop += $(self.ghost).offset().top - C.Desktop.topHeight;
            else if ($(self.ghost).offset().top + C.Desktop.taskIconHeight > C.Desktop.topHeight + gl_desktop.middleHeight)
                gl_desktop.node.tasks_list.scrollTop = gl_desktop.node.tasks_list.scrollTop + $(self.ghost).offset().top - C.Desktop.topHeight - gl_desktop.middleHeight + C.Desktop.taskIconHeight;
        }
    }

    // Moves the dragged task to the page under the cursor.
    self.mouseUp = function (e)
    {
        var preview = gl_desktop.taskPreview;
        
        // If the task has been dragged
        if (!self.resistance)
        {
            // The task has been moved in a new page
            if (self.taskCache.numberPage != self.page || self.taskCache.createPage)
            {
                // Removes the old task
                self.getPage().removeTask(self, true);
                // Puts the task in a new page
                if (self.taskCache.createPage)
                    (new Page()).addTask(self);
                // Adds the task to the its new page
                else
                    self.ghost.parentNode.object.addTask(self, gl_desktop.taskPreview.position, gl_desktop.taskPreview.task, self.ghost);
                self.getPage().hide();
                self.getPage().display();
            }
            // The page hasn't changed (on the tasks list)
            else if (self.taskCache.numberPage == self.page)
            {
                // The task hasn't moved
                if (!preview.task && !preview.page)
                    $(self.icon).insertBefore(self.ghost);
                // The icon is moved in the same page
                else if (!preview.page && preview.task.getPage() == self.getPage())
                {
                    $(self.icon).insertBefore(self.ghost);
                    var page = self.getPage();
                    page.removeNode(self.node);
                    page.addNode(self, preview.position, preview.task);
                    page.onResize();
                }
                // The task is moved on the edge of the page
                else if (preview.page && preview.page == self.getPage())
                {
                    self.getPage().removeTask(self, true);
                    preview.page.addTask(self, preview.position);
                    $(self.icon).insertBefore(self.ghost);
                    self.focus(true);
                    preview.page.onResize();
                }
                // The task is moved on the edge of another page
                else if (preview.page)
                {
                    self.getPage().removeTask(self, true);
                    preview.page.addTask(self, preview.position);
                    preview.page.onResize();
                }
                // The task is moved in another page
                else
                {
                    self.getPage().removeTask(self, true);
                    preview.task.getPage().addTask(self, preview.position, preview.task, preview.task.icon);
                    preview.task.getPage().onResize();
                }
            }
        }
        // Otherwise the page is hidden if it was already focused before the mouse down event
        else if (!self.getPage().justFocused)
            self.getPage().hide();
        // Clears the context
        self.taskCache = undefined;
        $(self.icon).removeClass("drag");
        $(self.icon.parentNode).removeClass("empty");
        $(self.ghost).remove();
        $(self.content).removeClass("create_page");
        $(gl_desktop.node.tasks_list.bottom).height(0);
        gl_desktop.taskButtons.stopDrag(e);
        gl_desktop.taskPreview.hide(e);
    }
    
    // Closes the task with the middle mouse.
    self.mouseUpClose = function (e)
    {
        if (e.which == 2 && !gl_desktop.drag.isDragging())
        {
            gl_desktop.taskButtons.hideButtons();
            self.getPage().removeTask(self);
        }
    }
    
    // Puts the focus on the page of the task.
    self.mouseDownContent = function (e)
    {
        self.getPage().display();
    }
    
    // The mouse entered the task content.
    self.mouseEnterContent = function (e)
    {
        $(self.icon).addClass("over_content");
    }
    
    // The mouse leaved the task content.
    self.mouseLeaveContent = function (e)
    {
        $(self.icon).removeClass("over_content");
    }
        
    // Displays the content of the task.
    self.display = function ()
    {
        $(self.content).addClass("display");
    }

    // Adds / removes the focus on the content of the task.
    self.focus = function (focus)
    {
        if (focus && $(self.content).hasClass("focus"))
            return ;
        if (focus)
        {
            $(self.content).addClass("focus");
            self.lastFocusDate = new Date().getTime();
        }
        else
        {
            $(self.content).removeClass("focus");
            self.lastFocusDate = 0;
        }
    }
    
    // Hides the content of the task.
    self.hide = function ()
    {
        $(self.content).removeClass("display");
    }

    // Closes the task.
    self.close = function ()
    {
        if (self.resource.close)
            self.resource.close();
        $(self.icon).remove();
        $(self.content).remove();
        self.icon.object = undefined;
        self.content.object = undefined;
        for (var key in self)
            self[key] = undefined;
        gl_desktop.taskButtons.removeTask(self);
    }
    
    // Removes the resistance and starts to drag the task.
    self.startDrag = function (y)
    {
        var elementY = self.element.y - gl_desktop.node.tasks_list.scrollTop

        self.resistance = false;
        // Updates the position of the mouse in the task
        if (y < elementY)
        {
            var delta = elementY - y;
            self.mouse.y -= delta;
            y += delta;
        }
        else if (y > elementY)
        {
            var delta = y - elementY;
            self.mouse.y += delta;
            y -= delta;
        }
        // The mouse is outside the task
        if (self.mouse.y < 0)
        {
            y -= 2 - self.mouse.y;
            self.mouse.y = 2;
        }
        else if (self.mouse.y > C.Desktop.taskIconHeight)
        {
            y += self.mouse.y - C.Desktop.taskIconHeight + 2;
            self.mouse.y = C.Desktop.taskIconHeight - 2;
        }
        // Moves the task
        self.icon.style.top = y + "px";
        $(self.icon).addClass("drag");
        // Creates the ghost
        self.ghost = $("<div></div>")[0];
        $(self.ghost).addClass("ghost");
        $(self.ghost).insertAfter(self.icon);
        gl_desktop.taskButtons.startDrag(self);
        return (y);
    }
    
    // Sets the coordinates of the content of the task.
    self.setCoordinates = function(left, top, width, height)
    {
        // Compensates the presence of the task border
        if (!$(self.content).hasClass("no_border"))
        {
            width = Math.max(width - C.Desktop.taskBorder * 2, 0);
            height = Math.max(height - C.Desktop.taskBorder * 2, 0);
        }
        // Sets the new coordinates
        self.left = left;
        self.top = top;
        self.width = width;
        self.height = height;
        $(self.content).css("left", left);
        $(self.content).css("top", top);
        self.content.style.width = width + "px";
        self.content.style.height = height + "px";
        // Hides the borders when the height is null
        if (self.height <= 0 || self.width <= 0)
            $(self.content).addClass("hide_border");
        else
            $(self.content).removeClass("hide_border");
        // Calls the onResize method of the resource
        if (self.resource.onResize)
            self.resource.onResize(left, top, width, height);
    }
    
    // Changes the container that displays the task.
    self.setContainer = function (container)
    {
        if (container != gl_desktop)
            $(self.content).addClass("window");
        else
            $(self.content).removeClass("window");
        self.focus(false);
    }
    
    // Sets the z-indes of the content of the task.
    self.setZIndex = function (zIndex)
    {
        self.content.style.zIndex = zIndex;
    }
    
    // Returns the page of the task.
    self.getPage = function () { return (self.icon.parentNode.object); }
    
    // Returns true if the task icon is on the new page area.
    self.isOnCreatePage = function ()
    {
        return (self.taskCache && self.taskCache.createPage);
    }

// Resource interface
// The resources have to implement this interface in order to receive the task events.
    {
        // Closes the resource.
        function close() {}
        // The task have been resized.
        function onResize(left, top, width, height) {}
    }

// Resource api
// These methods are called by the resources in order to interact with the task.
    {
        // Sets the instance of the resource that manages the content.
        // Allows the task to call the methods onResize and close of the resource.
        self.setResource = function (resource)
        {
            self.resource = resource;
        }
        
        // Sets the background of the task.
        // @param display : If false the background and the border will be transparent.
        // @param background : The css class that will be applied to the task in order to modify its background and its border.
        self.setBackground = function (display, background)
        {
            // Remove the previous background
            if (self.lastBackgroundSet)
            {
                $(self.content).removeClass(self.lastBackgroundSet);
                delete self.lastBackgroundSet;
            }
            // No background
            if (!display)
                $(self.content).addClass("no_background");
            // Custom background
            else
            {
                $(self.content).removeClass("no_background");
                if (background)
                {
                    self.lastBackgroundSet = background;
                    $(self.content).addClass(background);
                }
            }
        }
        
        // Displays / hides the task borders.
        // @param border : True to display the task border.
        self.setBorder = function (border)
        {
            if (border)
                $(self.content).removeClass("no_border");
            else
                $(self.content).addClass("no_border");
        }
        
        // Allows the resources to disable the overflow of the content.
        // @param overflow : True if the overflow is enabled.
        self.setOverflow = function (overflow)
        {
            if (overflow != self.overflow)
            {
                self.overflow = overflow;
                if (!overflow)
                    $(self.content).addClass("disable_overflow");
                else
                    $(self.content).removeClass("disable_overflow");
            }
        }
        
        // Returns true if the page has the focus.
        self.isFocus = function ()
        {
            // The focus also have to be on the task for more that 10 milliseconds, in order to avoid event order problems
            return ($(self.content).hasClass("focus") && (new Date().getTime() - self.lastFocusDate) > 10);
        }
        
        // Returns true if the page is a window.
        self.isWindow = function ()
        {
            return ($(self.content).hasClass("window"));
        }
    }
        
    self.init();
    return (self);
}

// Represents a node of the tasks tree that allows to display the tasks in a page.
function TaskTreeNode(node)
{
    var self = this;
    
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
    self.init = function ()
    {
        for (var key in node)
            self[key] = node[key];
        self.resize; // The resize bar managed by the node
        self.mouse; // The position of the mouse in resize
        self.element; // The original position of the element
        self.leftNode;
        self.rightNode;
        self.topNode;
        self.bottomNode;
        self.startResize;
    }

    // Creates the resize bar that allows to resize the node.
    // @param pageContent : The page in which the bar is inserted.
    self.createResize = function (pageContent)
    {
        self.resize = document.createElement("div");
        self.resize.className = "resize";
        self.resize.taskNode = self;
        $(self.resize).mousedown(function (e) { self.mouseDown(e); });
        $(self.resize).mouseenter(function (e) { self.mouseEnter(e); });
        $(self.resize).mouseleave(function (e) { self.mouseLeave(e); });
        pageContent.appendChild(self.resize);
    }
    
    // Starts to resize the node.
    self.mouseDown = function (e)
    {
        if (e.which != 1 || gl_desktop.drag.isDragging())
            return ;
        gl_desktop.drag.start(e, self.resize, self, "mouseMove", "mouseWheel", "mouseUp");
        self.mouse = gl_desktop.drag.getMouse();
        self.element = gl_desktop.drag.getElement();
        // Searches nodes that are in the edge of the current node and save their properties
        var previous, level;
        for (var node = self.parent, previous = self, level = 1; node; previous = node, node = node.parent, level++)
            if (self.h == -1 && node.h != -1 && node.second == previous && !self.leftNode)
                self.leftNode = { parent : node, h : node.h, width : node.width, level : level };
            else if (self.h == -1 && node.h != -1 && node.first == previous && !self.rightNode)
                self.rightNode = { parent : node, h : node.h, width : node.width, level : level };
            else if (self.v == -1 && node.v != -1 && node.second == previous && !self.topNode)
                self.topNode = { parent : node, v : node.v, height : node.height, level : level };
            else if (self.v == -1 && node.v != -1 && node.first == previous && !self.bottomNode)
                self.bottomNode = { parent : node, v : node.v, height : node.height, level : level };
        // Ensures that the page has the focus
        self.resize.parentNode.object.display();
        $(self.resize.parentNode).addClass("resize_node");
    }

    // Resizes the node according to the mouse position.
    self.mouseMove = function (e)
    {
        var taskMargin = C.Desktop.taskMargin;
        var x = e.pageX;
        var y = e.pageY;

        // Resizes horizontally
        if (self.h >= 0)
        {
            // Takes into account where the user clicked in the resize bar
            x += taskMargin / 2 - self.mouse.x;
            // Computes the horizontal resize
            self.h = (x - self.left) / Math.max(self.width, 1);
            self.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, self.h));
            // The number of pixels the mouse moved vertically from the click position
            var diff = y - self.mouse.y - self.element.y;
            // Applies a little resistance before starting to resize vertically
            if (!self.startResize && Math.abs(diff) < C.Desktop.resizeResistance)
                diff = 0;
            else
            {
                // Starts to resize vertically
                if (!self.startResize)
                    self.startResize = diff;
                (self.startResize > 0) ? diff -= C.Desktop.resizeResistance : diff += C.Desktop.resizeResistance;
            }
            if (self.topNode)
                self.topNode.parent.v = self.topNode.v;
            if (self.bottomNode)
                self.bottomNode.parent.v = self.bottomNode.v;
            // Resizes vertically the top node
            if (self.topNode && (!self.bottomNode || diff < 0))
            {
                var v = self.topNode.v + (1 - (self.topNode.height - diff) / Math.max(self.topNode.height, 1));
                self.topNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, v));
                // Adjusts the bottom node to the new top node ratio, so that the bottom node doesn't move
                if (self.bottomNode && self.topNode.level > self.bottomNode.level)
                {
                    // Old size of the bottom node second, divided by the new size of the bottom node
                    var a = 1 - (1 - self.bottomNode.v) / ((1 - self.topNode.parent.v) / (1 - self.topNode.v));
                    if (self.topNode.parent.v * self.topNode.height <= taskMargin / 2)
                        a = 1 - (self.bottomNode.height * (1 - self.bottomNode.v)) / ((self.bottomNode.height / (1 - self.topNode.v)) - taskMargin / 2);
                    self.bottomNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
                }
            }
            // Resizes vertically the bottom node
            if (self.bottomNode && (!self.topNode || diff > 0))
            {
                var v = self.bottomNode.v + (1 - (self.bottomNode.height - diff) / Math.max(self.bottomNode.height, 1));
                self.bottomNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, v));
                // Adapts the top node to the new bottom node ratio, so that the top node doesn't move
                if (self.topNode && self.bottomNode.level > self.topNode.level)
                {
                    // Old size of the top node first, divided by the new size of the top node
                    var a = self.topNode.v / (self.bottomNode.parent.v / self.bottomNode.v);
                    if ((1 - self.bottomNode.parent.v) * self.bottomNode.height <= taskMargin / 2)
                        a = (self.topNode.height * self.topNode.v) / ((self.topNode.height / self.bottomNode.v) - taskMargin / 2);
                    self.topNode.parent.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
                }
            }
        }
        // Resizes vertically
        else
        {
            // Takes into account where the user clicked in the resize bar
            y += taskMargin / 2 - self.mouse.y;
            // Computes the vertical resize
            self.v = (y - self.top) / Math.max(self.height, 1);
            self.v = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, self.v));
            // The number of pixels the mouse moved horizontally from the click position
            var diff = x - self.mouse.x - self.element.x;
            // Applies a little resistance before starting to resize horizontally
            if (!self.startResize && Math.abs(diff) < C.Desktop.resizeResistance)
                diff = 0;
            else
            {
                // Starts to resize horizontally
                if (!self.startResize)
                    self.startResize = diff;
                (self.startResize > 0) ? diff -= C.Desktop.resizeResistance : diff += C.Desktop.resizeResistance;
            }
            if (self.leftNode)
                self.leftNode.parent.h = self.leftNode.h;
            if (self.rightNode)
                self.rightNode.parent.h = self.rightNode.h;
            // Resizes horitontally the left node
            if (self.leftNode && (!self.rightNode || diff < 0))
            {
                var h = self.leftNode.h + (1 - (self.leftNode.width - diff) / Math.max(self.leftNode.width, 1));
                self.leftNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, h));
                // Adjusts the right node to the new left node ratio, so that the right node doesn't move
                if (self.rightNode && self.leftNode.level > self.rightNode.level)
                {
                    // Old size of the right node second, divided by the new size of the right node
                    var a = 1 - (1 - self.rightNode.h) / ((1 - self.leftNode.parent.h) / (1 - self.leftNode.h));
                    if (self.leftNode.parent.h * self.leftNode.width <= taskMargin / 2)
                        a = 1 - (self.rightNode.width * (1 - self.rightNode.h)) / ((self.rightNode.width / (1 - self.leftNode.h)) - taskMargin / 2);
                    self.rightNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
                }
            }
            // Resizes horitontally the right node
            if (self.rightNode && (!self.leftNode || diff > 0))
            {
                var h = self.rightNode.h + (1 - (self.rightNode.width - diff) / Math.max(self.rightNode.width, 1));
                self.rightNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, h));
                // Adapts the left node to the new right node ratio, so that the left node doesn't move
                if (self.leftNode && self.rightNode.level > self.leftNode.level)
                {
                    // Old size of the left node first, divided by the new size of the left node
                    var a = self.leftNode.h / (self.rightNode.parent.h / self.rightNode.h);
                    if ((1 - self.rightNode.parent.h) * self.rightNode.width <= taskMargin / 2)
                        a = (self.leftNode.width * self.leftNode.h) / ((self.leftNode.width / self.rightNode.h) - taskMargin / 2);
                    self.leftNode.parent.h = Math.max(C.Desktop.resizeTaskLimitMin, Math.min(C.Desktop.resizeTaskLimitMax, a));
                }
            }
        }
        // Retreives the page of the node
        for (var node = self.first; node; node = node.first)
            if (node.task)
                // Displays the modifications
                node.task.getPage().renderTree();
    }

    // Resizes the tasks margin and the resize bar.
    self.mouseWheel = function (e, delta)
    {
        // Changes the margin between the tasks
        C.Desktop.taskMargin += delta / C.Desktop.resizeDeltaDivisor;
        C.Desktop.taskMargin = Math.max(Math.round(C.Desktop.taskMargin), 1);
        // Takes into account the position of the mouse in the bar
        if (self.h != -1)
            self.mouse.x = C.Desktop.taskMargin / 2;
        else
            self.mouse.y = C.Desktop.taskMargin / 2;
        // Retreives the page of the node
        for (var node = self.first; node; node = node.first)
            if (node.task)
                // Displays the modifications
                node.task.getPage().renderTree();
    }

    // Stops the resize.
    self.mouseUp = function ()
    {
        delete self.leftNode;
        delete self.rightNode;
        delete self.topNode;
        delete self.bottomNode;
        delete self.startResize;
        $(self.resize.parentNode).removeClass("resize_node");
    }

    // The mouse entered the resize bar.
    self.mouseEnter = function (e)
    {
        $(self.resize.parentNode).addClass("over_resize");
    }
    
    // The mouse leaved the resize bar.
    self.mouseLeave = function (e)
    {
        $(self.resize.parentNode).removeClass("over_resize");
    }
    
    self.init();
    return (self);
}

// Manages the buttons of the tasks icons.
function TaskButtons()
{
    var self = this;
    
    self.init = function ()
    {
        self.buttons = $(gl_desktop.node.tasks_list.container).children(".buttons")[0];
        // The buttons
        self.close = $(self.buttons).children(".close")[0];
        self.hide = $(self.buttons).children(".hide")[0];
        self.window = $(self.buttons).children(".window")[0];
        self.fullscreen = $(self.buttons).children(".fullscreen")[0];
        self.task; // The last task on which the mouse entered.
        
        // Generates the SVG icons
        self.addIcon($(self.close).children(".icon")[0], 9.5, gl_svg.TaskButtons.close);
        self.addIcon($(self.hide).children(".icon")[0], 9.5, gl_svg.TaskButtons.hide);
        self.addIcon($(self.window).children(".icon")[0], 9.5, gl_svg.TaskButtons.window);
        self.addIcon($(self.fullscreen).children(".icon")[0], 9, gl_svg.TaskButtons.fullscreen);
        
        // Events
        $(self.close).click(function (e) { self.click(e); });
        $(self.hide).click(function (e) { self.click(e); });
        $(self.window).click(function (e) { self.click(e); });
        $(self.fullscreen).click(function (e) { self.click(e); });
    }
    
    // Creates an icon and adds it to the destination.
    self.addIcon = function (destination, top, path)
    {
        element = Raphael(destination, 20, 18).path(path);
        element.translate(10, top);
        element.attr("fill", "#dddddd");
        element.attr("stroke", "none");
        // Changes the color when the mouse is over the icon
        destination.parentNode.svg = element;
        $(destination.parentNode).mouseenter(function (e)
        {
            e.currentTarget.svg.attr("fill", "#333333");
        });
        $(destination.parentNode).mouseleave(function (e)
        {
            e.currentTarget.svg.attr("fill", "#dddddd");
        });
    }
    
    // Registers the events that allow to display the buttons of the task.
    self.addTask = function (task)
    {
        $(task.icon).mouseenter(function (e) { self.mouseEnter(e, task); });
        $(task.icon).mouseleave(function (e) { self.mouseLeave(e, task); });
    }
    
    // Removes the last task if it has been closed.
    self.removeTask = function (task)
    {
        if (task == self.task)
            self.task = undefined;
    }

    // Activate a button
    self.click = function (e)
    {
        var page = self.task.getPage();
        
        if (e.which != 1)
            return ;
        if (e.currentTarget == self.close)
        {
            // Gets the next task
            var task = $(self.task.icon).next(".task")[0];
            if (!task)
                task = $(self.task.icon.parentNode.nextSibling).children(".task:first")[0];
            // Or the previous task
            if (!task)
                task = $(self.task.icon).prev(".task")[0];
            if (!task)
                task = $(self.task.icon.parentNode.previousSibling).children(".task:last")[0];
            // Removes the old task
            self.task.getPage().removeTask(self.task);
            // Puts the focus on the new task
            if (task)
            {
                $(task).addClass("over");
                self.task = task.object;
            }
            else
                $(self.buttons).removeClass("display");
        }
        else if (e.currentTarget == self.hide)
        {
            if (page.isDisplayed())
                self.task.getPage().hide();
            else
                self.task.getPage().display();
        }
        else if (e.currentTarget == self.window)
        {
            if (page.getContainer() == gl_desktop)
                gl_windows.open(page);
            else
                page.setContainer(gl_desktop);
            page.onResize();
            page.display();
        }
    }
    
    // Displays the buttons while the mouse is on the task icon.
    self.mouseEnter = function (e, task)
    {
        if (!gl_desktop.drag.isDragging())
        {
            self.buttons.style.top = $(task.icon).offset().top + "px";
            self.task = task;
            self._display();
        }
    }

    // Hides the buttons when the mouse leaves the task icon.
    self.mouseLeave = function (e, task)
    {
        // If the mouse has not entered the buttons area
        if (!$(self.buttons).has(e.relatedTarget).length)
            self._hide(task);
        // Otherwise we will hide the buttons when the mouse leaves it
        else
            $(self.buttons).mouseleave(function (e)
            {
                // If we are not on the task icon
                if (!$(task.icon).has(e.relatedTarget).length)
                    self._hide(task);
                // The event must be removed
                $(self.buttons).off("mouseleave");
            });
    }
        
    // Hides the buttons while the task is being dragged.
    // @param task : The task being dragged.
    self.startDrag = function (task)
    {
        self._hide(task);
    }
    
    // When the drag is finished, we display the buttons if the mouse is over the task
    self.stopDrag = function (e)
    {
        var offset = $(self.task.icon).offset();
        if (e.pageY > offset.top && e.pageY < offset.top + C.Desktop.taskIconHeight
            && e.pageX > offset.left && e.pageX < offset.left + $(self.task.icon).width())
        {
            self.buttons.style.top = $(self.task.icon).offset().top + "px";
            self._display();
        }
        else
            $(self.task.icon).removeClass("over");
    }

    // Updates the position of the buttons according to the task icon.
    self.updatePosition = function ()
    {
        if (self.task)
            self.buttons.style.top = $(self.task.icon).offset().top + "px";
    }

    // Ensures that the buttons are not displayed while the tasks list is scrolling.
    self.startScroll = function ()
    {
        var tasks_list = gl_desktop.node.tasks_list;
        if (tasks_list.scrollTop > 0 && tasks_list.scrollTop < tasks_list.scrollHeight - gl_desktop.middleHeight)
            $(self.buttons).addClass("scroll");
        else
            self.stopScroll();
    }

    // Displays the buttons when the scroll is finished
    self.stopScroll = function ()
    {
        var task;
        var offset;
        var mouse = gl_desktop.mouse;
        
        $(self.buttons).removeClass("scroll");
        // If we are on the scrolling top limit, gets the first task
        if (gl_desktop.node.tasks_list.scrollTop <= 0 && mouse.y < C.Desktop.topHeight + C.Desktop.tasksListScrollHeight)
            task = $(gl_desktop.node.tasks_list).children(".page:first").children(".task:first")[0];
        // If we are on the scrolling bottom limit, gets the last task
        else if (gl_desktop.node.tasks_list.scrollTop >= gl_desktop.node.tasks_list.scrollHeight - gl_desktop.middleHeight && mouse.y > C.Desktop.topHeight + gl_desktop.middleHeight - C.Desktop.tasksListScrollHeight)
            task = $(gl_desktop.node.tasks_list).children(".page:last").children(".task:last")[0];
        // Then displays its buttons
        if (task && !gl_desktop.drag.isDragging())
        {
            $(self.task.icon).removeClass("over");
            self.task = task.object;
            var offset = $(self.task.icon).offset();
            // If the mouse is on the task
            if (mouse.y > offset.top && mouse.y < offset.top + C.Desktop.taskIconHeight
                && mouse.x > offset.left && mouse.x < offset.left + $(self.task.icon).width())
                self._display();
        }
        self.buttons.style.top = $(self.task.icon).offset().top + "px";
    }
    
    // Hides the buttons
    self.hideButtons = function ()
    {
        $(self.buttons).removeClass("display");
    }
    
    // Displays the buttons and highlights the content of the task
    self._display = function ()
    {
        $(self.buttons).addClass("display");
        $(self.task.icon).addClass("over");
        // The highlight on the content is only added if there is more than one task in the page
        if (self.task.getPage().numberTasks > 1)
            $(self.task.content).addClass("highlight");
        else
            $(self.task.content).removeClass("highlight");
    }
    // Hides the buttons and the highlight
    self._hide = function (task)
    {
        $(self.buttons).removeClass("display");
        $(task.icon).removeClass("over");
        $(task.content).removeClass("highlight");
    }
    
    self.init();
    return (self);
}

// Manages the scroll of the tasks list.
function TasksList()
{
    var self = this;
    
    self.init = function ()
    {
        self.tasks_list = gl_desktop.node.tasks_list;
        self.over = false; // True while the mouse is over the tasks list
        self.delta; // The position of the mouse in the scrolling area
        self.interval; // The interval that actually scroll the tasks list
        
        // Events
        $(self.tasks_list).mouseenter(function (e) { self.mouseEnter(e); });
        $(self.tasks_list).mouseleave(function (e) { self.mouseLeave(e); });
        $(self.tasks_list).mousemove(function (e) { self.mouseMove(e); });
        $(self.tasks_list).click(function (e) { self.click(e); });
    }
    
    // The mouse entered the tasks list.
    self.mouseEnter = function (e)
    {
        self.over = true;
    }
    
    // The mouse leaved the tasks list.
    self.mouseLeave = function (e)
    {
        self.over = false;
        self.stopScroll();
    }
    
    // Manages the scrolling areas at the top and the bottom of the tasks list.
    self.mouseMove = function (e)
    {
        // The top scrolling area of the tasks list
        if (e.pageY > C.Desktop.topHeight && e.pageY <= C.Desktop.topHeight + C.Desktop.tasksListScrollHeight)
            self.scroll(1 - (e.pageY - C.Desktop.topHeight) / C.Desktop.tasksListScrollHeight);
        // The bottom scrolling area of the tasks list
        else if (e.pageY >= C.Desktop.topHeight + gl_desktop.middleHeight - C.Desktop.tasksListScrollHeight && e.pageY < C.Desktop.topHeight + gl_desktop.middleHeight)
            self.scroll((C.Desktop.topHeight + gl_desktop.middleHeight - C.Desktop.tasksListScrollHeight - e.pageY) / C.Desktop.tasksListScrollHeight);
        // Ensures that the scrolling is stopped outside of the scrolling areas
        else if (self.interval)
            self.stopScroll();
    }

    // Scrolls the tasks list using the mouse wheel.
    self.mouseWheel = function (delta)
    {
        if (self.over)
        {
            self.tasks_list.scrollTop -= delta;
            gl_desktop.taskButtons.updatePosition();
            if (gl_desktop.drag.isDragging("Task"))
                gl_desktop.drag.getObject().updateTasksList(gl_desktop.mouse.pageY);
        }
    }
    
    // Hides all the pages, or displays them back.
    self.click = function (e)
    {
        if (e.which != 1 || e.target != self.tasks_list)
            return ;
        // Saves the pages displayed and hide the them
        var pages = gl_desktop.sessions.save("tasksList").each(function () { this.object.hide(); });
        // If no page has been hidden we display them back
        if (pages.length == 0)
            gl_desktop.sessions.load("tasksList").each(function () { this.object.display(); });
    }
    
    // Scrolls the tasks list up or down, depending on the delta.
    // @param delta : The speed of the scroll. Scroll top if positive, down if negative.
    // Should be between 1 and -1.
    self.scroll = function (delta)
    {
        var height = self.tasks_list.scrollHeight - gl_desktop.middleHeight;
        
        self.delta = delta;
        // Nothing to scroll
        if (!height || !delta || (delta > 0 && self.tasks_list.scrollTop == 0) || (delta < 0 && self.tasks_list.scrollTop == height))
            self.stopScroll();
        // Creates the interval that will scroll the tasks
        else if (!self.interval)
            self.interval = setInterval("gl_desktop.tasksList.scrollInterval()", 1000 / 30);
        if (self.interval)
            gl_desktop.taskButtons.startScroll();
    }
    // This method is called by setInterval and scrolls the tasks list according to the delta.
    self.scrollInterval = function ()
    {
        var height = self.tasks_list.scrollHeight - gl_desktop.middleHeight;

        if (!self.interval)
            return ;
        // Calculates the number of pixels to scroll
        if (self.delta > 0)
            self.tasks_list.scrollTop -= (Math.exp(Math.abs(self.delta)) - 1) * C.Desktop.tasksListScrollSpeed;
        else
            self.tasks_list.scrollTop += (Math.exp(Math.abs(self.delta)) - 1) * C.Desktop.tasksListScrollSpeed;
        // Updates the position of the dragged task or page
        if (gl_desktop.drag.isDragging("Task"))
            gl_desktop.drag.getObject().updateTasksList(gl_desktop.mouse.pageY);
        else if (gl_desktop.drag.isDragging("Page"))
            gl_desktop.drag.getObject().mouseMove(undefined, gl_desktop.mouse.pageY);
        // Scroll up finished
        if (self.tasks_list.scrollTop <= 0)
        {
            self.tasks_list.scrollTop = 0;
            self.stopScroll();
        }
        // Scroll down finished
        else if (self.tasks_list.scrollTop >= height)
        {
            self.tasks_list.scrollTop = height;
            self.stopScroll();
        }
    }

    // Stops the scrolling of the tasks list.
    self.stopScroll = function ()
    {
        if (self.interval)
        {
            clearInterval(self.interval);
            delete self.interval;
            delete self.delta;
            gl_desktop.taskButtons.stopScroll();
        }
    }
    
    self.init();
    return (self);
}

// Displays a preview while a task icon is being dragged, to show where the it will be dropped.
function TaskPreview()
{
    var self = this;
    
    self.init = function ()
    {
        self.position; // The position of the preview (n s e w)
        self.task; // The task on which the preview is applied
        self.page; // The page on which the preview is applied (when task is undefined)
        self.currentTask; // The last task on which the mouse moved
    }
    
    // Registers the events that allow to display the preview.
    self.addTask = function (task)
    {
        $(task.content).mousemove(function (e) { self.mouseMoveContent(e); });
        $(task.content).mouseleave(function (e) { self.mouseLeaveContent(e); });
        $(gl_desktop.node.preview).mousemove(function (e) { self.mouseMoveContent(e); });
        $(gl_desktop.node.preview).mouseleave(function (e) { self.mouseLeavePreview(e); });
        $(task.icon).mouseleave(function (e) { self.mouseLeaveIcon(e); });
    }
    
    // The mouse moved over the content of a task, while a task icon is being dragged.
    // Displays a preview of the future position of the dragged task in the content.
    self.mouseMoveContent = function (e)
    {
        if (!gl_desktop.drag.isDragging("Task"))
            return ;
        if (e.currentTarget != gl_desktop.node.preview)
            self.currentTask = e.currentTarget.object;
        if (!self.currentTask && !self._searchCurrentTask(e))
            return self.hide();
        var width = self.currentTask.width;
        var height = self.currentTask.height;
        var x = e.pageX - self.currentTask.left;
        var y = e.pageY - self.currentTask.top;
        var ratio = width / height;
        var task;
        var oldPosition = self.position;
        var oldTask = self.task;
        var oldPage = self.page;

        self.position = undefined;
        self.task = undefined;
        self.page = self.currentTask.getPage();
        // If there is more than one task in the page
        if (self.page.numberTasks > 1)
            // We display the preview in the edge of the page
            if (e.pageX - self.page.left < C.Desktop.insertTaskAreaSize)
                self.position = "w";
            else if (e.pageX - self.page.left > self.page.width - C.Desktop.insertTaskAreaSize)
                self.position = "e";
            else if (e.pageY - self.page.top < C.Desktop.insertTaskAreaSize)
                self.position = "n";
            else if (e.pageY - self.page.top > self.page.height - C.Desktop.insertTaskAreaSize)
                self.position = "s";
        // Otherwise we display the preview on the current task
        if (!self.position)
        {
            // If we are on the content of the dragged task the preview is not displayed
            if (self.currentTask == gl_desktop.drag.getObject())
                return self.hide();
            self.position = oldPosition;
            self.task = self.currentTask;
            self.page = undefined;
            // In the corner of the task we use the diagonal to get the position
            if (!self.position || y < height / 3 && x < width / 3 || y < height / 3 && x > width * 2 / 3
                               || y > height * 2 / 3 && x < width / 3 || y > height * 2 / 3 && x > width * 2 / 3)
            {
                if (x > y * ratio && (width - x) > y * ratio)
                    self.position = "n";
                else if (x > y * ratio)
                    self.position = "e";
                else if ((width - x) > y * ratio)
                    self.position = "w";
                else
                    self.position = "s";
            }
            // Facilitates the transition east/west
            else if (self.position == "e" || self.position == "w")
            {
                if (y >= height / 3 && y <= height * 2 / 3 && x <= width / 2)
                    self.position = "w";
                else if (y >= height / 3 && y <= height * 2 / 3 && x >= width / 2)
                    self.position = "e";
                else if (y < height / 2)
                    self.position = "n";
                else
                    self.position = "s";
            }
            // Facilitates the transition north/south
            else
            {
                if (x >= width / 3 && x <= width * 2 / 3 && y <= height / 2)
                    self.position = "n";
                else if (x >= width / 3 && x <= width * 2 / 3 && y >= height / 2)
                    self.position = "s";
                else if (x < width / 2)
                    self.position = "w";
                else
                    self.position = "e";
            }
        }
        // If the position has changed, the preview is displayed
        if (oldPosition != self.position || oldTask != self.task || oldPage != self.page)
            self.display(self.position, self.task, self.page);
    }
    
    // When a task is dragged over a page or an other task, this method displays a preview of its future position.
    // @param position : The future position of the task (n s e w).
    // @param task : The task on which the preview is applied. If undefined, the preview is shown on the page.
    // @param page : If task is not defined, the preview is displayed for this page.
    self.display = function (position, task, page)
    {
        var drag = gl_desktop.drag.getObject();
        
        var preview = gl_desktop.node.preview;
        if (task)
            page = task.getPage();
        // No preview is needed if the task or the page are the same
        if ((task && drag == task) || (((!task && !position) || page.numberTasks == 1) && drag.getPage() == page) || drag.isOnCreatePage())
            return self.hide();
        // Displays the preview on a task
        else if (task)
        {
            // Displays the preview based on the position of the mouse in the task
            preview.style.top = task.top + "px";
            preview.style.left = task.left + "px";
            preview.style.width = task.width + "px";
            preview.style.height = task.height + "px";
            preview.style.zIndex = task.content.style.zIndex;
            $(preview).addClass("display");
            if (position == "n")
                preview.style.height = Math.ceil(task.height / 2) + "px";
            if (position == "s")
            {
                preview.style.top = task.top + Math.floor(task.height / 2) + 1 + "px"; // The +1 takes into account the border
                preview.style.height = Math.ceil(task.height / 2) - 1 + "px";
            }
            if (position == "w")
                preview.style.width = Math.ceil(task.width / 2) + "px";
            if (position == "e")
            {
                preview.style.left = task.left + Math.floor(task.width / 2) + 1 + "px";
                preview.style.width = Math.ceil(task.width / 2) - 1 + "px";
            }
        }
        // Displays the preview on the page
        else if (position)
        {
            // Displays the preview
            preview.style.top = page.top + "px";
            preview.style.left = page.left + "px";
            preview.style.width = page.width + "px";
            preview.style.height = page.height + "px";
            preview.style.zIndex = page.content.style.zIndex;
            $(preview).addClass("display");
            if (position == "n" || position == "s")
                preview.style.height = C.Desktop.insertTaskAreaSize + "px";
            if (position == "s")
                preview.style.top = page.top + (page.height - C.Desktop.insertTaskAreaSize) + "px";
            if (position == "w" || position == "e")
                preview.style.width = C.Desktop.insertTaskAreaSize + "px";
            if (position == "e")
                preview.style.left = page.left + (page.width - C.Desktop.insertTaskAreaSize) + "px";
        }
        // Uses the default position
        else
        {
            position = C.Desktop.defaultPosition;
            // Moves the preview to a node in static position
            $(gl_desktop.node.desktop).append(preview);
            // Displays the preview
            preview.style.top = page.top + "px";
            preview.style.left = page.left + "px";
            preview.style.width = page.width + "px";
            preview.style.height = page.height + "px";
            preview.style.zIndex = page.content.style.zIndex;
            $(preview).addClass("display");
            // The number of horizontal and vertical branches in the tree helps to compute the size
            var h = (page.countNode(page.tree, "h") + 2);
            var v = (page.countNode(page.tree, "v") + 2);
            // The size of the preview is based on the position of the mouse in the page
            if (position == "n" || position == "s")
                preview.style.height = page.height / v + "px";
            if (position == "s")
                preview.style.top = page.top + page.height * (v - 1) / v + "px";
            if (position == "w" || position == "e")
                preview.style.width = page.width / h + "px";
            if (position == "e")
                preview.style.left = page.left + page.width * (h - 1) / h + "px";
        }
        // Sets the style of the preview
        $(preview).removeClass("focus");
        $(preview).removeClass("window");
        if ($(page.icon).hasClass("focus"))
            $(preview).addClass("focus");
        else if ($(page.icon).hasClass("window"))
            $(preview).addClass("window");
        // Updates the state of the preview
        self.position = position;
        self.task = task;
        self.currentTask = task;
        self.page = page;
        if (task)
            self.page = undefined;
    }
    
    // The mouse leaved the task content.
    self.mouseLeaveContent = function (e)
    {
        if (!gl_desktop.drag.isDragging("Task"))
            return ;
        // If the mouse is really out of the content
        if (e.relatedTarget != gl_desktop.node.preview
            && !$(gl_desktop.drag.getObject().icon).has(e.relatedTarget).length
            && (!$(e.relatedTarget).hasClass("resize") || !$(e.relatedTarget.parentNode).hasClass("page")))
        {
            self.hide();
            var page;
            // Displays the default drop position
            if (gl_desktop.drag.isDragging("Task") && (page = gl_desktop.drag.getObject().ghost.parentNode))
                gl_desktop.taskPreview.display(undefined, undefined, page.object);
        }
    }
    
    // The mouse leaved the preview.
    self.mouseLeavePreview = function (e)
    {
        if (!gl_desktop.drag.isDragging("Task"))
            return ;
        // If the mouse is really out of the task content
        if (!$(gl_desktop.drag.getObject().icon).has(e.relatedTarget).length
            && (!self.task || !$(self.task.content).has(e.relatedTarget).length)
            && (!$(e.relatedTarget).hasClass("resize") || !$(e.relatedTarget.parentNode).hasClass("page")))
        {
            self.hide();
            var page;
            if (gl_desktop.drag.isDragging("Task") && (page = gl_desktop.drag.getObject().ghost.parentNode))
                gl_desktop.taskPreview.display(undefined, undefined, page.object);
        }
    }
    
    // The mouse leaved the icon.
    self.mouseLeaveIcon = function (e)
    {
        if (!gl_desktop.drag.isDragging("Task"))
            return ;
        // If the mouse is really out of the task content
        if (e.relatedTarget != gl_desktop.node.preview
            && (!self.task || !$(self.task.content).has(e.relatedTarget).length)
            && (!$(e.relatedTarget).hasClass("resize") || !$(e.relatedTarget.parentNode).hasClass("page")))
        {
            self.hide();
            var page;
            if (gl_desktop.drag.isDragging("Task") && (page = gl_desktop.drag.getObject().ghost.parentNode))
                gl_desktop.taskPreview.display(undefined, undefined, page.object);
        }
    }
    
    // Hides the preview.
    self.hide = function ()
    {
        $(gl_desktop.node.preview).removeClass("display");
        delete self.task;
        delete self.position;
        delete self.page;
        delete self.currentTask;
    }
    
    // Searches the task over which the mouse is.
    self._searchCurrentTask = function (e)
    {
        if (!self.page)
            return (false);
        var tasks = $(self.page.icon).children(".task");
        for (var i = 0; i < tasks.length; ++i)
        {
            var task = tasks[i].object;
            if (task.left <= e.pageX && task.top <= e.pageY
                && task.left + task.width + 2 >= e.pageX && task.top + task.height + 2 >= e.pageY)
            {
                self.currentTask = task;
                self.page = undefined;
                return (true);
            }
        }
        return (false);
    }
    
    self.init();
    return (self);
}

// Manages the sessions.
function Sessions()
{
    var self = this;
    
    self.init = function ()
    {
        self.pages = new Object(); // The list of the pages saved.
    }
    
    // Saves the pages displayed.
    // @param name : The name of the session saved.
    // @return The pages saved.
    self.save = function (name)
    {
        var pages = $(gl_desktop.node.pages).children(".display");
        // Sorts the pages by z-index
        pages.sort(function (a, b)
        {
            if (a.object.zIndex > b.object.zIndex)
                return (1);
            else if (a.object.zIndex < b.object.zIndex)
                return (-1);
            return (0);
        });
        if (pages.length)
            self.pages[name] = pages;
        return (pages);
    }
    
    // Returns the pages saved.
    // @param name : The name of the session to load.
    self.load = function (name)
    {
        if (!self.pages[name])
            return ($());
        // Removes the pages that no longer exists
        var pages = new Array();
        self.pages[name].each(function () { if (this.parentNode) pages.push(this); })
        return ($(pages));
    }
    
    self.init();
    return (self);
}

// Helps to drag an object (only one at a time).
function Drag()
{
    var self = this;
    
    self.init = function ()
    {
        self.object; // The dragged object
        self.objectName; // The name of its class
        self.mouse = { x : 0, y : 0 }; // The position of the mouse in the element
        self.element = { x : 0, y : 0 }; // The initial position of the element
        // Callbacks
        self.mouseMoveCallback;
        self.mouseUpCallback;
        self.mouseWheelCallback;
        self.parameter;
    }

    // Starts to drag an object.
    // @param e : The event that triggered the drag.
    // @param node : The html node to drag.
    // @param object : The object to drag.
    // @param mouseMove : The name of a method in the object to call when the mouse moves.
    // @param mouseWheel : The name of a method in the object to call when the mouse wheel clicks. Optional.
    // @param mouseUp : The name of a method in the object to call when the drag is finished. Optional.
    // @param parameter : A parameter that will be passed to the callbacks. Optional.
    self.start = function (e, node, object, mouseMove, mouseWheel, mouseUp, parameter)
    {
        var mouse = { x : e.pageX, y : e.pageY };
        var element = $(node).offset();
        
        // If something was already being dragged, we stop it
        if (self.isDragging())
            self._stop();
        // The coordinates of the mouse inside the node
        self.mouse = {x : mouse.x - element.left,
                      y : mouse.y - element.top};
        // The absolute coordinates of the node
        self.element = {x : element.left,
                        y : element.top};
        self.object = object;
        self.objectName = getObjectName(object);
        self.mouseMoveCallback = mouseMove;
        self.mouseUpCallback = mouseUp;
        self.mouseWheelCallback = mouseWheel;
        self.parameter = parameter;
        // This class is set to the desktop while something is being dragged
        $(gl_desktop.node.desktop).addClass("drag");
        // The selection is disabled during the drag
        disableSelection(false);
    }

    // Called when the mouse moves.
    self.mouseMove = function (e)
    {
        if (self.object && self.mouseMoveCallback)
            self.object[self.mouseMoveCallback](e, self.parameter);
    }

    // Called when the mouse wheel clicks.
    self.mouseWheel = function (e, delta)
    {
        if (self.object && self.mouseWheelCallback)
            self.object[self.mouseWheelCallback](e, delta, self.parameter);
    }

    // Called when the mouse is up (the drag is finished).
    self.mouseUp = function (e)
    {
        if (e.which != 1)
            return ;
        if (self.object && self.mouseUpCallback)
            stop = self.object[self.mouseUpCallback](e, self.parameter);
         self._stop();
    }

    // Stops the dragging of the object.
    self._stop = function ()
    {
        self.object = undefined;
        self.objectName = undefined;
        self.mouseMoveCallback = undefined;
        self.mouseUpCallback = undefined;
        self.mouseWheelCallback = undefined;
        self.parameter = undefined;
        self.mouse = { x : 0, y : 0 };
        self.element = { x : 0, y : 0 };
        $(gl_desktop.node.desktop).removeClass("drag");
        disableSelection(true);
    }

    // True if the object is being dragged
    // @param object : If object is a string, returns true if the dragged object is the same type.
    // If it is an object, returns true if the dragged object is the same.
    // Otherwise returns true if something is being dragged.
    self.isDragging = function (object)
    {
        if (!object && self.object)
            return (true);
        else if (!self.object)
            return (false);
        else if (self.object === object)
            return (true);
        else if (self.objectName === object)
            return (true);
        return (false);
    }
    // The position of the mouse inside the dragged element.
    // @return : { x, y }
    self.getMouse = function () { return (self.mouse); }
    // The initial position of the dragged element.
    // @return : { x, y }
    self.getElement = function () { return (self.element); }
    // Returns the instance of the object being dragged.
    self.getObject = function () { return (self.object); }

    self.init();
    return (self);
}
