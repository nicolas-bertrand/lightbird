// Manages the top part of the desktop.
var gl_header;

function Header(task)
{
    var self = this;
    gl_header = self;
    
// Header methods
{
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.top = $("#desktop>.top");
        self.node.menu = self.node.top.children(".menu");
        self.node.controls = self.node.top.children(".controls");
        self.node.files = self.node.menu.children(".files");
        self.node.upload = self.node.menu.children("span.upload");
        self.node.uploadInput = self.node.menu.children("div.upload").children("input");

        // Members
        self.height = C.Header.defaultHeight; // The height of the header
        self.menu = new self.Menu(); // Manages the menu
        self.controls = new self.Controls(); // Manages the control buttons
        self.fullScreenHideTimeout; // Used to hide the header in full screen mode
        self.mouseOverHeader; // True while the mouse is over the header
        
        // Default values
        self.node.top.height(self.height);
        
        // Events
        self.node.top.mouseenter(function () { self.mouseEnter(); });
        self.node.top.mouseleave(function () { self.mouseLeave(); });
    }
    
    // We entered/leaved the full screen mode.
    self.onFullScreen = function (fullScreen)
    {
        self.clearFullScreenTimeout();
        if (fullScreen)
            self.hideFullScreen();
        else
        {
            self.displayFullScreen();
            gl_desktop.events.unbind("mousedown", self);
        }
    }
    
    // Displays the header when the mouse enters it, in full screen mode.
    self.mouseEnter = function ()
    {
        self.mouseOverHeader = true;
        if (gl_desktop.isFullScreen())
        {
            self.displayFullScreen();
            self.clearFullScreenTimeout();
        }
    }
    
    // Hides the header after the timeout when the mouse leaves it, in full screen mode.
    self.mouseLeave = function ()
    {
        self.mouseOverHeader = false;
        if (gl_desktop.isFullScreen())
            self.fullScreenHideTimeout = setTimeout(function ()
            {
                self.hideFullScreen();
                self.fullScreenHideTimeout = undefined;
            }, C.Header.FullScreen.displayDuration);
    }
    
    // Displays the header, in full screen mode.
    // @param fullScreenTimeout: If the full screen timeout have to be enabled if the mouse is outside of the header.
    self.displayFullScreen = function (fullScreenTimeout)
    {
        self.node.top.removeClass("hide");
        self.node.top.css("height", C.Header.defaultHeight);
        self.clearFullScreenTimeout();
        gl_tasksList.headerHeightChanged(C.Header.defaultHeight);
        // Hides the header when the user clicks outside it
        gl_desktop.events.bind("mousedown", self, function() {
            if (!F.isMouseOverNode(self.node.top))
                self.hideFullScreen();
        });
        // Sets the full screen timeout if the mouse is outside of the header
        if (fullScreenTimeout && !self.mouseOverHeader)
            self.fullScreenHideTimeout = setTimeout(function () {
                self.hideFullScreen();
                delete self.fullScreenHideTimeout;
            }, C.TasksList.FullScreen.displayDuration);
    }
    
    // Hides the header, in full screen mode.
    self.hideFullScreen = function ()
    {
        self.node.top.addClass("hide");
        self.node.top.css("height", C.Header.FullScreen.hideHeight);
        gl_tasksList.headerHeightChanged(0);
        gl_desktop.events.unbind("mousedown", self);
        self.clearFullScreenTimeout();
    }
    
    // Clears the full screen timeout.
    self.clearFullScreenTimeout = function ()
    {
        if (self.fullScreenHideTimeout)
        {
            clearTimeout(self.fullScreenHideTimeout);
            delete self.fullScreenHideTimeout;
        }
    }
}
    
