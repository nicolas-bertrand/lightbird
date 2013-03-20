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
        self.node.top = $("#desktop>.top");
        self.node.middle = $("#desktop>.middle");
        self.node.middle_area = self.node.middle.children(".area");
        self.node.middle_background = self.node.middle_area.children(".background");
        self.node.pages = self.node.middle.children(".pages");
        self.node.tasks = self.node.middle.children("#tasks");
        self.node.preview = $("#desktop>#preview")[0];

        // Members
        self.events = new Events(); // Manages the events
        self.drag = new Drag(); // Allows to drag elements
        self.taskPreview = new TaskPreview(); // Displays a preview while a task icon is being dragged
        self.sessions = new Sessions(); // Manages the sessions
        self.mouse = { x : 0, y : 0 }; // The position of the mouse on the desktop
        self.currentPage = undefined; // The page currently displayed by the desktop
        self.fullScreen = false; // True while we are in full screen mode.
        // The coordinates of the main area of the desktop
        self.left = C.TasksList.defaultWidth;
        self.right = 0;
        self.top = C.Header.defaultHeight;
        self.bottom = C.Player.defaultHeight;
        self.width;
        self.height;
        
        // Events
        self.events.bind("mousemove", self, function (e) { self.mouse = {pageX: e.pageX, pageY: e.pageY}; });
    }
    
    // Called when the browser is resized. Updates the size of the desktop.
    self.onResize = function ()
    {
        self.width = gl_browserSize.width - self.left - self.right;
        self.height = gl_browserSize.height - self.top - self.bottom;
        self.node.middle.css({marginTop: self.top, height: self.height});
        self.node.middle_background.css({left: self.left, width: self.width});
        if (self.currentPage)
            self.currentPage.onResize();
        gl_player.onResize(gl_browserSize.width, gl_browserSize.height);
        gl_tasksList.onResize(gl_browserSize.width, gl_browserSize.height);
        gl_windows.onResize(gl_browserSize.width, gl_browserSize.height);
    }
    
    // We entered/leaved the full screen mode.
    // @param fullScreen: True if we are entering the full screen mode, and false for the normal mode.
    self.onFullScreen = function (fullScreen)
    {
        gl_header.onFullScreen(fullScreen);
        gl_player.onFullScreen(fullScreen);
        gl_tasksList.onFullScreen(fullScreen);
        gl_windows.onFullScreen(fullScreen);
    }
    
    // Called when the user has been authentified.
    self.onConnect = function ()
    {
        gl_files.onConnect();
    }
    
    // Called when the user is disconnecting.
    self.onDisconnect = function ()
    {
        gl_files.onDisconnect();
        // Closes all the pages and their tasks.
        var pages = gl_tasksList.getPages();
        pages.each(function () { this.object.close(); });
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
            // Ensures that the tasks list is displayed
            gl_tasksList.display();
            // Creates the page and the task
            var page = new Page();
            var task = new Task(resource, html);
            page.addTask(task);
            gl_resources.callJs(resource, task, parameter);
            page.display();
            gl_tasksList.scrollToBottom();
        });
    }
    
    // Switches between the normal and the full screen modes.
    self.setFullScreen = function ()
    {
        self.fullScreen = !self.fullScreen;
        if (self.fullScreen)
        {
            // The desktop takes all the browser space in full screen
            self.left = 0;
            self.right = 0;
            self.top = 0;
            self.bottom = 0;
            self.onFullScreen(true);
            self.onResize();
            // Asks the browser to go in full screen mode
            if (C.Desktop.browserFullScreen)
            {
                var e = document.body;
                ((e.mozRequestFullScreen && e.mozRequestFullScreen()) ||
                 (e.webkitRequestFullScreen && e.webkitRequestFullScreen()) ||
                 (e.requestFullScreen && e.requestFullScreen()))
            }
        }
        else
        {
            self.left = gl_tasksList.width;
            self.right = 0;
            self.top = gl_header.height;
            self.bottom = gl_player.height;
            self.onFullScreen(false);
            self.onResize();
            // Asks the browser to quit the full screen mode
            var e = document;
            ((e.mozCancelFullScreen && e.mozCancelFullScreen()) ||
             (e.webkitCancelFullScreen && e.webkitCancelFullScreen()) ||
             (e.cancelFullScreen && e.cancelFullScreen()))
        }
    }
    
    // Returns true if we are in full screen mode.
    self.isFullScreen = function ()
    {
        return (self.fullScreen);
    }
    
    // Sets the bottom margin of the desktop. Has no effect in full screen mode.
    self.setBottom = function (bottom)
    {
        if (!self.fullScreen)
        {
            self.bottom = bottom;
            gl_desktop.onResize();
        }
    }
    
    // Sets the page displayed by the desktop.
    // page: The new current page. If undefined, no page is displayed by the desktop.
    self.setCurrentPage = function (page)
    {
        if (page)
        {
            self.currentPage = page;
            //self.node.desktop.addClass("page_displayed");
            self.node.middle_background.addClass("display");
            gl_player.onDesktopPage(true);
        }
        else
        {
            self.currentPage = undefined;
            self.node.desktop.removeClass("page_displayed");
            self.node.middle_background.removeClass("display");
            gl_player.onDesktopPage(false);
        }
    }
    
