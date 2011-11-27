/* Stores the scripts related to the management of the user. */

// Initialize the global variables
var gl_identified = false;		// If the user is identified on the server.
var gl_identification = false;	// If the identification is in progress. Avoid multiple identifications at the same time.
var gl_identificationTimer;		// If the user is not identified, a new session id is created every 30 seconds with this timer.
var gl_identificationRemember = true;	// If the connection has to be remembered in order to identify the user directly at the next reload of the page.
var gl_disconnecting = false;	// Ensure that there is only one disconnection at the same time.

// Initialize the Account scripts
function initializeAccount()
{
	// Launch the identification timer
	gl_identificationTimer = setTimeout("getNewSessionId(true)", C.identificationTimerTimeout);
	// Check if the user is identifies to an account
	checkIdentification();
}

// If the user is already identified, the desktop is loaded directly.
// Otherwise, the idenfication form is displayed.
function checkIdentification()
{
	var identification = document.getElementById("identification");

	// If the browser is Internet Explorer, the padding of the identification forms is modified
	if (/MSIE (\d+\.\d+);/.test(navigator.userAgent))
	{
		var forms = identification.getElementsByTagName("input");
		for (i = 0; i < forms.length; ++i)
		{
			forms[i].style.paddingTop = "9px";
			forms[i].style.height = "29px";
		}
	}
	
	// Check if the session has been remembered
	if (localStorage.getItem("remember") != "false")
		localStorage.setItem("remember", "true");
	if (localStorage.getItem("remember") == "false")
	{
		gl_identificationRemember = false;
		// Tells the server that the session can be destroyed
		request("GET", "Execute/Disconnect");
		// Delete the session cookie and the identifiant
		setCookie("sid", "", 0);
		localStorage.removeItem("identifiant");
		document.getElementById("identification_icon_blue_lock").style.display = "none";
	}
	else
		document.getElementById("identification_icon_blue_unlock").style.display = "none";
	
	// Get the value of the session cookie
	var sid = getCookie("sid");
	var identifiant = localStorage.getItem("identifiant");
	// If the sid and the identifiant cookies are defined, we try to identify the user
	if (sid.length > 0 && identifiant != undefined)
	{
		var callback = function (HttpRequest)
		{
			// The user id identified
			if (HttpRequest.status == 200)
			{
				gl_identified = true;
				document.getElementById("menus").style.display = "block";
				animation(document.getElementById("menus"), 2000, animationOpacity, true);
				animation(document.getElementById("background"), 250, animationOpacity, true, null);
			}
			// Display the identification panel
			else
			{
				animation(identification, 1000, animationOpacity, true, null, 250);
				animation(document.getElementById("background"), 250, animationOpacity, false, null);
			}
		}
		// Try to identify the user
		request("GET", "blank", callback);
	}
	// Display the identification panel if the user is not connected
	else
	{
		animation(identification, 1000, animationOpacity, true, null, 250);
		animation(document.getElementById("background"), 250, animationOpacity, false, null);
		getNewSessionId(false);
		// Display the background the first time the page is loaded
		if (!gl_loaded)
			animation(document.getElementById("background_identification"), 500, animationOpacity, true, null, 0, 10);
	}
}

// This method is called when the focus is on an identification form, and adds
// or removes the default value.
// @focus : True if form is focus, false if it is blur.
// @value : The default value of the form.
// @form : The input concerned.
function focusOnIdentificationForm(focus, value, form)
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

