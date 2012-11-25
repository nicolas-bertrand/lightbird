// Manages the file player and the playlist.
var gl_player;

function Player(task)
{
    var self = this;
    gl_player = self;
    
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.bottom = $("#desktop>.bottom");
        self.node.timeline = self.node.bottom.children(".timeline");
        self.node.before = self.node.timeline.children(".before");
        self.node.played = self.node.timeline.children(".played");
        self.node.buffered = self.node.timeline.children(".buffered");
        self.node.after = self.node.timeline.children(".after");
        self.node.player = self.node.bottom.children(".player");
        self.node.play = self.node.player.children(".play");
        self.node.controls = self.node.player.children(".controls");
        self.node.numerator = self.node.play.find(".numerator")[0]; // The number of the current file played
        self.node.denominator = self.node.play.find(".denominator")[0]; // The total number of files
        self.node.time = self.node.play.children(".time");
        self.node.elapsed = self.node.play.find(".elapsed")[0]; // The time elapsed for the current file
        self.node.duration = self.node.play.find(".duration")[0]; // The duration of the file
        // Playlist
        self.node.playlist = self.node.bottom.children(".playlist");
        self.node.header = self.node.playlist.children(".header");
        self.node.tabs = self.node.header.children(".tabs");
        self.node.add = self.node.header.children(".add");
        self.node.pin = self.node.header.children(".pin");
        self.node.list = self.node.playlist.children(".list");

        // Members
        self.header; // Manages the header of the playlist
        self.playlist; // Manages the playlist
        self.mouseLeaveTimeout = 0; // Delays the effect of the mouse leave
        
        // Default values
        C.Desktop.bottomHeight = C.Player.defaultHeight;
        self.node.bottom.height(C.Desktop.bottomHeight);
        self.node.before.width("0%");
        self.node.played.width("0%");
        self.node.buffered.width("0%");
        self.node.after.width("100%");
        self.node.numerator.innerHTML = "0";
        self.node.denominator.innerHTML = "0";
        self.node.elapsed.innerHTML = "0:00";
        self.node.duration.innerHTML = "0:00";
        self.playlist = new self.Playlist(self);
        self.header = new self.Header(self);
        
        // Icons
        self.generateIcons();
        
        // Events
        self.node.bottom.mouseenter(function (e) { self.mouseEnter(e); });
        self.node.bottom.mouseleave(function (e) { self.mouseLeave(e); });
        self.node.play.children(".number").mousedown(function (e) { self.mouseDown(e); });
        self.node.player.children(".name").mousedown(function (e) { self.mouseDown(e); });
    }
    
    // Resizes the player when the browser size changes.
    self.onResize = function (width, height)
    {
        self.node.playlist.width(width);
        // The playlist is highter than the top part of the desktop
        if (self.playlist.height > height - C.Desktop.bottomHeight - C.Desktop.topHeight)
            self.playlist.setHeight(self.playlist.height);
    }
    
    // The mouse entered the player area.
    self.mouseEnter = function (e)
    {
        // The player is already in the correct state
        if (self.mouseLeaveTimeout)
        {
            clearTimeout(self.mouseLeaveTimeout);
            self.mouseLeaveTimeout = 0;
        }
        else
        {
            C.Desktop.bottomHeight = C.Player.defaultHeight + C.Player.timelineOverHeight;
            self.node.timeline.height(C.Player.timelineHeight + C.Player.timelineOverHeight);
            gl_desktop.onResize();
        }
    }
    
    // The mouse leaved the player area.
    self.mouseLeave = function (e)
    {
        self.mouseLeaveTimeout = setTimeout(function ()
        {
            C.Desktop.bottomHeight = C.Player.defaultHeight;
            self.node.timeline.height(C.Player.timelineHeight);
            self.playlist.hide();
            gl_desktop.onResize();
            self.mouseLeaveTimeout = 0;
        }, C.Player.mouseLeaveTimeout);
    }
    
    // Generates the SVG icons of the player.
    self.generateIcons = function ()
    {
        self.drawIcon($(self.node.play).children(".play").children("div")[0], 16, gl_svg.Player.play, 0, 0.3);
      //self.drawIcon($(self.node.play).children(".play").children("div")[0], 16, gl_svg.Player.pause);
        self.drawIcon($(self.node.play).children(".previous").children("div")[0], 14, gl_svg.Player.previous, 0.5);
        self.drawIcon($(self.node.play).children(".next").children("div")[0], 14, gl_svg.Player.next, -0.1);
        self.drawIcon($(self.node.controls).children(".volume").children("div")[0], 25, gl_svg.Player.volume);
      //self.drawIcon($(self.node.controls).children(".volume").children("div")[0], 25, gl_svg.Player.mute);
        self.drawIcon($(self.node.controls).children(".settings").children("div")[0], 21, gl_svg.Player.settings);
      //self.drawIcon($(self.node.controls).children(".repeat").children("div")[0], 27, gl_svg.Player.repeat, 0.2);
      //self.drawIcon($(self.node.controls).children(".repeat").children("div")[0], 27, gl_svg.Player.repeatOne, 0.2, 3.2);
        self.drawIcon($(self.node.controls).children(".repeat").children("div")[0], 27, gl_svg.Player.noRepeat, 0.2);
      //self.drawIcon($(self.node.controls).children(".random").children("div")[0], 27, gl_svg.Player.random);
        self.drawIcon($(self.node.controls).children(".random").children("div")[0], 27, gl_svg.Player.linear);
        self.drawIcon($(self.node.controls).children(".fullscreen").children("div")[0], 25, gl_svg.Player.fullscreen);
      //self.drawIcon($(self.node.controls).children(".fullscreen").children("div")[0], 25, gl_svg.Player.normalScreen);
    }

    // Draws an SVG icon.
    self.drawIcon = function (destination, width, path, left, top)
    {
        var paper = Raphael(destination, width, 20);
        var icon = paper.path(path);
        icon.translate(left, top);
        icon.attr("fill", "#eeeeee");
        icon.attr("stroke", "none");
        // Events
        $(destination.parentNode).mouseenter(function (e)
        {
            icon.attr("fill", "#444444");
        });
        $(destination.parentNode).mouseleave(function (e)
        {
            icon.attr("fill", "#eeeeee");
        });
    }
    
    // Mouse down on a button of the player.
    self.mouseDown = function (e)
    {
        if (e.which != 1)
            return ;
        // Opens the playlist
        if ($(e.delegateTarget).hasClass("number") || $(e.delegateTarget).hasClass("name"))
        {
            self.playlist.display();
        }
    }

