// Manages the context menus.
var gl_context;

function Context()
{
    var self = this;
    gl_context = self;
    
    self.init = function ()
    {
        self.root = $("body>#context");
        self.hide();
    }
    
    // Returns the root of the context menu.
    self.getRoot = function ()
    {
        return self.root;
    }
    
    // Displays the context menu.
    // @param x, y: The position of the mouse.
	// @param actions: The list of the actions in the menu: [{name, handler, param}]
    // If undefined, the context menu is displayed as is.
    self.display = function (x, y, actions)
    {
        if (actions)
        {
            self.root.children().remove();
            for (var i = 0; i < actions.length; ++i)
            {
                var action = $('<div class="action">' + actions[i].name + '</div>');
                action[0].contextAction = actions[i];
                action.mouseup(function (e) { if (e.which == 1) { e.delegateTarget.contextAction.handler(e.delegateTarget.contextAction.param); self.hide(); }});
                self.root.append(action);
            }
        }
        self.root.css("display", "block");
        self.root.css("top", y);
        self.root.css("left", x);
        // Hides the context menu when the mouse is down. Delays the binding using setTimeout so that hide() is not called directly after this function.
        gl_desktop.events.unbind("mousedown", self);
        setTimeout(function() { gl_desktop.events.bind("mousedown", self, self.hide); }, 0);
    }
    
    // Hides the context menu.
    self.hide = function (e)
    {
        // Hides the context menu if the mouse is outside it.
        if (!e || (e.target != self.root[0] && e.target.parentNode != self.root[0]))
        {
            self.root.css("display", "none");
            gl_desktop.events.unbind("mousedown", self);
        }
    }
    
    // Returns true if the context menu is displayed.
    self.isDisplayed = function ()
    {
        return (self.root.css("display") == "block");
    }
    
    self.init();
    return (self);
}
