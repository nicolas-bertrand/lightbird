/* This file manages the desktop. */

// The desktop singleton
var gl_desktop;

function Desktop()
{
    // Stores the nodes of the desktop
    this.node = new Object();
    this.node.desktop = document.getElementById("desktop");
    this.node.top = getElementsByClassName("top", this.node.desktop, true);
    this.node.menus = getElementsByClassName("menus", this.node.top, true);
    this.node.tasks = getElementsByClassName("tasks", this.node.top, true);
    this.node.middle = getElementsByClassName("middle", this.node.top, true);
    this.node.player_document = getElementsByClassName("player_document", this.node.middle, true);
    this.node.main = getElementsByClassName("main", this.node.middle, true);
    this.node.player_media = getElementsByClassName("player_media", this.node.desktop, true);
    // Set the default size
    this.node.desktop.style.minWidth = C.Desktop.minWidth + "px";
    this.node.desktop.style.minHeight = C.Desktop.minHeight + "px";
    this.node.player_document.style.height = C.Desktop.player_document_height + "px";
    this.node.player_media.style.height = C.Desktop.player_media_height + "px";
}

// Resize the desktop
Desktop.prototype.onResize = function ()
{
    var height = (gl_browserSize.height > C.Desktop.minHeight) ? gl_browserSize.height : C.Desktop.minHeight;
    var top_height = height - C.Desktop.player_media_height;
    this.node.top.style.height = top_height + "px";
    this.node.main.style.height = (top_height - C.Desktop.player_document_height) + "px";
}