// Manages the playlist.
self.Playlist = function (player)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.height; // The height of the playlist
        
        // Default values
        self.setHeight(C.Player.playlistHeight);
        self.hide();
        
        // Events
    }
    
    // Sets the height of the playlist.
    self.setHeight = function (height)
    {
        // Clamp the height
        if (height <= C.Player.headerHeight)
            height = C.Player.headerHeight + 1;
        else if (height > gl_desktop.middleHeight)
            height = gl_desktop.middleHeight;
        // Resizes the playlist
        var listHeight = height - C.Player.headerHeight;
        self.height = height;
        node.playlist.height(height);
        node.list.height(listHeight);
        node.playlist[0].style.top = -height + "px";
        // Fills the empty space of the playlist with empty rows
        var wantedFilesNumber = listHeight / C.Player.listFileHeight;
        var currentFilesNumber = node.list.children().length;
        for (var i = currentFilesNumber; i < wantedFilesNumber; i++)
            $("<div></div>").addClass(i % 2 ? "even" : "odd").appendTo(node.list);
        currentFilesNumber = i;
        for (var i = currentFilesNumber - 2; i > wantedFilesNumber; i--)
            node.list.children().last().remove();
    }

    // Displays the playlist.
    self.display = function ()
    {
        node.playlist.addClass("display");
        node.timeline.addClass("opaque");
        node.playlist.height(self.height);
    }

    // Hides the playlist.
    self.hide = function ()
    {
        node.playlist.removeClass("display");
        node.timeline.removeClass("opaque");
        node.playlist.height(0);
    }
    
    self.init();
    return (self);
}
    
