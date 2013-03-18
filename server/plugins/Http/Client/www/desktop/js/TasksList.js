// Manages the tasks list.
var gl_tasksList;

function TasksList(task)
{
    var self = this;
    gl_tasksList = self;
    
// TasksList methods
{
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.tasks_list = $("#desktop>.middle>.area>.tasks_list");
        self.node.icons = self.node.tasks_list.children(".icons");
        self.node.top = $(self.node.icons).children(".top");
        self.node.bottom = $(self.node.icons).children(".bottom");
        self.node.background = self.node.tasks_list.children(".background");

        // Members
        self.width = C.TasksList.defaultWidth; // The width of the tasks list
        self.fullScreenHideTimeout; // Used to hide the tasks list in full screen mode
        self.over = false; // True while the mouse is over the tasks list
        self.scrollDelta; // The position of the mouse in the scrolling area
        self.scrollInterval; // The interval that actually scroll the tasks list
        self.top = gl_header.height; // The top position of the tasks list
        
        // Default values
        self.node.tasks_list.height(self.height);
        self.node.top.height(C.TasksList.top);
        self.node.background.css("top", -gl_header.height);
        
        // Events
        self.node.tasks_list.mouseenter(function (e) { self.mouseEnter(e); });
        self.node.tasks_list.mouseleave(function (e) { self.mouseLeave(e); });
        self.node.tasks_list.mousemove(function (e) { self.mouseMove(e); });
        self.node.tasks_list.click(function (e) { self.click(e); });
    }
    
    // The browser has been resized.
    self.onResize = function (width, height)
    {
        // The background should always have the same height as the desktop
        self.node.background.height(height);
    }
    
    // We entered/leaved the full screen mode.
    self.onFullScreen = function (fullScreen)
    {
        if (fullScreen)
        {
            self.top = 0;
            self.node.background.css("top", self.top);
            self.hideFullScreen();
        }
        else
        {
            self.displayFullScreen();
            self.top = gl_header.height;
            self.node.background.css("top", -self.top);
            self.node.tasks_list.css("z-index", "auto");
        }
    }
    
    // The mouse entered the tasks list.
    self.mouseEnter = function ()
    {
        self.over = true;
        // Displays the tasks list when the mouse enters it, in full screen mode.
        if (gl_desktop.isFullScreen())
            self.displayFullScreen();
    }
    
    // The mouse leaved the tasks list.
    self.mouseLeave = function ()
    {
        self.over = false;
        self.stopScroll();
        // Hides the tasks list after the timeout when the mouse leaves it, in full screen mode.
        if (gl_desktop.isFullScreen())
            self.fullScreenHideTimeout = setTimeout(function () {
                self.hideFullScreen();
            }, C.TasksList.FullScreen.displayDuration);
    }
    
    // Manages the scrolling areas at the top and the bottom of the tasks list.
    self.mouseMove = function (e)
    {
        // The top scrolling area of the tasks list
        if (e.pageY > self.top && e.pageY <= self.top + C.TasksList.Scroll.height)
            self.scroll(1 - (e.pageY - self.top) / C.TasksList.Scroll.height);
        // The bottom scrolling area of the tasks list
        else if (e.pageY >= self.top + gl_desktop.height - C.TasksList.Scroll.height && e.pageY < self.top + gl_desktop.height)
            self.scroll((self.top + gl_desktop.height - C.TasksList.Scroll.height - e.pageY) / C.TasksList.Scroll.height);
        // Ensures that the scrolling is stopped outside of the scrolling areas
        else if (self.scrollInterval)
            self.stopScroll();
    }

    // Scrolls the tasks list using the mouse wheel.
    self.mouseWheel = function (delta)
    {
        if (self.over)
        {
            self.node.icons[0].scrollTop -= delta;
            if (gl_desktop.drag.isDragging("Task"))
                gl_desktop.drag.getObject().updateTasksList(gl_desktop.mouse.pageY - self.top);
        }
    }
    
    // Hides all the pages, or displays them back.
    self.click = function (e)
    {
        if (e.which != 1 || e.target != self.node.icons[0])
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
        var height = self.node.icons[0].scrollHeight - gl_desktop.height;
        
        self.scrollDelta = delta;
        // Nothing to scroll
        if (!height || !delta || (delta > 0 && self.node.icons[0].scrollTop == 0) || (delta < 0 && self.node.icons[0].scrollTop == height))
            self.stopScroll();
        // Creates the interval that will scroll the tasks
        else if (!self.scrollInterval)
            self.scrollInterval = setInterval(function () { self.doScroll(); }, C.TasksList.Scroll.fps);
    }
    // This method is called by setInterval and scrolls the tasks list according to the delta.
    self.doScroll = function ()
    {
        var height = self.node.icons[0].scrollHeight - gl_desktop.height;

        if (!self.scrollInterval)
            return ;
        // Calculates the number of pixels to scroll
        if (self.scrollDelta > 0)
            self.node.icons[0].scrollTop -= (Math.exp(Math.abs(self.scrollDelta)) - 1) * C.TasksList.Scroll.speed;
        else
            self.node.icons[0].scrollTop += (Math.exp(Math.abs(self.scrollDelta)) - 1) * C.TasksList.Scroll.speed;
        // Updates the position of the dragged task or page
        if (gl_desktop.drag.isDragging("Task"))
            gl_desktop.drag.getObject().updateTasksList(gl_desktop.mouse.pageY - self.top);
        else if (gl_desktop.drag.isDragging("Page"))
            gl_desktop.drag.getObject().mouseMove(undefined, gl_desktop.mouse.pageY);
        // Scroll up finished
        if (self.node.icons[0].scrollTop <= 0)
        {
            self.node.icons[0].scrollTop = 0;
            self.stopScroll();
        }
        // Scroll down finished
        else if (self.node.icons[0].scrollTop >= height)
        {
            self.node.icons[0].scrollTop = height;
            self.stopScroll();
        }
    }

    // Stops the scrolling of the tasks list.
    self.stopScroll = function ()
    {
        if (self.scrollInterval)
        {
            clearInterval(self.scrollInterval);
            delete self.scrollInterval;
            delete self.scrollDelta;
        }
    }
    
    // Displays the tasks list when there is at least one page in it.
    self.display = function ()
    {
        $(self.node.tasks_list).addClass("display");
    }
    
    // Hides the tasks list when it is empty.
    self.hide = function ()
    {
        $(self.node.tasks_list).removeClass("display");
        if (gl_desktop.isFullScreen())
            self.hideFullScreen();
    }
    
    // Displays the tasks list in full screen mode.
    self.displayFullScreen = function ()
    {
        self.node.tasks_list.removeClass("hide_full_screen");
        self.node.tasks_list.css("width", C.TasksList.defaultWidth);
        self.node.tasks_list.css("z-index", gl_windows.getZIndex());
        if (self.fullScreenHideTimeout)
        {
            clearTimeout(self.fullScreenHideTimeout);
            self.fullScreenHideTimeout = undefined;
        }
    }
    
    // Hides (partially) the tasks list in full screen mode.
    self.hideFullScreen = function ()
    {
        self.node.tasks_list.addClass("hide_full_screen");
        self.node.tasks_list.css({"width": C.TasksList.FullScreen.hideWidth,
                                  "z-index": C.TasksList.FullScreen.zIndex});
        if (self.fullScreenHideTimeout)
        {
            clearTimeout(self.fullScreenHideTimeout);
            self.fullScreenHideTimeout = undefined;
        }
    }
    
    // Returns the pages of the tasks list.
    self.getPages = function ()
    {
        return (self.node.icons.children(".page"));
    }
    
    // Set or get the scroll top of the tasks list depending on the parameter.
    self.scrollTop = function (scrollTop)
    {
        if (scrollTop != undefined)
            self.node.icons[0].scrollTop = scrollTop
        return (self.node.icons[0].scrollTop);
    }
    
    // Scrolls to the bottom of the tasks list (less the height of the pages margin).
    self.scrollToBottom = function ()
    {
        self.node.icons[0].scrollTop = self.node.icons[0].scrollHeight - gl_desktop.height - C.TasksList.pageMargin;
    }
}