// Manages the menu.
self.Menu = function ()
{
    var self = this;
    var node = gl_header.node;
    
    self.init = function ()
    {
        // Members
        self.C = C.Header.Menu; // The configuration of the menu
        self.paper; // The SVG paper on which the buttons are drawn
        self.slope; // The slope of the buttons
        // The buttons properties
        self.defaultBackground;
        self.files = {background: 0, link: 0, over: 0};
        self.upload = {background: 0, link: 0, over: 0, input: gl_header.node.uploadInput, div: gl_header.node.uploadInput.parent()};
        
        // Default values
        self.slope = self.C.slopeRatio * gl_header.height;
        node.menu.css("width", self.C.paperWidth);
        self.paper = Raphael(node.menu[0], "100%", "100%");
        self.createButtons();
        self.updatePositions();
    }
    
    // Creates the buttons.
    self.createButtons = function ()
    {
        var slope = self.slope / 2;
        var left = 0;
        
        // Default background
        var backgroundWidth = left + self.C.defaultButtonWidth + slope + self.C.margin / 2;
        self.defaultBackground = self.createBackground(-100, backgroundWidth + 100, self.C.defaultBackgroundAttr, false);
        self.defaultBackground.toBack();

        // Files
        var backgroundWidth = left + self.C.defaultButtonWidth + slope + self.C.margin / 2;
        self.files.background = self.createBackground(-100, backgroundWidth + 100, self.C.filesBackgroundAttr).hide();
        self.files.link = self.files.background.clone().attr(self.C.linkAttr);
        node.files.css("left", self.C.margin / 2);
        self.addEvents(self.files, node.files, "files");
        left += self.C.defaultButtonWidth;

        // Upload
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.defaultButtonWidth + slope * 2;
        self.upload.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.uploadBackgroundAttr).hide();
        self.upload.link = self.upload.background.clone().attr(self.C.linkAttr);
        node.upload.css("left", left);
        self.addEventsUpload(self.upload, node.upload);
        // Upload input
        self.upload.div.css("left", self.upload.input._left = (left - self.C.margin / 2 + slope));
        self.upload.div.width(self.upload.input._width = (self.C.margin + self.C.defaultButtonWidth - slope * 2));
        left += self.C.defaultButtonWidth;
        
        // The width of the paper
        node.menu.css("width", left + self.C.margin / 2 + Math.abs(slope));
    }
    
    // Adds the events to a button.
    self.addEvents = function (button, text, task)
    {
        var mouseEnter = function (button)
        {
            button.over = true;
            button.background.show();
        }
        var mouseLeave = function (button)
        {
            button.over = false;
            button.background.hide();
        }
    
        $(button.link.node).mouseover(function (e)
        {
            if (!button.over)
                mouseEnter(button);
        });
        $(button.link.node).mouseout(function (e)
        {
            if (e.relatedTarget != text[0])
                mouseLeave(button);
        });
        text.mouseover(function (e)
        {
            if (!button.over)
                mouseEnter(button);
        });
        text.mouseout(function (e)
        {
            if (e.relatedTarget != button.link.node)
                mouseLeave(button);
        });
        var mousedown = function (e)
        {
            // Opens the task
            gl_desktop.openPage(task, '', e);
            // Displays the tasks list if we are in full screen mode
            if (gl_desktop.isFullScreen())
                gl_tasksList.displayFullScreen(true);
        };
        $(button.link.node).mousedown(mousedown);
        text.mousedown(mousedown);
    }
    
    // Adds the events of the upload button
    self.addEventsUpload = function (button, text)
    {
        // The mouse leaved the upload button.
        var mouseLeave = function ()
        {
            gl_desktop.events.unbind("mousemove", self);
            self.upload.div.unbind("mouseleave");
            button.over = false;
            button.background.hide();
            self.upload.div.removeClass("display");
        }
        
        // The mouse entered the upload button.
        var mouseEnter = function ()
        {
            button.over = true;
            button.background.show();
            self.upload.div.addClass("display");
            gl_desktop.events.bind("mousemove", self, function (e)
            {
                // Checks if the mouse is outside the button
                var left = e.pageX - self.upload.input._left + (self.C.slopeRatio * e.pageY);
                var right = e.pageX - self.upload.input._left + (self.C.slopeRatio * e.pageY) - self.upload.input._width - self.slope;
                if (left < 0 || right > 0)
                    mouseLeave(button);
            });
        }
    
        $(button.link.node).mouseover(function (e) { if (!button.over) mouseEnter(button); });
        $(button.link.node).mousemove(function (e) { if (!button.over) mouseEnter(button); });
        text.mouseover(function (e) { if (!button.over) mouseEnter(button); });
        self.upload.div.mouseleave(function (e) { mouseLeave(button); });
        self.upload.input.mouseleave(function (e) { mouseLeave(button); });
        self.upload.input.change(function (e)
        {
            if (this.files.length <= 0)
                return ;
            for (var i = 0; i < this.files.length; ++i)
                gl_uploads.add(this.files[i]);
        });
    }
    
    // Update the position and the width of the buttons based on their texts width.
    self.updatePositions = function ()
    {
        var slope = self.slope / 2;
        var left = self.C.margin / 2;
        var filesWidth = node.files.width();
        var uploadWidth = node.upload.width();
        
        // Default background
        var backgroundWidth = left + filesWidth + slope + self.C.margin / 2;
        self.updateBackground(self.defaultBackground, -100, backgroundWidth + 100, self.C.defaultBackgroundAttr, false);

        // Files
        var backgroundWidth = left + filesWidth + slope + self.C.margin / 2;
        self.updateBackground(self.files.background, -100, backgroundWidth + 100);
        self.updateBackground(self.files.link, -100, backgroundWidth + 100);
        left += filesWidth;

        // Upload
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + uploadWidth + slope * 2;
        self.updateBackground(self.upload.background, backgroundLeft, backgroundWidth);
        self.updateBackground(self.upload.link, backgroundLeft, backgroundWidth);
        node.upload.css("left", left);
        // Upload input
        self.upload.div.css("left", self.upload.input._left = (left - self.C.margin / 2 + slope));
        self.upload.div.width(self.upload.input._width = (self.C.margin + uploadWidth - slope * 2));
        left += uploadWidth;
        
        // The width of the paper
        node.menu.css("width", left + self.C.margin / 2 + Math.abs(slope));
    }
    
    // Creates and returns a button background.
    // @correctGap: If not false, the width is incremented by 1, in order to close gap between two adjacent backgrounds.
    self.createBackground = function (left, width, attr, correctGap)
    {
        if (self.C.correctGap && correctGap !== false)
            width++;
        var path = self.paper.path("M0," + gl_header.height + "L" + self.slope + " 0,H" + width + ",l" + -self.slope + " " + gl_header.height + "z");
        path.transform("T" + left + ",0");
        path.attr(attr);
        return (path);
    }
    // Updates the background position and width.
    // @correctGap: If not false, the width is incremented by 1, in order to close gap between two adjacent backgrounds.
    self.updateBackground = function (background, left, width, correctGap)
    {
        if (self.C.correctGap && correctGap !== false)
            width++;
        background.attr({path: "M0," + gl_header.height + "L" + self.slope + " 0,H" + width + ",l" + -self.slope + " " + gl_header.height + "z"});
        background.transform("T" + left + ",0");
    }
    
    self.init();
    return (self);
}

