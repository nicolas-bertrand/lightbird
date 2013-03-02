// Manages the top part of the desktop.
var gl_header;

function Header(task)
{
    var self = this;
    gl_header = self;
    
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.top = $("#desktop>.top");
        self.node.menu = self.node.top.children(".menu");
        self.node.controls = self.node.top.children(".controls");
        self.node.files = self.node.menu.children(".files");
        self.node.uploads = self.node.menu.children(".uploads");

        // Members
        self.height = C.Header.defaultHeight; // The height of the header
        self.menu = new self.Menu(); // Manages the menu
        self.controls = new self.Controls(); // Manages the control buttons
        
        // Default values
        self.node.top.height(self.height);
        
        // Events
    }
    
    // The screen mode has changed (full/normal).
    self.onFullScreen = function (fullScreen)
    {
        if (fullScreen)
            self.node.top.addClass("hide");
        else
            self.node.top.removeClass("hide");
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
        self.textWidth = {interval: undefined, elapsed: 0}; // Used to calculate the width of the texts
        // The buttons properties
        self.defaultBackground;
        self.files = {background: 0, link: 0, over: 0};
        self.uploads = {background: 0, link: 0, over: 0};
        
        // Default values
        self.slope = self.C.slopeRatio * gl_header.height;
        node.menu.css("width", self.C.paperWidth);
        self.paper = Raphael(node.menu[0], "100%", "100%");
        self.createButtons();
        self.updateTextsWidth();
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

        // Uploads
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.defaultButtonWidth + slope * 2;
        self.uploads.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.uploadsBackgroundAttr).hide();
        self.uploads.link = self.uploads.background.clone().attr(self.C.linkAttr);
        node.uploads.css("left", left);
        self.addEvents(self.uploads, node.uploads, "uploads");
        left += self.C.defaultButtonWidth;
        
        // The width of the paper
        node.menu.css("width", left + self.C.margin / 2 + Math.abs(slope));
    }
    
    // Adds the events to a button.
    self.addEvents = function (button, text, task)
    {
        $(button.link.node).mouseover(function (e)
        {
            if (!button.over)
                self.mouseEnter(button);
        });
        $(button.link.node).mouseout(function (e)
        {
            if (e.relatedTarget != text[0])
                self.mouseLeave(button);
        });
        text.mouseover(function (e)
        {
            if (!button.over)
                self.mouseEnter(button);
        });
        text.mouseout(function (e)
        {
            if (e.relatedTarget != button.link.node)
                self.mouseLeave(button);
        });
        var mousedown = function (e) { gl_desktop.openPage(task, '', e); };
        $(button.link.node).mousedown(mousedown);
        text.mousedown(mousedown);
    }
    
    // Waits the html engine to update the texts width, using an inverval loop.
    self.updateTextsWidth = function ()
    {
        var tw = self.textWidth;
    
        // Starts the inverval loop
        if (tw.interval == undefined)
        {
            tw.interval = setInterval(self.updateTextsWidth, self.C.updateTextsWidthDuration / self.C.updateTextsWidthSteps);
            tw.elapsed = 1;
        }
        // Ends the interval loop
        if (tw.elapsed++ >= self.C.updateTextsWidthSteps || (node.files.width() && node.uploads.width()))
        {
            self.updatePositions();
            clearInterval(tw.interval);
            tw.interval = undefined;
        }
    }
    
    // Update the position and the width of the buttons based on their texts width.
    self.updatePositions = function ()
    {
        var slope = self.slope / 2;
        var left = self.C.margin / 2;
        var filesWidth = node.files.width();
        var uploadsWidth = node.uploads.width();
        
        // Default background
        var backgroundWidth = left + filesWidth + slope + self.C.margin / 2;
        self.updateBackground(self.defaultBackground, -100, backgroundWidth + 100, self.C.defaultBackgroundAttr, false);

        // Files
        var backgroundWidth = left + filesWidth + slope + self.C.margin / 2;
        self.updateBackground(self.files.background, -100, backgroundWidth + 100);
        self.updateBackground(self.files.link, -100, backgroundWidth + 100);
        left += filesWidth;

        // Uploads
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + uploadsWidth + slope * 2;
        self.updateBackground(self.uploads.background, backgroundLeft, backgroundWidth);
        self.updateBackground(self.uploads.link, backgroundLeft, backgroundWidth);
        node.uploads.css("left", left);
        left += uploadsWidth;
        
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
    
    // The mouse entered a button.
    self.mouseEnter = function (button)
    {
        button.over = true;
        button.background.show();
    }
    
    // The mouse leaved a button.
    self.mouseLeave = function (button)
    {
        button.over = false;
        button.background.hide();
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
        $(self.disconnect.link.node).click(function (e) { gl_account.disconnect(); });
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