// Manages the buttons of a task icons.
TasksList.Buttons = function (task)
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.task = task; // The task for which this instance is managing the buttons
        self.close = {node: 0, paper: 0, path: 0, background: 0, link: 0}; // The close button
        self.window = {node: 0, paper: 0, path: 0, background: 0, link: 0}; // The window button
        
        // Default values
        self.drawClose();
        self.drawWindow();
        
        // Events
        $(self.task.icon).mouseenter(function (e) { self._display(); });
        $(self.task.icon).mouseleave(function (e) { self._hide(); });
    }
    
    self.deinit = function ()
    {
        for (var key in self)
            self[key] = undefined;
    }
    
    // Draws the close button.
    self.drawClose = function ()
    {
        var c = self.close;
        var slope = C.TasksList.Buttons.slopeRatio * C.TasksList.taskTitleHeight;
        var width = Math.round(C.TasksList.Buttons.margin * 2 + C.TasksList.Buttons.close.width + Math.abs(slope) / 2 + C.TasksList.Buttons.margin / 2);
        
        // Creates the node
        c.node = $("<div></div>").addClass("close")
        c.node.appendTo($(self.task.icon).children(".title"));
        c.node.width(width);
        c.paper = Raphael(c.node[0], "100%", "100%");
        // Draws the path of the icon
        c.path = c.paper.path(gl_svg.TasksList.close);
        var left = width - C.TasksList.Buttons.close.width - C.TasksList.Buttons.margin;
        var top = C.TasksList.taskTitleHeight / 2 - C.TasksList.Buttons.close.height / 2;
        c.path.transform("T" + left + "," + top);
        c.path.attr(C.TasksList.Buttons.close.attr);
        c.path.hide();
        // Draws the background
        c.background = c.paper.path("M0," + C.TasksList.taskTitleHeight + "L" + slope + " 0,H200,l0 " + C.TasksList.taskTitleHeight + "z");
        c.background.transform("T" + (Math.abs(slope / 2) - slope / 2) + ",0");
        c.background.attr(C.TasksList.Buttons.close.background);
        c.background.toBack();
        c.background.hide();
        c.link = c.background.clone()
        c.link.attr(C.TasksList.Buttons.linkAttr);
        c.link.toFront();
        // Events
        $(c.link.node).mouseover(function ()
        {
            c.background.show();
            c.path.attr(C.TasksList.Buttons.close.attrOver);
        });
        $(c.link.node).mouseout(function ()
        {
            c.background.hide();
            c.path.attr(C.TasksList.Buttons.close.attr);
        });
        $(c.link.node).click(function ()
        {
            self.task.getPage().removeTask(self.task);
        });
    }
    
    // Draws the window button.
    self.drawWindow = function ()
    {
        var c = self.window;
        var slope = Math.abs(C.TasksList.Buttons.slopeRatio) * (C.TasksList.Buttons.margin * 2 + C.TasksList.Buttons.window.height);
        var width = Math.round(C.TasksList.Buttons.margin * 2 + C.TasksList.Buttons.window.width + slope / 2 + C.TasksList.Buttons.margin);
        var height = Math.round(width * 1 / C.TasksList.Buttons.slopeRatio);
        
        slope = Math.abs(C.TasksList.Buttons.slopeRatio) * height;
        // Creates the node
        c.node = $("<div></div>").addClass("window")
        c.node.appendTo(self.task.icon.children(".content"));
        c.node.width(width);
        c.node.height(height);
        c.paper = Raphael(c.node[0], "100%", "100%");
        // Draws the path of the icon
        c.path = c.paper.path(gl_svg.TasksList.window);
        var left = width - C.TasksList.Buttons.window.width - C.TasksList.Buttons.margin;
        var top = height - C.TasksList.Buttons.window.height - C.TasksList.Buttons.margin;
        c.path.transform("T" + left + "," + top);
        c.path.attr(C.TasksList.Buttons.window.attr);
        c.path.hide();
        // Draws the background
        c.background = c.paper.path("M0," + height + "L" + slope + " 0,H200,l0 " + height + "z");
        c.background.attr(C.TasksList.Buttons.window.background);
        c.background.toBack();
        c.background.hide();
        c.link = c.background.clone();
        c.link.attr(C.TasksList.Buttons.linkAttr);
        c.link.toFront();
        // Events
        $(c.link.node).mouseover(function ()
        {
            if (self.task.isWindow())
                c.background.attr(C.TasksList.Buttons.window.backgroundWindow);
            else
                c.background.attr(C.TasksList.Buttons.window.backgroundDesktop);
            c.background.show();
            c.path.attr(C.TasksList.Buttons.window.attrOver);
        });
        $(c.link.node).mouseout(function ()
        {
            c.background.hide();
            c.path.attr(C.TasksList.Buttons.window.attr);
        });
        $(c.link.node).click(function ()
        {
            var page = self.task.getPage();
            if (self.task.isWindow())
            {
                c.background.attr(C.TasksList.Buttons.window.backgroundDesktop);
                page.setContainer(gl_desktop);
            }
            else
            {
                c.background.attr(C.TasksList.Buttons.window.backgroundWindow);
                gl_windows.open(page);
            }
            page.onResize();
            page.display();
        });
    }
    
    // Hides the buttons while the task is being dragged.
    self.startDrag = function ()
    {
        self.close.path.hide();
        self.window.path.hide();
        self.task.content.removeClass("highlight");
    }
    
    // Displays the buttons back, if the mouse is over the task.
    self.stopDrag = function (e)
    {
        if (F.isMouseOverNode(e, self.task.icon))
            self._display();
        else
            $(self.task.icon).removeClass("over");
    }
    
    // Displays the buttons and highlights the content of the task
    self._display = function ()
    {
        // Displays the buttons
        self.close.path.show();
        self.window.path.show();
        $(self.task.icon).addClass("over");
        // The highlight on the content is only added if there is more than one task in the page
        if (self.task.getPage().numberTasks > 1)
            self.task.content.addClass("highlight");
        else
            self.task.content.removeClass("highlight");
    }
    // Hides the buttons and the highlight
    self._hide = function ()
    {
        if (!gl_desktop.drag.isDragging())
        {
            self.close.path.hide();
            self.window.path.hide();
            self.task.content.removeClass("highlight");
            self.task.icon.removeClass("over");
        }
    }
    
    // Returns true if the target is part of the task's button.
    TasksList.Buttons.isButton = function (e)
    {
        return (e.target.tagName.toLowerCase() == "path");
    }

    self.init();
    return (self);
}

    self.init();
    return (self);
}
