/* This file contains the main scripts of the client. */

// True if the page has been loaded
var gl_loaded = false;
// Holds the size of the browser
var gl_browserSize;

// Initialize the page (called by onload)
function load()
{
	if (gl_loaded)
		return ;
	// Initialize the resources
	gl_resources = new Resources();
	// Initialize the desktop
	gl_desktop = new Desktop();
	// Initialize the browser size
	onResize();
	// onResize is called every time the browser is resized
	window.onresize = onResize;
	// Initialize the Account management system
	initializeAccount();
	// The right click is disabled in order to replace the normal contextual menu of the browser.
	//document.oncontextmenu = function() { return (false); };
	document.getElementById("loading_client").style.display = "none";
	// Ensure that the load function will not be called twice
	gl_loaded = true;
}

// This function is called when the size of the browser change
function onResize()
{
	// Get the new browser size
	gl_browserSize = getBrowserSize();
	// Update the background
	adjustBackgroundSize();
    // Resize the desktop
    gl_desktop.onResize();
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

	// Add a random string in the uri to prevent the result being cached
	url += (url.indexOf("?") == -1 ? "?" : "&");
	url += "r=" + Math.random();
	// Add the token that identify the client : SHA256(identifiant + date + url)
	if (localStorage.getItem("identifiant") != undefined)
	{
		var location = url.substring(0, (url.indexOf("?") != -1 ? url.indexOf("?") : url.length));
		url += "&token=" + SHA256(localStorage.getItem("identifiant") + ISODateString(new Date()) + location);
	}

	// Execute the request
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

// Converts a date in a ISO8601 string, without T&Z
function ISODateString(d)
{
  function pad(n){return n<10 ? '0'+n : n}
  return d.getUTCFullYear()+'-'
      + pad(d.getUTCMonth()+1)+'-'
      + pad(d.getUTCDate())+' '
      + pad(d.getUTCHours())+':'
      + pad(d.getUTCMinutes());
}

// Generate a random string.
// @param size : The size of the string returned.
function randomString(size)
{
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for(var i = 0; i < size; i++)
        text += possible.charAt(Math.floor(Math.random() * possible.length));
    return (text);
}

// Translate the text in the correct language
function tr(text)
{
    if (text)
        document.write(text);
    else
        document.write("Untranslated");
}

