/* This file contains the main scripts of the client. */

// True if the page has been loaded
var gl_loaded = false;
// Holds the size of the browser
var gl_browserSize;

// Initialise the page
function load()
{
	if (gl_loaded)
		return ;
	// The right click is disabled in order to replace the normal contextual menu of the browser.
	// This behaviour should be editable via the preferences.
	//document.oncontextmenu = function() { return (false); };
	// Initialise the browser size
	onResize();
	// Initialize the Account management
	initializeAccount();
	// Load the resources
	gl_resources = new Resources();
	// Ensure that the load function will not be called twice
	gl_loaded = true;
	document.getElementById("loading_client").style.display = "none";
}

// This function is called when the size of the browser change
function onResize()
{
	// Get the new browser size
	gl_browserSize = getBrowserSize();
	// Update the background
	adjustBackgroundSize();
	// Call the resize function of all the windows that implements it
	for (var id in gl_windows)
		if (gl_windows[id].onResize != undefined)
			gl_windows[id].onResize();
}

// Ajust the background size according to the size of the browser
function adjustBackgroundSize()
{
	// Set the background size to the screen size
	var background = document.getElementById("background");
	background.style.width = gl_browserSize.width + "px";
	background.style.height = gl_browserSize.height + "px";
	var background_identification = document.getElementById("background_identification");
	background_identification.style.width = gl_browserSize.width + "px";
	background_identification.style.height = gl_browserSize.height + "px";
	
	// Set the size of the desktop (which is above the background)
	var desktop = document.getElementById("desktop");
	desktop.style.width = gl_browserSize.width + "px";
	desktop.style.height = gl_browserSize.height + "px";
	
	// Display the background the first time the page is loaded
	if (!gl_loaded)
	{
		window.onresize = onResize;
		animation(background_identification, 500, animationOpacity, true, null, 0, 10);
	}
}

// Find the size of the browser window
function getBrowserSize()
{
	var width = 1280;
	var height = 1024;
 
	if (typeof(window.innerWidth) == 'number')
	{
		//Non-IE
		width = window.innerWidth;
		height = window.innerHeight;
	}
	else if (document.documentElement && (document.documentElement.clientWidth || document.documentElement.clientHeight))
	{
		//IE 6+ in 'standards compliant mode'
		width = document.documentElement.clientWidth;
		height = document.documentElement.clientHeight;
	}
	else if (document.body && ( document.body.clientWidth || document.body.clientHeight))
	{
		//IE 4 compatible
		width = document.body.clientWidth;
		height = document.body.clientHeight;
	}
	return {width : width,
			height : height};
}

// Execute a Http Request on the server.
// @method : The Http method to execute (GET or POST).
// @url : The url of the Http request.
// @callback : A function that will be called once the responce has been received.
// @cache : If the response can be cached (false by default).
// @data : The data that has to be sent with the POST method.
// The response may be invalid.
function request(method, url, callback, cache, data)
{
	var HttpRequest;

	// Creates the XMLHttpRequest object
	if (window.XMLHttpRequest)
		// code for IE7+, Firefox, Chrome, Opera, Safari
		HttpRequest = new XMLHttpRequest();
	else
		// code for IE6, IE5
		HttpRequest = new ActiveXObject("Microsoft.XMLHTTP");

	// Set the ready state anonym function
	HttpRequest.onreadystatechange = function()
	{
		if (HttpRequest.readyState == 4)
			if (callback != null)
				callback(HttpRequest);
	}

	// Execute the request
	if (cache != true)
	{
		if (url.indexOf("?") != -1)
			url += "&";
		else
			url += "?";
		url += "t=" + Math.random();
	}
	HttpRequest.open(method, "Client/" + url, true);
	if (method.toUpperCase() == "POST")
	{
		HttpRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
		HttpRequest.send(data);
	}
	else
		HttpRequest.send();
	return (HttpRequest);
}

function nl2br(text)
{
	return (text);
	text = escape(text);
	if (text.indexOf('%0D%0A') > -1)
		re_nlchar = /%0D%0A/g;
	else if (text.indexOf('%0A') > -1)
		re_nlchar = /%0A/g;
	else if (text.indexOf('%0D') > -1)
		re_nlchar = /%0D/g;
	if (typeof(renlchar) == "undefined")
		return unescape(text);
	else
		return unescape(text.replace(renlchar, '<br />'));
}

// Create a cookie using the parameters.
// @name : The name of the cookie.
// @value : The value of the cookie.
// @expireDays : The number of days after which the cookie will be destroyed.
// The default value is two years.
function setCookie(name, value, expireDays)
{
	var exdate = new Date();
	if (expireDays == null)
		expireDays = 2 * 365;
	exdate.setDate(exdate.getDate() + expireDays);
	document.cookie = name+ "=" + escape(value) + ((expireDays == null) ? "" : ";expires=" + exdate.toUTCString());
}

