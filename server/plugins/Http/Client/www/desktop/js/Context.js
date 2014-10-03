// Manages the context menus.
var gl_context;

function Context()
{
    var self = this;
    gl_context = self;
    
    self.init = function ()
    {
        self.root = $("body>#context");

        // Mouse wheel event
        if (C.Context.cyclicScroll)
            self.root.mousewheel(self.mouseWheel);

        self.hide();
    }
    
    // Displays the context menu.
    // @param x, y: The position of the mouse.
	// @param actions: The list of the actions to add to the context (can be created with createAction()).
    // If undefined, the context menu is displayed as is.
    // @param reverseActions: If true, the order of the actions is reversed if the bottom of the context menu is displayed close to the mouse.
    self.display = function (x, y, actions, reverseActions)
    {
        if (actions)
        {
            self.root.css("height", "");
            self.root.css("width", "");
            self.root.removeClass();
            self.root.children().remove();
            for (var i = 0; i < actions.length; ++i)
                self.root.append(actions[i]);
        }
        self.root.css("display", "block");
        
        // Sets the position
        var width = self.root.width();
        var height = self.root.height();
        var inverse = false;
        if (x + width > gl_browserSize.width)
            x = gl_browserSize.width - width;
        if (y + height > gl_browserSize.height)
            if (y > gl_browserSize.height / 2)
            {
                y -= height;
                if (reverseActions)
                    self.root.append(self.root.children().get().reverse());
                inverse = true;
            }
        
        // The displays the scroll bar if the menu is too big
        if ((!inverse && y + height > gl_browserSize.height) || (inverse && y < 0))
        {
            self.root.addClass("scroll");
            self.root.scrollTop(0);
            if (!inverse)
                self.root.height(gl_browserSize.height - y);
            else
            {
                self.root.height(y + height);
                y = 0;
                if (reverseActions)
                    self.root.scrollTop(height);
            }
        }
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
    
    // Rotates the actions in a cyclic manner.
    self.mouseWheel = function (e, delta)
    {
        if (self.root.hasClass("scroll"))
            return ;
        if (delta > 0)
            self.root.prepend(self.root[0].lastChild);
        else if (delta < 0)
            self.root.append(self.root[0].firstChild);
    }
    
    // Creates an action that can be added to the root div of the context.
    // @param name: The text displayed in the action.
    // @param handler: The function called when the action is fired. It takes param and the action node in parameter.
    // @param param: The parameter passed to the handler when it is called.
    // @param hide: False if the context is not hidden when the action is fired. True is the default.
    self.createAction = function (name, handler, param, hide)
    {
        var action = $('<div class="action">' + name + '</div>');
            action.mouseup(function (e)
            {
                if (e.which == 1)
                {
                    if (handler)
                        handler(param, action);
                    if (hide !== false)
                        self.hide();
                }
            });
        return action;
    }
    
    self.getRoot = function ()
    {
        return self.root;
    }
    
    self.init();
    return (self);
}
