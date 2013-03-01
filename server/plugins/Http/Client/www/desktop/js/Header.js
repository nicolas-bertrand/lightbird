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
        self.node.icons = self.node.top.children(".icons");

        // Members
        
        // Default values
        self.node.top.height(C.Desktop.topHeight);
        self.addIcon(self.node.icons.children(".search").children("div")[0], gl_svg.Header.search, 10);
        self.addIcon(self.node.icons.children(".settings").children("div")[0], gl_svg.Header.settings, 10.5);
        self.addIcon(self.node.icons.children(".help").children("div")[0], gl_svg.Header.help, 16.4);
        self.addIcon(self.node.icons.children(".disconnect").children("div")[0], gl_svg.Header.disconnect, 11);
        
        // Events
    }

    // Creates an icon and adds it to the destination.
    self.addIcon = function (destination, path, left, top)
    {
        var paper = Raphael(destination, 46, 30);
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
        $(destination.parentNode).click(function (e) { self.clickIcon(e); });
    }

    // Creates an icon and adds it to the destination.
    self.clickIcon = function (e)
    {
        if ($(e.delegateTarget).hasClass("disconnect"))
            gl_account.disconnect();
    }
    
    self.init();
    return (self);
}