// Container API
// These methods allow the pages to interact with their container.
    {
        // Changes the page displayed by the desktop.
        // @param page : The page to display.
        self.display = function(page)
        {
            // Hides the windows
            gl_windows.hide();
            if (page != self.currentPage)
            {
                // Hides the old page
                self.hide();
                // Sets the new page
                self.setCurrentPage(page);
                $(self.currentPage.icon).removeClass("window");
                self.currentPage.setZIndex(0);
            }
        }

        // Hides the page currently displayed by the desktop.
        self.hide = function ()
        {
            if (self.currentPage)
            {
                var currentPage = self.currentPage;
                self.setCurrentPage();
                currentPage.hide();
            }
        }
        
        // Notifies the desktop that a page has been closed.
        // @parem page : The page closed.
        self.close = function (page)
        {
            if (self.currentPage == page)
                self.setCurrentPage();
        }
        
        // Notifies that the desktop is no longer the container of the page.
        // @parem page : The page concerned.
        self.containerChanged = function(page)
        {
            if (self.currentPage == page)
                self.setCurrentPage();
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
        self.activeArea; // The active area of the page icon, above the tasks. Used to move and close the page.
        self.content; // The content of the page (contains its resize bars)
        self.tree; // The tree allows to render the tasks of the page. It consists of TaskTreeNode objects.
        self.container = gl_desktop; // The container that displays the page
        self.zIndex = 0; // The z-index of the content of the page
        self.justFocused = false; // True if the page was not focused before the mouse down event
        self.oldWindowCoordinates; // The coordinates of the last time the page was in a window
        // Drag properties
        self.element; // The initial position of the icon {x, y}
        self.mouse; // The position of the mouse in the icon {x, y}
        self.target; // A node that shows the future position of the page while it is dragged
        self.pagesPositions; // The positions of the pages when the drag started. Array({y: , node: })
        // The coordinates of the page
        self.left;
        self.top;
        self.width;
        self.height;
    
        // Creates the icon and insert it in the tasks list
        self.icon = $("<div></div>").addClass("page");
        self.icon[0].object = self;
        // Creates the active area of the icon
        self.activeArea = $("<div></div>").addClass("active_area");
        self.activeArea.appendTo(self.icon);
        self.activeArea.mouseenter(function (e) { self.icon.addClass("over"); });
        self.activeArea.mouseleave(function (e) { if (!gl_desktop.drag.isDragging()) self.icon.removeClass("over"); });
        self.activeArea.mouseup(function (e) { self.mouseUpClose(e); });
        self.icon.mousedown(function (e) { self.mouseDown(e); });
        self.icon.insertBefore(gl_tasksList.node.bottom);
        
        // Creates the content of the page
        self.content = $("<div></div>").addClass("page");
        self.content[0].object = self;
        self.content.appendTo(gl_desktop.node.pages);
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
            task.icon.insertBefore(beforeIcon);
        else
            self.icon.append(task.icon);
        self.numberTasks++;
        self.updateIconHeight();
        task.setContainer(self.container);
        task.setZIndex(self.zIndex);
    }
    
    // Displays the page and its tasks.
    self.display = function ()
    {
        // Displays the page
        self.container.display(self);
        if (!self.isDisplayed())
        {
            // Display the page
            self.content.addClass("display");
            // Displays the tasks of the page
            self.icon.children(".task").each(function () { this.object.display(); });
            // Updates the coordinates
            self.onResize();
        }
        // Puts the focus on it
        var pages = gl_tasksList.node.icons.children(".page.focus").removeClass("focus");
        pages.each(function () { if (this != self.icon[0]) $(this).children(".task").each(function () { this.object.focus(false); }); });
        self.icon.addClass("focus");
        self.icon.children(".task").each(function () { this.object.focus(true); });
    }

    // Hides the page and its tasks.
    self.hide = function ()
    {
        self.icon.removeClass("focus");
        if (self.isDisplayed())
        {
            self.content.removeClass("display");
            // Hides the tasks of the page.
            self.icon.children(".task").each(function () { this.object.hide(); });
        }
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
            task.icon.detach();
        self.numberTasks--;
        self.updateIconHeight();
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
        self.icon.children(".task").each(function () { this.object.close(); });
        // Removes the page nodes
        self.icon.remove();
        self.content.remove();
        // Hides the tasks list if there is no page remaining
        if (!gl_tasksList.getPages().length)
            gl_tasksList.hide();
    }
    
    // Puts the focus on the page when we click on it, and starts to drag it if necessary.
    self.mouseDown = function (e)
    {
        if (e.which != 1 || TasksList.Buttons.isButton(e))
            return ;
        // Displays the page
        if (!self.icon.hasClass("focus"))
        {
            self.justFocused = true;
            self.display();
        }
        else
            self.justFocused = false;
        // Starts to drag the icon if we clicked on its active area
        if (e.target == self.activeArea[0])
        {
            gl_desktop.drag.start(e, self.icon[0], self, "mouseMove", "mouseWheel", "mouseUp");
            gl_desktop.drag.setCursor("move");
            self.element = gl_desktop.drag.getElement();
            self.mouse = gl_desktop.drag.getMouse();
            // Takes the scroll into account
            self.element.y += gl_tasksList.scrollTop() - gl_tasksList.top;
            // Creates the target
            self.target = $("<div></div>");
            self.target.addClass("target");
            self.target.height(self.icon.height());
            self.target.insertBefore(self.icon);
            // Moves the icon to the mouse position
            self.icon.addClass("drag");
            self.icon.css("top", e.pageY - self.mouse.y - gl_tasksList.top + gl_tasksList.offset);
            // Computes the positions of the pages
            var pages = gl_tasksList.getPages();
            var y = C.TasksList.top;
            self.pagesPositions = new Array();
            for (var i = 0; i < pages.length; ++i)
            {
                y += $(pages[i]).height() / 2 + C.TasksList.pageMargin / 2;
                self.pagesPositions.push({y: y, node: pages[i]});
                y += $(pages[i]).height() / 2 + C.TasksList.pageMargin / 2;
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
        self.icon.css("top", mouseY - self.mouse.y - gl_tasksList.top + gl_tasksList.offset);
        // The reference y changes if we are above or below the original position of the page
        var y = mouseY - self.mouse.y - gl_tasksList.top + gl_tasksList.scrollTop();
        if (self.element.y < y)
            y += self.icon.height() + C.TasksList.pageMargin;
        // Searches the page above which we are
        var lastPage = self.pagesPositions[self.pagesPositions.length - 1];
        if (y < lastPage.y)
        {
            for (var i = 0; i < self.pagesPositions.length && y > self.pagesPositions[i].y; ++i)
                ;
            if (self.pagesPositions[i].node != self.target[0].nextSibling)
                self.target.insertBefore(self.pagesPositions[i].node);
        }
        // We are above the last page
        else if (lastPage.node != self.target[0].previousSibling)
            self.target.insertAfter(lastPage.node);
    }
    
    // Updates the position of the icon.
    self.mouseWheel = function (e)
    {
        self.mouseMove(e);
    }
    
    // Drops the page icon at the target position.
    self.mouseUp = function (e)
    {
        self.icon.removeClass("drag");
        self.icon.insertBefore(self.target);
        self.target.remove();
        self.pagesPositions = undefined;
        if (!F.isMouseOverNode(e, self.activeArea))
            self.icon.removeClass("over");
        gl_tasksList.stopDrag(e);
    }
    
    // Closes the page with the middle mouse.
    self.mouseUpClose = function (e)
    {
        if (e.which == 2)
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
            position = C.TasksList.defaultPosition;
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
            node.resize.remove();
        if (parent.resize)
            parent.resize.remove();
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
                node.resize.addClass("vertical");
                node.resize.css("left", Math.max(l - taskMargin, left));
                node.resize.css("top", top);
                node.resize.width(Math.max(Math.min(taskMargin, width), 0));
                node.resize.height(height > 0 ? height : 0);
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
                node.resize.addClass("horizontal");
                node.resize.css("left", left);
                node.resize.css("top", Math.max(t - taskMargin, top));
                node.resize.width(width > 0 ? width : 0);
                node.resize.height(Math.max(Math.min(taskMargin, height), 0));
            }
        }
        // Saves the coordinates of the node
        node.left = left;
        node.top = top;
        node.width = width;
        node.height = height;
    }
    
    // Must be called when the height of the page icon changed.
    self.updateIconHeight = function ()
    {
        self.icon.height("auto");
        self.icon.height(self.icon.height());
    }
    
    // Changes the container that displays the page.
    self.setContainer = function (container)
    {
        self.container.containerChanged(self);
        self.container = container;
        self.icon.children(".task").each(function () { this.object.setContainer(container); });
        if (container != gl_desktop)
            self.content.addClass("window");
        else
            self.content.removeClass("window");
    }
    
    // Puts the focus on the page elements.
    self.setZIndex = function (zIndex)
    {
        self.content.css("z-index", zIndex);
        self.icon.children(".task").each(function () { this.object.setZIndex(zIndex); });
        self.zIndex = zIndex;
    }
    
    // Returns true is the page is displayed.
    self.isDisplayed = function ()
    {
        return (self.content.hasClass("display"));
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
        self.buttons; // Manages the buttons of the task
        self.lastFocusDate = 0; // The date of the last time the task was focused. Used by isFocus to tell if the task has the focus regardless of the events timing.
        self.lastBackgroundSet; // The last task background set using setBackground()
        self.overflow = true; // If the overflow of the content is enabled
        // The following members are used when the task icon is being dragged
        self.mouse; // The position of the mouse in the icon {x, y}
        self.element; // The initial position of the element {x, y}
        self.resistance; // Applies a resistance before starting to drag the task icon
        self.target; // A node that shows the future position of the task while it is dragged
        self.node; // The node of the task in the tree
        self.taskHeight; // The current height of the task
        self.tasksPositions; // The positions of the tasks when the drag started. Array({y: , above: , node: })
        self.createPage; // If a new page have to be created, the value is either "top" or "bottom"
        self.createPageTarget; // A temporary page used to host the target
        // The coordinates of the content of the task
        self.left;
        self.top;
        self.width;
        self.height;
        
        // Creates the tasks list icon
        self.icon = $("<div></div>").addClass("task");
        self.icon[0].object = self;
        self.icon.mousedown(function (e) { self.mouseDown(e); });
        self.icon.mouseup(function (e) { self.mouseUpClose(e); });
        // Adds the title of the icon
        var title = $("<div></div>");
        title.addClass("title");
        title.html("<span>250 " + T.Tasks[resource] + "</span>");
        title.appendTo(self.icon);
        // Adds the content of the icon
        var content = $("<div></div>").addClass("content");
        var container = $("<div></div>").addClass("container");
        container.html("120 images<br />23 videos<br />12 audios<br />5 documents");
        container.appendTo(content);
        content.appendTo(self.icon);
        // Adds the left margin of the content
        var left = $("<div></div>").addClass("left");
        left.prependTo(content);
        // Creates the buttons
        self.buttons = new TasksList.Buttons(self);
        
        // Creates the content of the task
        self.content = $("<div></div>").addClass("task");
        self.content[0].object = self;
        self.content.html(html);
        F.translate(self.content);
        // Events
        self.content.mousedown(function (e) { self.mouseDownContent(e); });
        self.content.mouseenter(function (e) { self.mouseEnterContent(e); });
        self.content.mouseleave(function (e) { self.mouseLeaveContent(e); });
        self.content.appendTo(gl_desktop.node.tasks);
    }
    
    // Closes the task.
    self.close = function ()
    {
        if (self.resource.close)
            self.resource.close();
        self.buttons.deinit();
        self.icon.remove();
        self.content.remove();
        self.icon[0].object = undefined;
        self.content[0].object = undefined;
        for (var key in self)
            self[key] = undefined;
    }
    
    // Starts to move the task icon
    self.mouseDown = function (e)
    {
        if (e.which != 1 || TasksList.Buttons.isButton(e))
            return ;
        gl_desktop.drag.start(e, self.icon, self, "mouseMove", "mouseWheel", "mouseUp");
        gl_desktop.drag.setCursor("pointer");
        self.mouse = gl_desktop.drag.getMouse();
        self.element = gl_desktop.drag.getElement();
        self.element.y -= gl_tasksList.top;
        // Takes the scroll into account
        self.mouse.y += gl_tasksList.scrollTop();
        self.element.y += gl_tasksList.scrollTop();
        self.taskHeight = self.icon.height();
        // There is a little resistance before the task begins to move
        self.resistance = true;
        // Saves all the pages displayed
        gl_desktop.sessions.save("task");
    }
    
    // Moves the task icon according to the mouse position.
    self.mouseMove = function (e)
    {
        // The new position of the task icon
        e.pageY -= gl_tasksList.top;
        var y = e.pageY - self.mouse.y;
        var x = e.pageX - self.mouse.x;
        var elementY = self.element.y - gl_tasksList.scrollTop();
        
        // Top limit
        if (y < 0 && !self.resistance)
            self.icon.css("top", 0);
        // Bottom limit
        else if (y > gl_desktop.height - self.taskHeight && !self.resistance)
            self.icon.css("top", gl_desktop.height - self.taskHeight);
        // If the resistance is broken, we start to drag the task
        else if (self.resistance && Math.sqrt(Math.pow(elementY - y, 2) + Math.pow(self.element.x - x, 2)) > C.TasksList.taskResistance)
            y = self.startDrag(y);
        // Moves the dragged task horizontally if the mouse is outside of the tasks list
        if (e.pageX >= gl_tasksList.width)
        {
            self.icon.css("left", e.pageX + C.TasksList.taskDragShift.x);
            self.icon.css("top", Math.min(Math.max(e.pageY + C.TasksList.taskDragShift.y, 0), gl_desktop.height - self.taskHeight));
            gl_desktop.drag.setCursor("auto");
        }
        // Moves the task vertically
        else if (!self.resistance)
        {
            self.icon.css("top", y + gl_tasksList.offset);
            gl_desktop.drag.setCursor("move");
            self.icon.css("left", 0);
            if (!self.resistance)
                self.updateTasksList(e.pageY);
        }
    }
    
    // Changes the displayed page if we are not on the tasks list and a task is being dragged.
    self.mouseWheel = function (e, delta)
    {
        // Ensures that we are outside the tasks list
        if (e.pageX < gl_tasksList.width)
            return ;
        
        var i;
        if (!self.createPage)
        {
            // Searches the current position of the target
            var nextTask = ((i = self.target.next(".task")).length ? i[0] : undefined);
            var prevTask = ((i = self.target.prev(".task")).length ? i[0] : undefined);
            for (i = 0; i < self.tasksPositions.length; ++i)
            {
                var p = self.tasksPositions[i];
                if ((p.above && nextTask == p.node) || (!p.above && prevTask == p.node))
                    break;
            }
        }
        else if (self.createPage == "top")
            i = -1;
        else if (self.createPage == "bottom")
            i = self.tasksPositions.length - 1;
        // Moves the target up
        if (delta > 0 && self.createPage != "top")
        {
            // Jumps one of two steps (above and below) of the current task icon, because they display same target
            if (p && !p.above && p.node == self.icon[0])
                i--;
            self.updateTasksList(undefined, self.tasksPositions[i - 1]);
        }
        // Down
        else if (delta < 0 && self.createPage != "bottom")
        {
            if (p && ((p.above && p.node == self.icon[0]) || (!p.above && self.target[0].nextSibling == self.icon[0])))
                i++;
            self.updateTasksList(undefined, self.tasksPositions[i + 1]);
        }
        // Scrolls to the target if necessary
        if (self.createPage == "top")
            gl_tasksList.scrollTop(0);
        else if (self.createPage == "bottom")
            gl_tasksList.scrollTop(gl_tasksList.node.icons.scrollHeight);
        else if (self.target.offset().top < gl_tasksList.top)
            gl_tasksList.scrollTop(gl_tasksList.scrollTop() + self.target.offset().top - gl_tasksList.top);
        else if (self.target.offset().top + self.taskHeight > gl_tasksList.top + gl_desktop.height)
            gl_tasksList.scrollTop(gl_tasksList.scrollTop() + self.target.offset().top - gl_tasksList.top - gl_desktop.height + self.taskHeight);
    }

    // Moves the dragged task to the page under the cursor.
    self.mouseUp = function (e)
    {
        // If the task has been dragged
        if (!self.resistance)
        {
            var preview = gl_desktop.taskPreview;
            var newPage = self.target[0].parentNode;
            // The task has been moved in a new page
            if (self.icon[0].parentNode != newPage || self.createPage)
            {
                // Removes the old task
                self.getPage().removeTask(self, true);
                // Puts the task in a new page
                if (self.createPage == "bottom")
                    (new Page()).addTask(self);
                else if (self.createPage == "top")
                    (new Page()).icon.insertAfter(gl_tasksList.node.top)[0].object.addTask(self);
                // Adds the task to its new page
                else
                    newPage.object.addTask(self, gl_desktop.taskPreview.position, gl_desktop.taskPreview.task, self.target);
                self.getPage().hide();
                self.getPage().display();
            }
            // The page hasn't changed (on the tasks list)
            else
            {
                // The task hasn't moved
                if (!preview.task && !preview.page)
                    self.icon.insertBefore(self.target);
                // The icon is moved in the same page
                else if (!preview.page && preview.task.getPage() == self.getPage())
                {
                    self.icon.insertBefore(self.target);
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
                    self.icon.insertBefore(self.target);
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
            self.stopDrag(e);
        }
        // Otherwise the page is hidden if it was already focused before the mouse down event
        else if (!self.getPage().justFocused)
            self.getPage().hide();
    }
    
    // Closes the task with the middle mouse.
    self.mouseUpClose = function (e)
    {
        if (e.which == 2)
            self.getPage().removeTask(self);
    }
    
    // Puts the focus on the page of the task.
    self.mouseDownContent = function (e)
    {
        self.getPage().display();
    }
    
    // The mouse entered the task content.
    self.mouseEnterContent = function (e)
    {
        self.icon.addClass("over_content");
    }
    
    // The mouse leaved the task content.
    self.mouseLeaveContent = function (e)
    {
        if (!gl_desktop.drag.isDragging())
            self.icon.removeClass("over_content");
    }
        
    // Displays the content of the task.
    self.display = function ()
    {
        self.content.addClass("display");
    }

    // Adds / removes the focus on the content of the task.
    self.focus = function (focus)
    {
        if (focus && self.content.hasClass("focus"))
            return ;
        if (focus)
        {
            self.content.addClass("focus");
            self.lastFocusDate = new Date().getTime();
        }
        else
        {
            self.content.removeClass("focus");
            self.lastFocusDate = 0;
        }
    }
    
    // Hides the content of the task.
    self.hide = function ()
    {
        self.content.removeClass("display");
    }

    // Removes the resistance and starts to drag the task.
    self.startDrag = function (mouseY)
    {
        var elementY = self.element.y - gl_tasksList.scrollTop();

        self.resistance = false;
        // Computes the positions of the tasks
        var afterCurrentTask = false; // True after the current task is passed
        var pages = gl_tasksList.getPages();
        var y = C.TasksList.top - 1; // -1 above the current task
        self.tasksPositions = new Array();
        self.tasksPositions.push({y: 0, above: true, node: $(pages[0]).children(".task:first")[0]});
        for (var i = 0; i < pages.length; ++i)
        {
            var tasks = $(pages[i]).children(".task");
            for (var j = 0; j < tasks.length; ++j)
                {
                    var halfHeight = $(tasks[j]).height() / 2;
                    y += halfHeight;
                    self.tasksPositions.push({y: y, above: false, node: tasks[j]});
                    y += halfHeight;
                    if (tasks[j] == self.icon[0])
                    {
                        afterCurrentTask = true;
                        y++;
                    }
                }
            if (afterCurrentTask)
                y += C.TasksList.pageMargin;
            if (i + 1 < pages.length)
                self.tasksPositions.push({y: y, above: true, node: $(pages[i + 1]).children(".task:first")[0]});
            else
                self.tasksPositions.push({y: y});
            if (!afterCurrentTask)
                y += C.TasksList.pageMargin;
        }
        // Updates the position of the mouse in the task
        if (mouseY < elementY)
        {
            var delta = elementY - mouseY;
            self.mouse.y -= delta;
            mouseY += delta;
        }
        else if (mouseY > elementY)
        {
            var delta = mouseY - elementY;
            self.mouse.y += delta;
            mouseY -= delta;
        }
        // The mouse is outside the task
        if (self.mouse.y < 0)
        {
            mouseY -= 2 - self.mouse.y;
            self.mouse.y = 2;
        }
        else if (self.mouse.y > self.taskHeight)
        {
            mouseY += self.mouse.y - self.taskHeight + 2;
            self.mouse.y = self.taskHeight - 2;
        }
        // Starts the drag
        self.icon.addClass("drag");
        gl_desktop.drag.setCursor("move");
        gl_desktop.taskPreview.updateActiveAreas();
        // Sets the style of the icon
        if (self.getPage().icon.hasClass("window"))
            self.icon.addClass("drag_window");
        // Creates the target
        self.target = $("<div></div>").addClass("target");
        self.target.insertAfter(self.icon);
        self.target.height(self.taskHeight);
        self.buttons.startDrag(self);
        return (mouseY);
    }
    
    // Clears the context after the drag of the icon.
    self.stopDrag = function (e)
    {
        gl_tasksList.node.top.height(C.TasksList.pageMargin);
        gl_tasksList.node.bottom.height(0);
        gl_desktop.taskPreview.hide(e);
        gl_desktop.drag.overlay.html("");
        self.target.remove();
        self.icon.parent().removeClass("empty");
        self.icon.removeClass("drag");
        self.icon.removeClass("drag_window");
        self.buttons.stopDrag(e);
        gl_tasksList.stopDrag(e);
        gl_tasksList.node.icons.children(".page").each(function () { this.object.updateIconHeight(); });
        if (self.createPage == "bottom")
            gl_tasksList.scrollTop(gl_tasksList.node.icons[0].style.scrollHeight);
        delete self.createPage;
        delete self.tasksPositions;
        if (self.createPageTarget)
            self.createPageTarget.close();
        delete self.createPageTarget;
    }
    
    // Puts a target task under the cursor to show the place where the dragged task will be moved.
    // @param mouseY : The y position of the mouse. taskPosition is used if mouseY is undefined.
    // @param taskPosition : Allows to tell directly where the target have to be, when mouseY is undefined.
    self.updateTasksList = function (mouseY, taskPosition)
    {
        if (mouseY != undefined)
        {
            // Changes the reference y depending if the task has been dragged up or down
            mouseY = mouseY - self.mouse.y + gl_tasksList.scrollTop();
            if (mouseY > self.element.y)
                mouseY += self.taskHeight;
            
            // Searches the position under which the task is
            var i;
            for (i = 0; i < self.tasksPositions.length && mouseY > self.tasksPositions[i].y; ++i)
                ;
            var p = self.tasksPositions[i - 1];
        }
        else
            var p = taskPosition;
        // We want to create a new page with the task
        if (!p || !p.node)
        {
            var createPage = !p ? "top" : !p.node ? "bottom" : undefined;
            // Nothing more to do
            if (self.createPage == createPage)
                return ;
            // If the task is already alone and in first or last position in the tasks list, we don't need to create a new one
            var isAlone = (self.getPage().numberTasks == 1);
            var isFirstTask = isAlone && !$(self.icon[0].parentNode).prev(".page").length;
            var isLastTask = isAlone && !$(self.icon[0].parentNode).next(".page").length;
            if ((createPage != "top" || !isFirstTask) && (createPage != "bottom" || !isLastTask))
            {
                self.createPage = createPage;
                if (self.createPageTarget)
                    self.createPageTarget.close();
                // Creates a temporary page to host the target
                var oldPage = self.target[0].parentNode;
                if (self.createPage == "top")
                {
                    gl_tasksList.node.top.height(0);
                    (self.createPageTarget = new Page()).icon.insertBefore(gl_tasksList.node.top).append(self.target);
                }
                else
                    (self.createPageTarget = new Page()).icon.append(self.target);
                if (oldPage)
                    oldPage.object.updateIconHeight();
                self.createPageTarget.display();
                // Displays the task as a full page
                self.icon.parent().addClass("focus");
                self.setCoordinates(gl_desktop.left, gl_desktop.top, gl_desktop.width, gl_desktop.height);
                self.display();
                self.icon.removeClass("drag_window");
                gl_desktop.taskPreview.hide();
                gl_desktop.drag.overlay.html("");
                return ;
            }
            else
                p = {node: self.icon[0]};
        }
        var resetBottomHeight = false; // Resets the bottom height at the end of this method
        // Moves the target to the new position
        if ((p.above && self.target[0].nextSibling != p.node) || (!p.above && self.target[0].previousSibling != p.node))
        {
            var oldPage = self.target[0].parentNode;
            // Ensures that the tasks list doesn't get smaller, so that it doesn't scroll when the target is moved, from the bottom
            gl_tasksList.node.bottom.height(self.taskHeight);
            // Moves the target
            p.above ? self.target.insertBefore(p.node) : self.target.insertAfter(p.node);
            // Updates the height of the page icon
            self.target[0].parentNode.object.updateIconHeight();
            if (oldPage)
                oldPage.object.updateIconHeight();
            resetBottomHeight = true;
        }
        // Clears the createPage
        if (self.createPage)
        {
            gl_tasksList.node.top.height(C.TasksList.pageMargin);
            self.hide();
            delete self.createPage;
            self.createPageTarget.close();
            delete self.createPageTarget;
        }
        // If the original page of the task is empty, we ensure that the bottom margin is displayed, in order to keep the stability of the tasks list
        if (self.getPage().numberTasks == 1)
            if (self.target[0].parentNode != self.icon[0].parentNode)
                self.icon.parent().addClass("empty");
            else
                self.icon.parent().removeClass("empty");
        var page = p.node.parentNode.object;
        // Displays the current page
        if (!page.isDisplayed() || (page != self.getPage() && self.getPage().isDisplayed()))
        {
            // Hides all the pages
            gl_desktop.node.pages.children(".display").each(function() { this.object.hide(); });
            // Displays the current page but keeps the focus on the original page of the task
            if (page != self.getPage())
            {
                page.display();
                page.icon.removeClass("focus");
                self.icon.parent().addClass("focus");
            }
            // Restore the state of the session when we clicked on the task icon
            else
            {
                gl_desktop.sessions.load("task").each(function () { this.object.display(); });
                page.display();
            }
            // Displays the preview
            gl_desktop.taskPreview.display(undefined, undefined, page);
            gl_desktop.taskPreview.updateActiveAreas();
        }
        if (resetBottomHeight)
            gl_tasksList.node.bottom.height(0);
    }
    
    // Sets the coordinates of the content of the task.
    self.setCoordinates = function(left, top, width, height)
    {
        // Compensates the presence of the task border
        if (!self.content.hasClass("no_border"))
        {
            width = Math.max(width - C.Desktop.taskBorder * 2, 0);
            height = Math.max(height - C.Desktop.taskBorder * 2, 0);
        }
        // Sets the new coordinates
        self.left = left;
        self.top = top;
        self.width = width;
        self.height = height;
        self.content.css("left", left);
        self.content.css("top", top);
        self.content.width(width);
        self.content.height(height);
        // Hides the borders when the height is null
        if (self.height <= 0 || self.width <= 0)
            self.content.addClass("hide_border");
        else
            self.content.removeClass("hide_border");
        // Calls the onResize method of the resource
        if (self.resource.onResize)
            self.resource.onResize(left, top, width, height);
    }
    
    // Changes the container that displays the task.
    self.setContainer = function (container)
    {
        if (container != gl_desktop)
            self.content.addClass("window");
        else
            self.content.removeClass("window");
        self.focus(false);
    }
    
    // Sets the z-indes of the content of the task.
    self.setZIndex = function (zIndex)
    {
        self.content.css("z-index", zIndex);
    }
    
    // Returns the page of the task.
    self.getPage = function () { return (self.icon[0].parentNode.object); }
    
    // Returns true if the task icon is on the new page area.
    self.isOnCreatePage = function ()
    {
        return (self.createPage);
    }

// Resource interface
// The resources have to implement this interface in order to receive the task events.
    {
        // Closes the resource.
        function close() {}
        // The task have been resized.
        function onResize(left, top, width, height) {}
    }

// Resource API
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
                self.content.removeClass(self.lastBackgroundSet);
                delete self.lastBackgroundSet;
            }
            // No background
            if (!display)
                self.content.addClass("no_background");
            // Custom background
            else
            {
                self.content.removeClass("no_background");
                if (background)
                {
                    self.lastBackgroundSet = background;
                    self.content.addClass(background);
                }
            }
        }
        
        // Displays / hides the task borders.
        // @param border : True to display the task border.
        self.setBorder = function (border)
        {
            if (border)
                self.content.removeClass("no_border");
            else
                self.content.addClass("no_border");
        }
        
        // Returns true if the task has a border.
        self.hasBorder = function (border)
        {
            return (!self.content.hasClass("no_border"));
        }
        
        // Allows the resources to disable the overflow of the content.
        // @param overflow : True if the overflow is enabled.
        self.setOverflow = function (overflow)
        {
            if (overflow != self.overflow)
            {
                self.overflow = overflow;
                if (!overflow)
                    self.content.addClass("disable_overflow");
                else
                    self.content.removeClass("disable_overflow");
            }
        }
        
        // Returns true if the page has the focus.
        self.isFocus = function ()
        {
            // The focus also have to be on the task for more that 10 milliseconds, in order to avoid event order problems
            return (self.content.hasClass("focus") && (new Date().getTime() - self.lastFocusDate) > 10);
        }
        
        // Returns true if the page is a window.
        self.isWindow = function ()
        {
            return (self.content.hasClass("window"));
        }
        
        // Ensures that the task icon style is up to date, based on the position of the mouse.
        // Should be called after a drag within the content.
        self.updateMouseOverContent = function (e)
        {
            if (F.isMouseOverNode(e, self.content))
                self.icon.addClass("over_content");
            else
                self.icon.removeClass("over_content");
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
        self.resize = $("<div></div>");
        self.resize.addClass("resize");
        self.resize[0].taskNode = self;
        self.resize.mousedown(function (e) { self.mouseDown(e); });
        self.resize.mouseenter(function (e) { self.mouseEnter(e); });
        self.resize.mouseleave(function (e) { self.mouseLeave(e); });
        self.resize.appendTo(pageContent);
    }
    
    // Starts to resize the node.
    self.mouseDown = function (e)
    {
        if (e.which != 1)
            return ;
        gl_desktop.drag.start(e, self.resize[0], self, "mouseMove", "mouseWheel", "mouseUp");
        gl_desktop.drag.setCursor(self.h > -1 ? "e-resize" : "s-resize");
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
        self.resize[0].parentNode.object.display();
        self.resize.parent().addClass("resize_node");
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
    self.mouseUp = function (e)
    {
        delete self.leftNode;
        delete self.rightNode;
        delete self.topNode;
        delete self.bottomNode;
        delete self.startResize;
        self.resize.parent().removeClass("resize_node");
        if (!F.isMouseOverNode(e, self.resize))
            self.resize.parent()[0].object.icon.removeClass("over");
    }

    // The mouse entered the resize bar.
    self.mouseEnter = function (e)
    {
        self.resize.parent().addClass("over_resize");
        self.resize.parent()[0].object.icon.addClass("over");
    }
    
    // The mouse leaved the resize bar.
    self.mouseLeave = function (e)
    {
        self.resize.parent().removeClass("over_resize");
        if (!gl_desktop.drag.isDragging())
            self.resize.parent()[0].object.icon.removeClass("over");
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
    
    // Creates the tasks active areas in the drag overlay.
    self.updateActiveAreas = function ()
    {
        gl_desktop.drag.overlay.html("");
        gl_desktop.node.tasks.children(".display").each(function ()
        {
            $("<div></div>").appendTo(gl_desktop.drag.overlay).css({
                left: this.object.left,
                top: this.object.top,
                width: this.object.width,
                height: this.object.height,
                zIndex: $(this).css("z-index"),
                position: "absolute"})
            .mousemove(function (e) { self.mouseMoveContent(e); })
            .mouseleave(function (e) { self.mouseLeaveContent(e); })
            [0].object = this.object;
        });
    }

    // The mouse moved over the content of a task, while a task icon is being dragged.
    // Displays a preview of the future position of the dragged task in the content.
    self.mouseMoveContent = function (e)
    {
        if (!(self.currentTask = e.delegateTarget.object))
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
            // Facilitates the east/west transition
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
            // Facilitates the north/south transition
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
    
    // The mouse leaved the task content.
    self.mouseLeaveContent = function (e)
    {
        self.hide();
        var page;
        // Displays the default drop position
        if ((page = gl_desktop.drag.getObject()) && (page = gl_desktop.drag.getObject().target) && (page = page[0].parentNode))
            self.display(undefined, undefined, page.object);
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
            var adjustBorder = task.hasBorder() ? C.Desktop.taskBorder * 2 : 0;
            preview.style.top = task.top + "px";
            preview.style.left = task.left + "px";
            preview.style.width = task.width + adjustBorder + "px";
            preview.style.height = task.height + adjustBorder + "px";
            preview.style.zIndex = task.content.css("z-index");
            $(preview).addClass("display");
            if (position == "n")
                preview.style.height = Math.ceil(task.height / 2) + adjustBorder / 2 + (-task.height % 2 * adjustBorder / 2 + (1 - adjustBorder / 2) * (task.height + 1) % 2) + "px"; // The last part adjusts the parity
            if (position == "s")
            {
                preview.style.top = task.top + Math.floor(task.height / 2) + "px";
                preview.style.height = Math.ceil(task.height / 2) + adjustBorder + "px";
            }
            if (position == "w")
                preview.style.width = Math.ceil(task.width / 2) + adjustBorder / 2 + (-task.width % 2 * adjustBorder / 2 + (1 - adjustBorder / 2) * (task.width + 1) % 2) + "px"; // The last part adjusts the parity
            if (position == "e")
            {
                preview.style.left = task.left + Math.floor(task.width / 2) + "px";
                preview.style.width = Math.ceil(task.width / 2) + adjustBorder + "px";
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
            preview.style.zIndex = page.content.css("z-index");
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
            position = C.TasksList.defaultPosition;
            // Moves the preview to a node in static position
            $(gl_desktop.node.desktop).append(preview);
            // Displays the preview
            preview.style.top = page.top + "px";
            preview.style.left = page.left + "px";
            preview.style.width = page.width + "px";
            preview.style.height = page.height + "px";
            preview.style.zIndex = page.content.css("z-index");
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
        $(preview).removeClass("window");
        if (page.icon.hasClass("window"))
            $(preview).addClass("window");
        // The dragged task icon matches the style of the preview
        drag.icon.removeClass("drag_window");
        if (page.icon.hasClass("window"))
            drag.icon.addClass("drag_window");
        // Updates the state of the preview
        self.position = position;
        self.task = task;
        self.currentTask = task;
        self.page = page;
        if (task)
            self.page = undefined;
    }
    
    // Hides the preview.
    self.hide = function ()
    {
        $(gl_desktop.node.preview).removeClass("display");
        delete self.task;
        delete self.position;
        delete self.page;
        delete self.currentTask;
        // Ensures that the dragged task icon has the style of its original page
        var drag = gl_desktop.drag.getObject();
        if (gl_desktop.drag.isDragging("Task") && !drag.createPage)
        {
            drag.icon.removeClass("drag_window");
            if (drag.getPage().icon.hasClass("window"))
                drag.icon.addClass("drag_window");
        }
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
        var pages = gl_desktop.node.pages.children(".display");
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
        // Members
        self.object; // The dragged object
        self.objectName; // The name of its class
        self.mouse = { x : 0, y : 0 }; // The position of the mouse in the element
        self.element = { x : 0, y : 0 }; // The initial position of the element
        self.overlay = $("body>.drag"); // This node is displayed over everything while something is being dragged
        self.parameter; // An user defined parameter passed to the events
        self.mouseMoveEvent;
        self.mouseUpEvent;
        self.mouseWheelEvent;
        
        // Events
        gl_desktop.events.bind("mousemove", self, function (e) { self.mouseMove(e); });
        gl_desktop.events.bind("mousewheel", self, function (e, delta) { self.mouseWheel(e, delta); });
        gl_desktop.events.bind("mouseup", self, function (e) { self.mouseUp(e); });
        gl_desktop.events.bind("mouseleave", self, function (e) { self.mouseLeave(e); });
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
        self.objectName = F.getObjectName(object);
        self.mouseMoveEvent = mouseMove;
        self.mouseUpEvent = mouseUp;
        self.mouseWheelEvent = mouseWheel;
        self.parameter = parameter;
        // The selection is disabled during the drag
        disableSelection(false);
        // Ensures that no event will reach the desktop while something is being dragged
        self.overlay.addClass("display");
    }

    // Called when the mouse moves.
    self.mouseMove = function (e)
    {
        if (self.object && self.mouseMoveEvent)
            self.object[self.mouseMoveEvent](e, self.parameter);
    }

    // Called when the mouse wheel clicks.
    self.mouseWheel = function (e, delta)
    {
        if (self.object && self.mouseWheelEvent)
            self.object[self.mouseWheelEvent](e, delta, self.parameter);
    }

    // Called when the mouse is up (the drag is finished).
    self.mouseUp = function (e)
    {
        if (!self.isDragging() || e.which != 1)
            return ;
        if (self.object && self.mouseUpEvent)
            stop = self.object[self.mouseUpEvent](e, self.parameter);
         self._stop();
    }

    // Called when the mouse leaves the desktop, in order to finish the drag.
    self.mouseLeave = function (e)
    {
        if (!self.isDragging() || !C.Desktop.stopDragLeaveDesktop)
            return ;
        if (self.object && self.mouseUpEvent)
            stop = self.object[self.mouseUpEvent](e, self.parameter);
         self._stop();
    }

    // Stops the dragging of the object.
    self._stop = function ()
    {
        self.object = undefined;
        self.objectName = undefined;
        self.mouseMoveEvent = undefined;
        self.mouseUpEvent = undefined;
        self.mouseWheelEvent = undefined;
        self.parameter = undefined;
        self.mouse = { x : 0, y : 0 };
        self.element = { x : 0, y : 0 };
        self.overlay.removeClass("display");
        self.setCursor();
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

    // Sets the CSS cursor displayed during the drag.
    self.setCursor = function (cursor)
    {
        if (cursor)
            self.overlay.css("cursor", cursor);
        else
            self.overlay.css("cursor", "auto");
    }
    
    self.init();
    return (self);
}

// Manages the events on the desktop.
function Events()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.events = new Object(); // Stores the events and the objects binded to them
        
        // Events
        $(document.body).mousedown(function (e) { self.call("mousedown", e); });
        $(document.body).mousewheel(function (e, delta) { self.call("mousewheel", e, Math.round(delta * C.Desktop.mouseWheelMultiplier)); });
        $(document.body).mousemove(function (e) { self.call("mousemove", e); });
        $(window).mouseup(function (e) { self.call("mouseup", e); });
        $(document.body).mouseleave(function (e) { self.call("mouseleave", e); });
    }

    // Binds an event to an object. The handler will be called when the event is called.
    // An object can only have one handler per event type.
    self.bind = function (type, object, handler)
    {
        if (!self.events[type])
            self.events[type] = new Array();
        var list = self.events[type];
        for (var i = 0; i < list.length; ++i)
            if (list[i].object == object)
                list.splice(i--, 1);
        self.events[type].push({object: object, handler: handler});
    }

    // Unbins an event from an object.
    self.unbind = function (type, object)
    {
        if (!self.events[type])
            return ;
        var list = self.events[type];
        for (var i = 0; i < list.length; ++i)
            if (list[i].object == object)
                list.splice(i--, 1);
    }

    // Calls all the handlers of an event.
    self.call = function (type, e, delta)
    {
        if (!self.events[type])
            return ;
        var list = self.events[type];
        // Makes a copy of the events so that they can be safely unbinded and iterated at the same time
        var copy = new Array();
        for (var i = 0; i < list.length; ++i)
            copy.push(list[i]);
        for (var i = 0; i < copy.length; ++i)
            copy[i].handler(e, delta);
    }
    
    self.init();
    return (self);
}
