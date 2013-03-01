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
        self.node.controls = self.node.top.children(".controls");

        // Members
        self.controls = new self.Controls();
        
        // Default values
        self.node.top.height(C.Desktop.topHeight);
        
        // Events
    }
    
// Manages the controls buttons.
self.Controls = function ()
{
    var self = this;
    var node = gl_header.node;
    
    self.init = function ()
    {
        // Members
        self.C = C.Header; // The configuration of the controls
        self.paper; // The SVG paper on which the buttons are drawn
        self.slope; // The slope of the buttons
        // The buttons properties
        self.defaultBackground;
        self.search = {icon: 0, background: 0, link: 0};
        self.settings = {icon: 0, background: 0, link: 0};
        self.help = {icon: 0, background: 0, link: 0};
        self.disconnect = {icon: 0, background: 0, link: 0};
        
        // Default values
        self.slope = self.C.slopeRatio * C.Desktop.topHeight;
        node.controls.css("width", self.C.paperWidth);
        self.paper = Raphael(node.controls[0], "100%", C.Desktop.topHeight);
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
        var path = self.paper.path("M0," + C.Desktop.topHeight + "L" + self.slope + " 0,H" + width + ",l" + -self.slope + " " + C.Desktop.topHeight + "z");
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
