// Manages the account.
var gl_account;

function Account()
{
    var self = this;
    gl_account = self;
    
    self.init = function ()
    {
        self.identified = false; // If the user is identified on the server
        self.identification = false; // If the identification is in progress. Avoid multiple identifications at the same time.
        self.remember = true; // If the connection has to be remembered in order to identify the user directly at the next reload of the page
        self.disconnecting = false; // Ensures that there is only one disconnection at the same time
        self.background = $("#background>.desktop")[0]; // The background displayed when the user is identified
        self.idenfificationBackground = $("#background>.identification")[0]; // The background displayed while the user is not identified
        
        // Checks if the user is identified to an account
        self.checkIdentification();
    }
   
    // If the user is already identified, the desktop is loaded directly.
    // Otherwise, the idenfication form is displayed.
    self.checkIdentification = function ()
    {
        var identification = document.getElementById("identification");
        
        // Translates the value of the user input
        identification.getElementsByTagName("input")[0].value = T.Identification.user;
        
        // Checks if the session has been remembered
        if (localStorage.getItem("remember") != "false")
            localStorage.setItem("remember", "true");
        if (localStorage.getItem("remember") == "false")
        {
            self.remember = false;
            // Tells the server that the session can be destroyed
            if (!gl_loaded)
                request("GET", "command/disconnect");
            // Deletes the session cookie and the identifiant
            setCookie("sid", "", 0);
            localStorage.removeItem("identifiant");
            document.getElementById("identification_icon_blue_lock").style.display = "none";
        }
        else
            document.getElementById("identification_icon_blue_unlock").style.display = "none";
        
        // Gets the value of the session cookie
        var sid = getCookie("sid");
        var identifiant = localStorage.getItem("identifiant");
        // If the sid and the identifiant cookies are defined, we try to identify the user
        if (sid.length > 0 && identifiant != undefined)
        {
            var callback = function (HttpRequest)
            {
                // The user is identified
                if (HttpRequest.status == 200)
                {
                    self.identified = true;
                    animation(document.getElementById("desktop"), 0, animationOpacity, true);
                    animation(self.background, 0, animationOpacity, true);
                    gl_files.getFiles();
                }
                // Displays the identification panel
                else
                {
                    animation(identification, 0, animationOpacity, true, null, 0);
                    animation(self.background, 0, animationOpacity, false);
                }
            }
            // Tries to identify the user
            request("GET", "blank", callback);
        }
        // Displays the identification panel if the user is not connected
        else
        {
            animation(identification, 0, animationOpacity, true, null, 0);
            animation(self.background, 0, animationOpacity, false);
            // Displays the background the first time the page is loaded
            if (!gl_loaded)
                animation(self.idenfificationBackground, 0, animationOpacity, true, null, 0, 10);
        }
    }

    // Tries to identify the user, using the input of the identification form.
    self.identify = function ()
    {
        // Only one identification can be performed at the same time
        if (self.identification)
            return ;
        self.identification = true;

        var blueButton = document.getElementById("identification_submit_button_blue");
        var yellowButton = document.getElementById("identification_submit_button_yellow");
        var greenButton = document.getElementById("identification_submit_button_green");
        var redButton = document.getElementById("identification_submit_button_red");
        var yellowIcon = document.getElementById("identification_icon_yellow");
        var greenIcon = document.getElementById("identification_icon_green");
        var redIcon = document.getElementById("identification_icon_red");
        
        // Displays the loading image
        animation(yellowIcon, 500, animationOpacity, true);
        // If the red button is displayed, it is hide before the identification
        if (self.isDisplayed(redButton))
        {
            animation(redButton, 500, animationOpacity, false);
            animation(redIcon, 500, animationOpacity, false);
        }
        // Hides the blue button
        else
        {
            animation(blueButton, 500, animationOpacity, false);
            self.displayLock(false);
        }
        
        // Gets the values of the inputs
        var inputs = document.getElementById("identification").getElementsByTagName("input");
        var name = inputs[0].value;
        var password = inputs[1].value;
        
        // Checks if the identification was successful
        var identify = function (HttpRequest)
        {
            // The user is identified
            if (HttpRequest.status == 200)
            {
                self.identified = true;
                // Replaces the yellow button by the green one
                animation(yellowButton, 500, animationOpacity, false);
                animation(yellowIcon, 500, animationOpacity, false);
                animation(greenIcon, 500, animationOpacity, true);
                // Displays the identification panel
                animation(greenButton, 500, animationOpacity, true, function()
                {
                    animation(document.getElementById("identification"), 0, animationOpacity, false, function()
                    {
                        inputs[0].value = "Utilisateur";
                        inputs[1].value = "Password";
                        inputs[0].className = "blur";
                        inputs[1].className = "blur";
                        greenButton.style.display = "none";
                        blueButton.style.display = "block";
                        self.displayLock(true);
                        greenIcon.style.display = "none";
                        changeOpacity(blueButton, 1);
                        self.identification = false;
                    }, 0);
                    animation(document.getElementById("desktop"), 0, animationOpacity, true, null, 0);
                    animation(self.background, 0, animationOpacity, true, null, 0);
                });
                gl_files.getFiles();
            }
            // Otherwise a wrong password has been gived
            else
            {
                // Replaces the yellow button by the red one
                animation(yellowButton, 500, animationOpacity, false);
                animation(yellowIcon, 500, animationOpacity, false);
                animation(redButton, 500, animationOpacity, true, function() { self.identification = false; });
                animation(redIcon, 500, animationOpacity, true);
                localStorage.removeItem("identifiant");
            }
        }
        
        // Now that we have the salt, we can generate the identifiant using the data privided by the user
        var generateIdentifiant = function(HttpRequest)
        {
            localStorage.setItem("identifiant", SHA256(name + SHA256(password + HttpRequest.responseText) + getCookie("sid")));
            request("GET", "command/identify", identify);
        }

        // Gets the salt from the account name, that will allow us to generate the identifiant
        var salt = getUuid();
        animation(yellowButton, 500, animationOpacity, true, function(){request("GET", "command/identify?name=" + SHA256(name + salt) + "&salt=" + salt, generateIdentifiant);});
    }

    // This method is called when the focus is on an identification form, and adds
    // or removes the default value.
    // @focus : True if form is focus, false if it is blur.
    // @value : The default value of the form.
    // @form : The input concerned.
    self.onFocus = function (focus, value, form)
    {
        if (focus == true && form.value == value)
        {
            form.value = "";
            form.className = "focus";
        }
        else if (focus == false && form.value == "")
        {
            form.value = value;
            form.className = "blur";
        }
    }

    // Replaces the error button by the connect button if the error is displayed.
    self.cleanError = function ()
    {
        if (self.isDisplayed(document.getElementById("identification_submit_button_red")) && self.identification == false)
        {
            animation(document.getElementById("identification_submit_button_red"), 500, animationOpacity, false);
            animation(document.getElementById("identification_submit_button_blue"), 500, animationOpacity, true);
            animation(document.getElementById("identification_icon_red"), 500, animationOpacity, false);
            self.displayLock(true);
        }
    }

    // Disconnects the user from the current session
    self.disconnect = function ()
    {
        // Checks if the client is not already disconnecting
        if (self.disconnecting == true)
            return ;
        self.disconnecting = true;
        // We are no longer identified
        self.identified = false;
        setCookie("sid", "", 0);
        localStorage.removeItem("identifiant");
        // Closes the all the pages
        gl_desktop.disconnect();
        // Hides the desktop and the background
        animation(document.getElementById("desktop"), 0, animationOpacity, false, function() {self.disconnecting = false; self.checkIdentification();});
        animation(self.background, 0, animationOpacity, false, null, 0);
        // Displays the identification background
        self.idenfificationBackground.style.display = "block";
        // Tells the server that the client want to disconnect
        request("GET", "command/disconnect");
    }

    // Handles the lock/unlock button that allows user to stay connected when the page is refreshed.
    self.changeLock = function (displayLock)
    {
        lock = document.getElementById("identification_icon_blue_lock");
        unlock = document.getElementById("identification_icon_blue_unlock");
        if ((lock.style.opacity < 1 && lock.style.opacity > 0) || (unlock.style.opacity < 1 && unlock.style.opacity > 0))
            return ;
        if (displayLock)
        {
            animation(lock, 250, animationOpacity, true);
            animation(unlock, 250, animationOpacity, false);
            localStorage.setItem("remember", "true");
            self.remember = true;
        }
        else
        {
            animation(unlock, 250, animationOpacity, true);
            animation(lock, 250, animationOpacity, false);
            localStorage.setItem("remember", "false");
            self.remember = false;
        }
    }

    self.displayLock = function (display)
    {
        lock = document.getElementById("identification_icon_blue_lock");
        unlock = document.getElementById("identification_icon_blue_unlock");
        if (display)
        {
            if (self.remember)
                animation(lock, 500, animationOpacity, true);
            else
                animation(unlock, 500, animationOpacity, true);
        }
        else
        {
            if (self.remember)
                animation(lock, 500, animationOpacity, false);
            else
                animation(unlock, 500, animationOpacity, false);
        }
    }

    // Returns true if the node in parameter is displayed, with the full opacity.
    self.isDisplayed = function (node)
    {
        if (node.style.display == "block" && (node.style.opacity == 1 || node.style.opacity == undefined))
            return (true);
        return (false);
    }
    
    // Returns true if the user is identified.
    self.isIdentified = function ()
    {
        return (self.identified);
    }
    
    self.init();
    return (self);
}