// Try to identify the user, using the input of the identification form
function identification()
{
	// Only one identification can be performed at the same time
	if (gl_identification)
		return ;
	gl_identification = true;

	var blueButton = document.getElementById("identification_submit_button_blue");
	var yellowButton = document.getElementById("identification_submit_button_yellow");
	var greenButton = document.getElementById("identification_submit_button_green");
	var redButton = document.getElementById("identification_submit_button_red");
	var yellowIcon = document.getElementById("identification_icon_yellow");
	var greenIcon = document.getElementById("identification_icon_green");
	var redIcon = document.getElementById("identification_icon_red");
	
	// Display the loading image
	animation(yellowIcon, 500, animationOpacity, true);
	// If the red button is displayed, it is hide before the identification
	if (isDisplayed(redButton))
	{
		animation(redButton, 500, animationOpacity, false);
		animation(redIcon, 500, animationOpacity, false);
	}
	// Hide the blue button
	else
	{
		animation(blueButton, 500, animationOpacity, false);
		identificationDisplayLock(false);
	}
	
	// Get the values of the inputs
	var inputs = document.getElementById("identification").getElementsByTagName("input");
	var name = inputs[0].value;
	var password = inputs[1].value;
	
	// Checks if the identification was successful
	var identify = function (HttpRequest)
	{
		// The user is identified
		if (HttpRequest.status == 200)
		{
			gl_identified = true;
			// Replace the yellow button by the green one
			animation(yellowButton, 500, animationOpacity, false);
			animation(yellowIcon, 500, animationOpacity, false);
			animation(greenIcon, 500, animationOpacity, true);
			// Display the identification panel
			animation(greenButton, 500, animationOpacity, true, function()
			{
				animation(document.getElementById("identification"), 500, animationOpacity, false, function()
				{
					inputs[0].value = "Utilisateur";
					inputs[1].value = "Password";
					inputs[0].className = "blur";
					inputs[1].className = "blur";
					greenButton.style.display = "none";
					blueButton.style.display = "block";
					identificationDisplayLock(true);
					greenIcon.style.display = "none";
					changeOpacity(blueButton, 1);
					gl_identification = false;
				}, 500);
				animation(document.getElementById("background"), 500, animationOpacity, true, null, 750);
				animation(document.getElementById("menus"), 2000, animationOpacity, true, null, 500);
			});
		}
		// Otherwise a wrong password has been gived
		else
		{
			// Replace the yellow button by the red one
			animation(yellowButton, 500, animationOpacity, false);
			animation(yellowIcon, 500, animationOpacity, false);
			animation(redButton, 500, animationOpacity, true, function() { gl_identification = false; });
			animation(redIcon, 500, animationOpacity, true);
			localStorage.removeItem("identifiant");
		}
	}
	
	// Now that we have the salt, we can generate the identifiant using the data privided by the user
	var generateIdentifiant = function(HttpRequest)
	{
		localStorage.setItem("identifiant", SHA256(name + SHA256(password + HttpRequest.responseText) + getCookie("sid")));
		request("GET", "Execute/Identify", identify);
	}

	// Get the salt from the account name, that will allow us to generate the identifiant
	var salt = randomString(32);
	animation(yellowButton, 500, animationOpacity, true, function(){request("GET", "Execute/Identify?name=" + SHA256(name + salt) + "&salt=" + salt, generateIdentifiant);});
}

// Replace the error button by the connect button if the error is displayed
function identificationCleanError()
{
	if (isDisplayed(document.getElementById("identification_submit_button_red")) && gl_identification == false)
	{
		animation(document.getElementById("identification_submit_button_red"), 500, animationOpacity, false);
		animation(document.getElementById("identification_submit_button_blue"), 500, animationOpacity, true);
		animation(document.getElementById("identification_icon_red"), 500, animationOpacity, false);
		identificationDisplayLock(true);
	}
}

// Disconnect the user from the current session
function disconnection()
{
	// Check if the client is not already disconnecting
	if (gl_disconnecting == true)
		return ;
	gl_disconnecting = true;
	var callback = function (HttpRequest)
	{
		// The server has disconnected the user
		gl_identified = false;
		setCookie("sid", "", 0);
		localStorage.removeItem("identifiant");
		var next;
		// Close the windows
		for (id in gl_windows)
			closeWindow(id);
		// Hide the help
		if (document.getElementById("help").style.display == "block")
			animation(document.getElementById("help"), 1000, animationOpacity, false);
		// Hide the menu and the background
		animation(document.getElementById("menus"), 1000, animationOpacity, false, function() {gl_disconnecting = false; checkIdentification();});
		animation(document.getElementById("background"), 250, animationOpacity, false, null, 250);
		// Display the identification background
		document.getElementById("background_identification").style.display = "block";
		// Get a new session id
		getNewSessionId(false);
	}
	
	// Tells the server that the client want to disconnect
	request("GET", "Execute/Disconnect", callback);
}

// If the user is not identified, a new session id is get every 30 seconds via this method.
// It is called by the timer gl_identificationTimer.
// @param repeat : If the timer has to be repeated
function getNewSessionId(repeat)
{
	// A new id is get only if the user is not identified
	if (gl_identified == false)
	{
		// Remove the session cookie
		var sid = setCookie("sid", "", 0);
		// Get the new session id
		request("GET", "blank");
	}
	// Repeat the timer
	if (repeat == true)
		gl_identificationTimer = setTimeout("getNewSessionId(true)", C.identificationTimerTimeout);
}

// Handle the lock/unlock button that allows user to stay connected when the page is refreshed.
function identificationChangeLock(displayLock)
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
		gl_identificationRemember = true;
	}
	else
	{
		animation(unlock, 250, animationOpacity, true);
		animation(lock, 250, animationOpacity, false);
		localStorage.setItem("remember", "false");
		gl_identificationRemember = false;
	}
}

function identificationDisplayLock(display)
{
	lock = document.getElementById("identification_icon_blue_lock");
	unlock = document.getElementById("identification_icon_blue_unlock");
	if (display)
	{
		if (gl_identificationRemember)
			animation(lock, 500, animationOpacity, true);
		else
			animation(unlock, 500, animationOpacity, true);
	}
	else
	{
		if (gl_identificationRemember)
			animation(lock, 500, animationOpacity, false);
		else
			animation(unlock, 500, animationOpacity, false);
	}
}

// Returns true if the node in parameter is displayed, with the full opacity.
function isDisplayed(node)
{
	if (node.style.display == "block" && (node.style.opacity == 1 || node.style.opacity == undefined))
		return (true);
	return (false);
}