// Manages the headers of the playlist, which includes the tabs, the pin and the resize.
self.Header = function (player)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.tabTemplate = node.header.children(".tab_template")[0].innerHTML; // The template used to create the tabs
        // Playlist resize
        self.element; // The initial y position of the header
        self.mouse; // The y position of the mouse in the header
        self.initialHeight; // The initial height of the playlist
        
        // Default values
        node.header.height(C.Player.headerHeight);
        self.addTab("Recent files");
        self.drawAdd();
        self.drawPin();
        
        // Events
        node.header.mousedown(function (e) { self.mouseDownOnHeader(e); });
    }
    
    // Adds a tab.
    self.addTab = function (name)
    {
        var tab = $(self.tabTemplate);
        var lastTab = node.tabs.children().last()[0];
        var position = -C.Player.tabShift;
        
        // Builds the tab
        tab[0].tabIconEvent = new Array(); // Allows to send events to the icon (focus, odd, even)
        self.drawTabIcon(tab.children(".left")[0], 4, gl_svg.Player.tabLeft);
        self.drawTabIcon(tab.children(".right")[0], -5, gl_svg.Player.tabRight);
        self.drawCloseIcon(tab.children(".close")[0], gl_svg.Player.tabClose);
        tab.children(".middle").children()[0].innerHTML = name;
        self.focus(tab);
        tab.mousedown(function (e) { self.mouseDownOnTab(e); });
        tab.click(function (e) { self.clickOnTab(e); });
        tab.appendTo(node.tabs);
        // Waits for the offsetWidth of the tab to be defined before displaying it,
        // since we can't know the width of the tab in advance because of its text.
        tab.addTabInterval = setInterval(function ()
        {
            if (tab[0].offsetWidth)
            {
                // Ensures that the tab doesn't "break"
                tab.width(tab[0].offsetWidth + 5);
                clearInterval(tab.addTabInterval);
                // We need the offsetWidth and offsetLeft of the previous tab, which might not be defined yet
                if (lastTab)
                    tab.addTabInterval = setInterval(function ()
                    {
                        if (lastTab.offsetWidth && lastTab.offsetLeft)
                        {
                            // Sets the position of the tab relative to the previous one
                            tab.css("left", (position + lastTab.offsetLeft + lastTab.offsetWidth - C.Player.tabMargin) + "px");
                            clearInterval(tab.addTabInterval);
                            tab.addClass("display");
                            // Puts the add icon after the new tab
                            self.updateAdd();
                        }
                    }, C.Player.tabOffsetInterval);
            }
        }, C.Player.tabOffsetInterval);
        // The first tab can be directly displayed
        if (!lastTab)
        {
            tab.css("left", position + "px");
            tab.addClass("display");
            self.updateAdd();
        }
    }
    
    // Draws the left or right part of the tab in SVG.
    self.drawTabIcon = function (destination, left, path)
    {
        var paper = Raphael(destination, 30, 24);
        var icon = paper.path(path);
        icon.node.isMainPath = true;  // Distinguishes the main path from its shadow
        icon.translate(left, 0);
        icon.attr("fill", "90-#252525-#2c2c2c");
        icon.attr("stroke", "none");
        icon.glow({ width : 6, color : "black", opacity : 0.15 });
        icon.glow({ width : 1, color : "#4d4d4d", opacity : 1 })[0].node.isMainPath = true; // Part of the main path
        // Covers the shadow on the bottom border
        var rect;
        if ($(destination).hasClass("left"))
            rect = paper.rect(0, 23, 5, 1);
        else
            rect = paper.rect(27, 23, 5, 1);
        rect.attr("fill", "#4d4d4d");
        rect.attr("stroke", "none");
        // Changes the background of the SVG depending on the event
        destination.parentNode.tabIconEvent.push(function (name)
        {
            if (name == "odd")
                icon.attr("fill", "#292929");
            else if (name == "even")
                icon.attr("fill", "#303030");
            else if (name == "focus")
                icon.attr("fill", "90-#252525-#2c2c2c");
        });
    }
    
    // Draws the SVG close icon of the tab.
    self.drawCloseIcon = function (destination, path)
    {
        var paper = Raphael(destination, 14, 14);
        var icon = paper.path(path);
        icon.translate(3.5, 3.5);
        icon.attr("fill", "white");
        icon.attr("fill-opacity", "0.25");
        icon.attr("stroke", "none");
        icon.glow({ width : 3, color : "#000000", opacity : 0.13 });
        // Events
        $(icon.node.parentNode).mouseenter(function (e)
        {
            icon.attr("fill-opacity", "0.8");
        });
        $(icon.node.parentNode).mouseleave(function (e)
        {
            icon.attr("fill-opacity", "0.25");
        });
        $(icon.node.parentNode).click(function (e)
        {
            if (e.which == 1)
                self.removeTab($(destination.parentNode));
        });
    }
    
    // Removes a tab from the header
    self.removeTab = function (tab)
    {
        var offset = tab[0].offsetWidth - C.Player.tabMargin - C.Player.tabShift;
        
        // Moves the next tabs to replace the removed tab
        for (var nextTab = tab.next(); nextTab.length; nextTab = nextTab.next())
            nextTab.css("left", nextTab[0].offsetLeft - offset);
        // Ensures that the focus is always on a tab
        if (tab.hasClass("focus"))
        {
            if (tab.next().length)
                self.focus(tab.next());
            else if (tab.prev().length)
                self.focus(tab.prev());
        }
        // Removes the tab and updates the others
        tab.remove();
        self.updateOddEven();
        self.updateAdd();
    }
    
    // Puts the focus on a tab.
    self.focus = function (tab)
    {
        var tabs = node.tabs.children();
        var z = tabs.length;
        
        // Updates the z-indes and removes the previous focus
        tabs.each(function ()
        {
            this.style.zIndex = z--;
            $(this).removeClass("focus");
        });
        self.updateOddEven();
        // Applies the new focus
        tab.addClass("focus");
        for (var j = 0; j < tab[0].tabIconEvent.length; ++j)
            tab[0].tabIconEvent[j]("focus");
        tab.css("z-index", tabs.length + 1);
    }
    
    // Updates the odd / even alternation of the tabs.
    self.updateOddEven = function ()
    {
        var tabs = node.tabs.children();
        var i = 0;
        
        tabs.each(function ()
        {
            var node = $(this);
            node.removeClass("odd");
            node.removeClass("even");
            if (i++ % 2)
            {
                node.addClass("odd");
                if (!node.hasClass("focus"))
                    for (var j = 0; j < this.tabIconEvent.length; ++j)
                        this.tabIconEvent[j]("odd");
            }
            else
            {
                node.addClass("even");
                if (!node.hasClass("focus"))
                    for (var j = 0; j < this.tabIconEvent.length; ++j)
                        this.tabIconEvent[j]("even");
            }
        });
    }
    
    // Draws the add element of the header.
    self.drawAdd = function ()
    {
        var paper = Raphael(node.add[0], 53, 23);
        // Background
        var tab = paper.path(gl_svg.Player.addTab);
        tab.translate(4, 0);
        tab.attr("fill", "#404040");
        tab.attr("stroke", "none");
        tab.glow({ width : 6, color : "black", opacity : 0.1 });
        tab.glow({ width : 1, color : "#4d4d4d", opacity : 1 });
        // Fills the left side of the background when there is no tab
        var rect = paper.rect(0, 0, 24, 24);
        rect.attr("stroke", "none");
        rect.attr("fill", "#242424");
        rect.hide();
        // The add icon
        var add = paper.path(gl_svg.Player.add);
        add.translate(28, 6.95);
        add.attr("fill", "white");
        add.attr("opacity", "0.5");
        add.attr("stroke", "none");
        add.glow({ width : 4, color : "black", opacity : 0.05 });
        node.add.mousedown(function (e) { self.mouseDownOnTab(e); });
        // The style displayed when there is no tab
        var glow = add.glow({ width : 4, color : "black", opacity : 0.05 });
        glow.translate(-3, 0);
        glow.hide();
        node.add.addIconNoTabStyle = function ()
        {
            if (!node.add.addIconNoTab)
            {
                node.add.addIconNoTab = true;
                tab.attr("fill", "#242424");
                add.translate(-3, 0);
                rect.show();
                glow.show();
            }
        };
        // Displays the default style back
        node.add.addIconDefaultStyle = function ()
        {
            if (node.add.addIconNoTab)
            {
                node.add.addIconNoTab = false;
                tab.attr("fill", "#404040");
                add.translate(3, 0);
                rect.hide();
                glow.hide();
            }
        };
        self.updateAdd();
    }
    
    // Puts the add icon after the last tab.
    self.updateAdd = function ()
    {
        var lastTab = node.tabs.children().last()[0];
        
        // The icon has not been drawn yet
        if (!node.add.addIconDefaultStyle)
            return ;
        // Clears the previous update if it is still running
        clearInterval(node.add.addTabInterval);
        // Waits for the offsetWidth of the last tab to be defined before updating the position
        if (lastTab)
        {
            node.add.addTabInterval = setInterval(function ()
            {
                if (lastTab.offsetWidth && lastTab.offsetLeft)
                {
                    node.add.css("left", lastTab.offsetLeft + lastTab.offsetWidth - C.Player.addIconMargin + "px");
                    clearInterval(node.add.addTabInterval);
                }
            }, C.Player.tabOffsetInterval);
            node.add.addIconDefaultStyle();
        }
        // There is no tab, so we put the add icon at the origin
        else
        {
            node.add.addIconNoTabStyle();
            node.add.css("left", C.Player.addIconOrigin + "px");
        }
    }
    
    // Draws the pin element of the header.
    self.drawPin = function ()
    {
        var paper = Raphael(node.pin[0], 53, 23);
        var tabLeft = paper.path(gl_svg.Player.tabLeft);
        tabLeft.translate(4, 0);
        tabLeft.attr("fill", "#242424");
        tabLeft.attr("stroke", "none");
        tabLeft.glow({ width : 6, color : "black", opacity : 0.15 });
        tabLeft.glow({ width : 1, color : "#4d4d4d", opacity : 1 });
        var rect = paper.rect(35, 0, 50, 24);
        rect.attr("fill", "#242424");
        rect.attr("stroke", "none");
        var pin = paper.path(gl_svg.Player.pin);
        pin.translate(28, 4);
        pin.attr("fill", "white");
        pin.attr("opacity", "0.5");
        pin.attr("stroke", "none");
        pin.glow({ width : 6, color : "black", opacity : 0.15 });
    }
    
    // Focus / add a tab.
    self.mouseDownOnTab = function (e)
    {
        var tab = self.getTabUnderMouse(e);
        
        if (e.which == 1 && tab)
        {
            if (tab.hasClass("tab"))
                self.focus(tab);
            else if (tab.hasClass("add"))
                self.addTab("New playlist " + (node.tabs.children().length));
        }
    }
    
    // Removes a tab.
    self.clickOnTab = function (e)
    {
        var tab = self.getTabUnderMouse(e);
        
        if (e.which == 2 && tab && tab.hasClass("tab"))
            self.removeTab(tab);
    }
    
    // Starts to resize the playlist.
    self.mouseDownOnHeader = function (e)
    {
        // Something is already being dragged
        if (e.which != 1 || gl_desktop.drag.isDragging() || !$(e.target).hasClass("header"))
            return ;
        gl_desktop.drag.start(e, node.header[0], self, "mouseMoveResize");
        self.element = gl_desktop.drag.getElement().y;
        self.mouse = gl_desktop.drag.getMouse().y;
        self.initialHeight = player.playlist.height;
    }
    
    // Resizes the playlist while the header is being dragged.
    self.mouseMoveResize = function (e)
    {
        player.playlist.setHeight(self.element + self.mouse - e.pageY + self.initialHeight);
    }
    
    // Returns the real tab under the mouse, which can be different from the
    // target because of the curves of the tabs.
    self.getTabUnderMouse = function (e)
    {
        var root;
        var target = e.target;
        var tab;
        var add;
        
        // We are on a tab
        if ((tab = $(e.delegateTarget)).hasClass("tab"))
        {
            // If we don't have a target inside the tab, we just make a rough approximation
            if ($(e.target).hasClass("tab"))
            {
                var position = e.pageX - tab[0].offsetLeft;
                if (position < C.Player.tabExternLeft)
                    return (tab.prev());
                else if (position > tab[0].offsetWidth - C.Player.tabExternRight)
                    return (tab.next().length ? tab.next() : node.add);
                return (tab);
            }
            // Otherwise we can use the path of the svg to ensure that the mouse is over the tab
            if (target.tagName == "svg")
                root = target.parentNode;
            else if (target.tagName == "path")
                root = target.parentNode.parentNode;
            // We might be out of the tab
            if (root && ($(root).hasClass("left") || $(root).hasClass("right")) && !target.isMainPath)
            {
                // Resolves a bug on the right side
                if (target.tagName != "svg" || (e.pageX - root.parentNode.offsetLeft - target.parentNode.offsetLeft) >= 0)
                {
                    // The correct tab is on the left
                    if ($(root).hasClass("left"))
                        tab = tab.prev();
                    // On the right
                    else if (!(tab = tab.next()).length)
                        // There is nothing on the right of the tab, so the mouse is over the add button
                        return (node.add);
                }
            }
            if (tab.length)
                return (tab);
        }
        // The mouse is on the add tab icon
        if ($(e.delegateTarget).hasClass("add"))
            return (node.add);
    }
    
    self.init();
    return (self);
}
    
    self.init();
    return (self);
}
