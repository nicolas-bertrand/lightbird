// Manages the user account.
var gl_user;

function User()
{
    var self = this;
    gl_user = self;
    var node = new Object();
    
    self.init = function ()
    {
        // Nodes
        node.background = $("#background");
        node.background.image = node.background.children("img");
        node.authentication = $("#authentication");
        node.user = node.authentication.find("input.user");
        node.password = node.authentication.find("input.password");
        node.user.background = node.authentication.find(".background>.user");
        node.password.background = node.authentication.find(".background>.password");
        node.user.focus = node.authentication.find(".focus>.user");
        node.password.focus = node.authentication.find(".focus>.password");
        node.user.test = node.authentication.find(".test_text>.user");
        node.password.test = node.authentication.find(".test_text>.password");
    
        // Members
        self.authenticated = false; // True while the user is authenticated on the server
        self.authentication = false; // True while the authentication is in progress. Avoids multiple authentication at the same time.
        self.disconnecting = false; // True while the disconnection is in progress. Avoids multiple disconnections at the same time.
        self.remember = true; // True if the connection has to be remembered in order to authenticate the user directly at the next reload of the page.
        
        // Default values
        node.user.defaultText = T.Authentication.user;
        node.password.defaultText = T.Authentication.password;
        self.updateInput(node.user);
        self.updateInput(node.password);
        
        // Checks if the user is already authenticated
        self.checkAuthentication();
        
        // Events
        node.user.keydown(function () { setTimeout(function () { self.updateInput(node.user); }, 0); });
        node.password.keydown(function () { setTimeout(function () { self.updateInput(node.password); }, 0); });
        node.authentication.find("form").submit(function (e) { self.authenticate(e); e.preventDefault(); });
    }
   
    // Display directly the desktop if the user is already authenticated.
    // Otherwise the authentication form is displayed.
    self.checkAuthentication = function ()
    {
        // Checks if the session has been remembered
        if (localStorage.getItem("remember") != "false")
            localStorage.setItem("remember", "true");
        if (localStorage.getItem("remember") == "false")
        {
            self.remember = false;
            // Tells the server that the session can be destroyed, if there is one
            if (!gl_loaded)
                F.request("GET", "command/disconnect");
            // Removes the session id and the identifiant
            localStorage.removeItem("sessionId");
            localStorage.removeItem("identifiant");
        }
        
        // If the session id and the identifiant are defined, we try to authenticate the user
        if (localStorage.getItem("sessionId") && localStorage.getItem("identifiant"))
        {
            // Tries to authenticate the user
            F.request("GET", "blank", function (HttpRequest)
            {
                // The user is authenticated
                if (HttpRequest.status == 200)
                    self.hide();
                // The authentication failed, so we display the form
                else
                    self.display();
                node.background.image.removeClass("hide");
            });
        }
        // Displays the authentication form if the user is not authenticated
        else
        {
            node.background.image.removeClass("hide");
            self.display();
        }
    }

    // Tries to authenticate the user using the inputs of the authentication form.
    self.authenticate = function (e)
    {
        // Gets the values of the inputs
        var name = node.user[0].value;
        var password = node.password[0].value;
        
        // Only one authentication can be performed at the same time
        if (self.authentication || self.authenticated || node.authentication.hasClass("error") || !name)
            return ;
        self.lockAuthentication(true);
        
        // Step 3: Checks the result of the authentication
        var authenticate = function (HttpRequest)
        {
            // The user is authenticated, so we hide the authentication form and display the desktop
            if (HttpRequest.status == 200)
                self.hide();
            // Otherwise the inputs are not valid
            else
            {
                node.authentication.addClass("error");
                localStorage.removeItem("sessionId");
                localStorage.removeItem("identifiant");
            }
            self.lockAuthentication(false);
        }
        
        // Step 2: Now that we have the salt, we can generate the identifiant using the data provided by the user
        var generateIdentifiant = function(HttpRequest)
        {
            if (HttpRequest.status == 200)
            {
                var response = jsonParse(HttpRequest.responseText);
                localStorage.setItem("sessionId", response.sessionId);
                localStorage.setItem("identifiant", SHA256(name + SHA256(password + response.salt) + response.sessionId));
                F.request("GET", "command/identify", authenticate);
            }
            // At this point we are not allowed to connect to the server
            else
            {
                node.authentication.addClass("error");
                self.lockAuthentication(false);
            }
        }

        // Step 1: Gets the salt from the user name, which will allow us to generate the identifiant
        var salt = F.getUuid();
        F.request("GET", "command/identify?name=" + SHA256(name + salt) + "&salt=" + salt, generateIdentifiant);
    }
    
    // Locks / unlocks the inputs during the authentication.
    self.lockAuthentication = function (lock)
    {
        self.authentication = lock;
        node.user[0].disabled = lock;
        node.password[0].disabled = lock;
    }

    // Updates the display of the form based on the value of the input.
    self.updateInput = function (input)
    {
        // The password value is replaced by black circles
        var value = (input == node.user ? input[0].value : input[0].value.replace(/./g, C.User.passwordCharacter));
        if (value == input.oldValue)
            return ;
        input.oldValue = value;
        if (value.length > 0)
        {
            // Computes the width of the input text in order to update the width of the background
            input.test.html(value);
            input.focus.width(input.test.width());
            input.focus.addClass("display");
            // Hides the default text
            input.background.html("");
        }
        // Displays the default text if the input is empty
        else
        {
            input.focus.removeClass("display");
            input.background.html(input.defaultText);
        }
        // Removes the error, if there is one
        node.authentication.removeClass("error");
    }

    // Disconnects destroys the current session.
    self.disconnect = function ()
    {
        // Ensures that we are not already disconnecting
        if (self.disconnecting)
            return ;
        self.disconnecting = true;
        // Tells the desktop that we are disconnecting
        gl_desktop.onDisconnect();
        // Tells the server to destroy the session
        F.request("GET", "command/disconnect");
        // Displays the authentication form
        self.display();
        self.disconnecting = false;
    }

    // Displays the authentication form, and hides the desktop.
    self.display = function ()
    {
        // The user is no longer authenticated
        self.authenticated = false;
        localStorage.removeItem("sessionId");
        localStorage.removeItem("identifiant");
        // Display the form
        node.background.addClass("foreground");
        node.authentication.addClass("display");
        node.user[0].focus();
    }
    
    // Hides the authentication form, and displays the desktop.
    self.hide = function ()
    {
        // The user is identified
        self.authenticated = true;
        gl_desktop.onConnect();
        node.authentication.removeClass("display");
        node.background.removeClass("foreground");
        // Resets the inputs
        node.user[0].value = "";
        node.password[0].value = "";
        setTimeout(function () { self.updateInput(node.user); });
        setTimeout(function () { self.updateInput(node.password); });
    }
    
    // A key has been pressed in the window.
    self.onKeyDown = function (e)
    {
        if (!self.authenticated && !self.authentication)
        {
            // Submits the authentication form if enter is pressed
            if (e.keyCode == "\r".charCodeAt(0))
                node.authentication.find("form").submit();
            // Switches between the inputs when tab is pressed
            else if (e.keyCode == "\t".charCodeAt(0) && C.User.activeElementLoop)
            {
                var activeElement = document.activeElement;
                // Allows time to update the activeElement
                setTimeout(function ()
                {
                    if (document.activeElement != node.user[0] && document.activeElement != node.password[0])
                    {
                        if (activeElement == node.password[0])
                            node.user[0].focus();
                        else
                            node.password[0].focus();
                    }
                }, 0);
            }
        }
    }

    self.init();
    return (self);
}