// Returns the value of a cookie.
// @param name : The name of the cookie.
// @return The value of the cookie.
function getCookie(name)
{
	if (document.cookie.length > 0)
	{
		c_start = document.cookie.indexOf(name + "=");
		if (c_start != -1)
		{
			c_start = c_start + name.length + 1;
			c_end = document.cookie.indexOf(";", c_start);
			if (c_end == -1)
				c_end = document.cookie.length;
			return (unescape(document.cookie.substring(c_start, c_end)));
		}
	}
	return ("");
}

// Change the opacity of the node, if the browser support it.
function changeOpacity(node, value)
{
	if (node.style.opacity != undefined)
		node.style.opacity = value;
}

// Manage the display of the interfaces buttons when the mouse is over it
// @param active : The node of the actived interface.
// @param over : If the mouse is over or out.
function interfaceOnMouseOver(active, over)
{
	var inactive1;
	var inactive2;
	
	if (active == document.getElementById("interface_desktop"))
	{
		inactive1 = document.getElementById("interface_web");
		inactive2 = document.getElementById("interface_mobile");
	}
	else if (active == document.getElementById("interface_web"))
	{
		inactive1 = document.getElementById("interface_desktop");
		inactive2 = document.getElementById("interface_mobile");
	}
	else
	{
		inactive1 = document.getElementById("interface_desktop");
		inactive2 = document.getElementById("interface_web");
	}
	if (over)
	{
		active.className = "on";
		inactive1.className = "off";
		inactive2.className = "off";
	}
	else
	{
		active.className = "off";
		document.getElementById("interface_desktop").className = "on";
	}
}

// Calculates the coordonates of the mouse
function mouseCoordinates(event)
{
	if (event.pageX || event.pageY)
	{
		return {x:event.pageX, y:event.pageY};
	}
	return {x:event.clientX + document.body.scrollLeft - document.body.clientLeft,
			y:event.clientY + document.body.scrollTop  - document.body.clientTop};
}

// Returns the coordonates of an element
function elementCoordinates(element)
{
	var left = 0;
	var top = 0;

	while (element.offsetParent)
	{
		left += element.offsetLeft;
		top += element.offsetTop;
		element = element.offsetParent;
	}
	left += element.offsetLeft;
	top += element.offsetTop;
	return {x:left, y:top};
}

// Add an event to an element.
// @ element : The element on which the event will be applied.
// @ event : The name of the event to apply (without the "on").
// @ fct : A function to call when the event occured.
function addEvent(element, event, fct)
{
	// IE
	if(element.attachEvent)
		element.attachEvent('on' + event, fct);
	// Others
	else
		element.addEventListener(event, fct, true);
}

// Remove an event to an element.
// @ element : The element on which the event will be removed.
// @ event : The name of the event to remove (without the "on").
// @ fct : A function to call when the event occured.
function removeEvent(element, event, fct)
{
	// IE
	if(element.detachEvent)
		element.detachEvent('on' + event, fct);
	// Others
	else
		element.removeEventListener(event, fct, true);
}

function getElementsByClassName(className, node, first)
{
	if(!node)
		node = document.getElementsByTagName("body")[0];
	if (first != false)
		first = true;
	var result = [];
	var regexp = new RegExp('\\b' + className + '\\b');
	var elements = node.getElementsByTagName("*");
	for(var i = 0, j = elements.length; i < j; i++)
		if (regexp.test(elements[i].className))
		{
			if (first)
				return (elements[i]);
			else
				result.push(elements[i]);
		}
	return (result);
}

// Class name management
function getClassName(node, className)
{
	var regexp = new RegExp('\\b' + className + '\\b');
	if (regexp.test(node.className))
		return (true);
	return (false);
}

function setClassName(node, className)
{
	if (!getClassName(node, className))
		node.className += " " + className;
}

function removeClassName(node, className)
{
	if (getClassName(node, className))
	{
		node.className = node.className.replace(new RegExp('\\b' + className + '\\b'), "");
		node.className = node.className.replace("  ", " ");
	}
}

// Compare too strings
function compare(str1, str2)
{
	for (var i = 0, s = str1.length; i < s; ++i)
		if (str2.length <= i)
			return (1);
		else if (str1[i] < str2[i])
			return (-1);
		else if (str1[i] > str2[i])
			return (1);
	if (str1.length == str2.length)
		return (0);
	return (-1);
}

// Find the target of the event, in a cross browser way.
function getEventTarget(event, name)
{
	target = (event.target || event.srcElement);
	if (name == undefined)
		return (target);
	while (target)
	{
		if (target.tagName.toLowerCase() == name.toLowerCase() ||
		    (target.className.toLowerCase() == name.toLowerCase() &&
			 target.className.length > 0))
			return (target);
		target = target.parentNode;
	}
}

// Same as parseInt, except that NaN is replaced by 0.
function toNumber(number)
{
	if (number == "")
		return (0);
	return (parseInt(number));
}

// If n is lesser than zero, zero is returned.
function positive(n)
{
	if (n < 0)
		return (0);
	return (n);
}