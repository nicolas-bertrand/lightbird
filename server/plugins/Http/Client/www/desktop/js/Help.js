/* Manage the help interface, which is displayed on the top of the screen. */

var help_over = false;	// True if the mouse is on the help panel, false otherwise.
var help_selected_menu;	// Point to the selected menu node.

// Display or hide the help panel, depending on the event.
function help(event)
{
	var id = document.getElementById("help");

	// Display the help
	if (id.style.display != "block")
	{
		if (document.getElementById("help_text").innerHTML.length == 0)
			loadHelp(document.getElementById("help_home"), "home.html");
		id.style.display = "block";
		animation(id, 500, animationOpacity, true);
	}
	// Hide it
	else if (!help_over && event.button < 2)
		animation(id, 205, animationOpacity, false);
}

// Load a part of the help
function loadHelp(link, file)
{
	if (link.className == "help_selected")
		return ;

	// Change the selected menu
	if (help_selected_menu != undefined)
		help_selected_menu.className = "";
	link.className = "help_selected";
	help_selected_menu = link;
	
	// Clear the old help text
	document.getElementById("help_text").innerHTML = "";
	
	// Display the loading image
	document.getElementById("help_loading").style.display = "block";
	
	// This function handle the response of the server
	var callback = function (HttpRequest)
	{
		if (HttpRequest.status == 200)
			// Display the content of the help
			document.getElementById("help_text").innerHTML = HttpRequest.responseText;
		else
			document.getElementById("help_text").innerHTML = "<h2>Erreur</h2>Malheureusement, l'aide est introuvable =/";
		// Hide the loading image
		document.getElementById("help_loading").style.display = "none";
	}
	
	// Ask the help file to the server
	request("GET", "help/help/" + file, callback);
}

/******************************************************************************
************************************ HINTS ************************************
******************************************************************************/
var gl_hints = new Object();	// Contains all the hints already loaded.

// Display or hide a hint.
// @param info : If info is false, the hint is hide. Otherwise, the value of info
// is the name of the hint to display on the screen.
function hint(info)
{
	var hint = document.getElementById("hint");
	// Display the hint
	if (info != false)
	{
		// If the hint has already been loaded
		if (gl_hints[info] != undefined)
		{
			// If the value if false, the hint has not been found on the server
			if (gl_hints[info] == false)
				return ;
			// Display the cached hint
			document.getElementById("hint_title").innerHTML = gl_hints[info].title;
			// The content span is removed and created because of a refresh bug on webkit
			var parent = document.getElementById("hint_content").parentNode;
			parent.removeChild(document.getElementById("hint_content"));
			parent.innerHTML = "<span id=\"hint_content\">" + gl_hints[info].content + "</span>";
			animation(hint, 200, animationOpacity, true);
		}
		// Ask the hint file to the server
		else
			request("GET", "help/hint/" + info + ".html", function (HttpRequest)
			{
				// If the hint exists
				if (HttpRequest.status == 200)
				{
					var text = HttpRequest.responseText;
					gl_hints[info] = new Object();
					gl_hints[info].title = text.slice(0, text.indexOf('\n'));
					gl_hints[info].content = text.slice(text.indexOf('\n') + 1);
					document.getElementById("hint_title").innerHTML = gl_hints[info].title;
					// The content span is removed and created because of a refresh bug on webkit
					var parent = document.getElementById("hint_content").parentNode;
					parent.removeChild(document.getElementById("hint_content"));
					parent.innerHTML = "<span id=\"hint_content\">" + gl_hints[info].content + "</span>";
					animation(hint, 200, animationOpacity, true);
				}
				// Otherwise, the page will never been asked again
				else
					gl_hints[info] = false;
			}, true);
	}
	else if (hint.style.display == "block")
		animation(hint, 200, animationOpacity, false);
}