// Manages the control buttons.
self.Controls = function ()
{
    var self = this;
    var node = gl_header.node;
    
    self.init = function ()
    {
        // Members
        self.C = C.Header.Controls; // The configuration of the controls
        self.paper; // The SVG paper on which the buttons are drawn
        self.slope; // The slope of the buttons
        // The buttons properties
        self.defaultBackground;
        self.search = {icon: 0, background: 0, link: 0};
        self.settings = {icon: 0, background: 0, link: 0};
        self.help = {icon: 0, background: 0, link: 0};
        self.disconnect = {icon: 0, background: 0, link: 0};
        
        // Default values
        self.slope = self.C.slopeRatio * gl_header.height;
        node.controls.css("width", self.C.paperWidth);
        self.paper = Raphael(node.controls[0], "100%", "100%");
        self.createButtons();
        self.addEvents();
    }
    
    // Creates the buttons.
    self.createButtons = function ()
    {
        var slope = self.slope / 2;
        var left = 0;
        var top = self.C.top;
        
        // Search
        left += self.C.margin / 2 + Math.abs(slope);
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.searchWidth + slope * 2;
        self.search.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.searchBackgroundAttr).hide();
        
        var search = self.paper.path(gl_svg.Header.search);
        search.transform("T" + left + "," + top);
        search.attr(self.C.iconAttr);
        search.glow = search.glow(self.C.iconGlow);
        self.search.icon = search;
        left += self.C.searchWidth;
        self.search.link = self.search.background.clone().attr(self.C.linkAttr);
        
        // Settings
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.settingsWidth + slope * 2;
        self.settings.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.settingsBackgroundAttr).hide();
        
        var settings = self.paper.path(gl_svg.Header.settings);
        settings.transform("T" + left + "," + top);
        settings.attr(self.C.iconAttr);
        settings.glow = settings.glow(self.C.iconGlow);
        self.settings.icon = settings;
        left += self.C.settingsWidth;
        self.settings.link = self.settings.background.clone().attr(self.C.linkAttr);
        
        // Help
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.helpWidth + slope * 2;
        self.help.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.helpBackgroundAttr).hide();
        
        var help = self.paper.path(gl_svg.Header.help);
        help.transform("T" + left + "," + top);
        help.attr(self.C.iconAttr);
        help.glow = help.glow(self.C.iconGlow);
        self.help.icon = help;
        left += self.C.helpWidth;
        self.help.link = self.help.background.clone().attr(self.C.linkAttr);
        
        // Full screen
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = 1000;
        self.disconnect.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.disconnectBackgroundAttr).hide();

        var disconnect = self.paper.path(gl_svg.Header.disconnect);
        disconnect.transform("T" + left + "," + top);
        disconnect.attr(self.C.iconAttr);
        disconnect.glow(self.C.iconGlow);
        self.disconnect.icon = disconnect;
        
        left += self.C.disconnectWidth;
        self.disconnect.link = self.disconnect.background.clone().attr(self.C.linkAttr);
        
        // Default background
        self.defaultBackground = self.createBackground(backgroundLeft, backgroundWidth, self.C.defaultBackgroundAttr, false);
        self.defaultBackground.toBack();
        
        // The width of the paper
        node.controls.css("width", left + self.C.margin / 2);
    }
    
    // Adds the events to the buttons.
    self.addEvents = function ()
    {
        // Search
        $(self.search.link.node).mouseover(function (e)
        {
            self.mouseEnter(self.search);
            self.search.icon.attr(self.C.iconAttrInverse);
            self.search.icon.glow.attr(self.C.iconGlowInverse);
        });
        $(self.search.link.node).mouseout(function (e)
        {
            self.mouseLeave(self.search);
            self.search.icon.attr(self.C.iconAttr);
            self.search.icon.glow.attr(self.C.iconGlowAttr);
        });
        // Settings
        $(self.settings.link.node).mouseover(function (e) { self.mouseEnter(self.settings); });
        $(self.settings.link.node).mouseout(function (e) { self.mouseLeave(self.settings); });
        // Help
        $(self.help.link.node).mouseover(function (e) { self.mouseEnter(self.help); });
        $(self.help.link.node).mouseout(function (e) { self.mouseLeave(self.help); });
        // Disconnect
        $(self.disconnect.link.node).mouseover(function (e) { self.mouseEnter(self.disconnect); });
        $(self.disconnect.link.node).mouseout(function (e) { self.mouseLeave(self.disconnect); });
        $(self.disconnect.link.node).click(function (e) { gl_user.disconnect(); });
    }
    
    // Creates and returns a button background.
    // @correctGap: If not false, the width is incremented by 1, in order to close gap between two adjacent backgrounds.
    self.createBackground = function (left, width, attr, correctGap)
    {
        if (self.C.correctGap && correctGap !== false)
            width++;
        var path = self.paper.path("M0," + gl_header.height + "L" + self.slope + " 0,H" + width + ",l" + -self.slope + " " + gl_header.height + "z");
        path.transform("T" + left + ",0");
        path.attr(attr);
        return (path);
    }
    
    // The mouse entered a button.
    self.mouseEnter = function (button)
    {
        button.background.show();
    }
    
    // The mouse leaved a button.
    self.mouseLeave = function (button)
    {
        button.background.hide();
    }
    
    self.init();
    return (self);
}

    self.init();
    return (self);
}
