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
    
    // Displays the context menu.
    // @param x, y: The position of the mouse.
	// @param actions: The list of the actions to add to the context (can be created with createAction()).
    // If undefined, the context menu is displayed as is.
    self.display = function (x, y, actions)
    {
        if (actions)
        {
            self.root.children().remove();
            for (var i = 0; i < actions.length; ++i)
                self.root.append(actions[i]);
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
        if (!e || (e.target != self.root[0] && e.target.parentNode != self.root[0] && e.target.parentNode.parentNode != self.root[0]))
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
    
    // Creates an action that can be added to the root div of the context.
    self.createAction = function (name, handler, param)
    {
        var action = $('<div class="action">' + name + '</div>');
            action.mouseup(function (e)
            {
                if (e.which == 1)
                {
                    if (handler)
                        handler(param);
                    self.hide();
                }
            });
        return action;
    }
    
    self.init();
    return (self);
}
