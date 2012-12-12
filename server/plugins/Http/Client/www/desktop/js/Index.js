/* This file contains the main scripts of the client. */

// True if the page has been loaded
var gl_loaded = false;
// Holds the size of the browser
var gl_browserSize;

// Initializes the page (called by onload)
function load()
{
	gl_browserSize = getBrowserSize();
    // Initialize the main systems
	new Resources();
	new Desktop();
	new Windows();
    new Header();
    new Player();
    new Files();
	new Account();
	// Initializes the browser size
	onResize();
	// onResize is called every time the browser is resized
	window.onresize = onResize;
	// The right click is disabled in order to replace the normal contextual menu of the browser.
	//document.oncontextmenu = function() { return (false); };
	document.getElementById("loading_client").style.display = "none";
    // Disables the text selection
    $(document.body).mousedown(function (e) { disableSelection(false); });
    $(document.body).mouseup(function () { disableSelection(true); });
    // Asks a confirmation when the user is about to leave the page.
    //window.onbeforeunload = function() { return ("Leaving now will save the current session."); }
	// Ensures that the load function will not be called twice
	gl_loaded = true;
}

// This function is called when the size of the browser change
function onResize()
{
	// Gets the new browser size
	gl_browserSize = getBrowserSize();
	// Updates the background
	adjustBackgroundSize();
    // Resizes the desktop
    gl_desktop.onResize();
	// Calls the resize function of all the windows that implements it
	for (var id in gl_windows)
		if (gl_windows[id].onResize != undefined)
			gl_windows[id].onResize();
}

// Ajusts the background size according to the size of the browser
function adjustBackgroundSize()
{
	// Sets the background size to the screen size
	var background = document.getElementById("background");
	$("#background").width(gl_browserSize.width);
	$("#background").height(gl_browserSize.height);
}

// Finds the size of the browser window
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
	return { width : (width > C.Desktop.minWidth ? width : C.Desktop.minWidth),
             height : (height > C.Desktop.minHeight ? height : C.Desktop.minHeight) };
}

// Executes a Http Request on the server.
// @method : The Http method to execute (GET or POST).
// @url : The url of the Http request.
// @callback : A function that will be called once the responce has been received.
// @data : The data that has to be sent with the POST method.
// @type : The content-type of the data. The default is "application/x-www-form-urlencoded".
// The response may be invalid.
function request(method, url, callback, data, type)
{
	var HttpRequest;

	// Creates the XMLHttpRequest object
	if (window.XMLHttpRequest)
		// code for IE7+, Firefox, Chrome, Opera, Safari
		HttpRequest = new XMLHttpRequest();
	else
		// code for IE6, IE5
		HttpRequest = new ActiveXObject("Microsoft.XMLHTTP");

	// Sets the ready state anonym function
	HttpRequest.onreadystatechange = function()
	{
		if (HttpRequest.readyState == 4)
			if (callback != null)
				callback(HttpRequest);
	}

	// Adds a random string in the uri to prevent the result from being cached
	url += (url.indexOf("?") == -1 ? "?" : "&");
	url += "r=" + Math.random();
	// Adds the session information that identify the user
    url += getSession();

	// Executes the request
	HttpRequest.open(method, "/Client/" + url, true);
	if (method.toUpperCase() == "POST")
	{
		HttpRequest.setRequestHeader("Content-type", (type ? type : "application/x-www-form-urlencoded"));
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

// Changes the opacity of the node, if the browser support it.
function changeOpacity(node, value)
{
	if (node.style.opacity != undefined)
		node.style.opacity = value;
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

// Returns the name of the object.
function getObjectName(object)
{ 
    var funcNameRegex = /function (.{1,})\(/;
    var results = (funcNameRegex).exec(object.constructor.toString());
    return (results && results.length > 1) ? results[1] : "";
}

// Removes the texts nodes of a node (nodes without a tagName).
function removeTextNodes(node)
{
    var child = node.firstChild;
    
    while (child)
    {
        var previous = child;
        child = child.nextSibling;
        // If the child doesn't have a tag name we remove it
        if (!previous.tagName)
            previous.parentNode.removeChild(previous);
    }
    return (node);
}

// Generates a universally unique identifier.
function getUuid()
{
    var S4 = function()
    {
       return (((1 + Math.random()) * 0x10000)|0).toString(16).substring(1);
    };
    return (S4() + S4() + "-" + S4() + "-" + S4() + "-" + S4()+"-" + S4() + S4() + S4());
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

// Returns the session information that have to be added at the end of the URLs in order to communicate with the server.
// @param useQuestionMark : Prepends the session url a "?". Otherwise a "&" is prepended.
function getSession(useQuestionMark)
{
    var result;

    // The user is not identified
    if (!localStorage.getItem("sid"))
        return (useQuestionMark ? "?" : "");
    result = (useQuestionMark ? "?" : "&") + "sid=" + localStorage.getItem("sid");
    if (localStorage.getItem("identifiant"))
        result += "&identifiant=" + localStorage.getItem("identifiant");
    return (result);
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

// Converts the size to a string like 5,42 Mb.
function sizeToString(size)
{
    size = new String(size);
    if (!size)
        return ("0 " + T.sizeUnits[0]);
    var n = Number(size);
    n = n / Math.pow(1024, parseInt((size.length - 1) / 3));
    var unit = T.sizeUnits[parseInt((size.length - 1) / 3)];
    n = (new String(n)).replace(".", ",").substr(0, 4);
    var c = n.indexOf(",");
    if (c == 3)
        n = n.substr(0, 3);
    if (!Number(n.substr(c + 1, 3)))
        n = n.substr(0, c);
    if (!n)
        n = 0;
    return (n + " " + unit);
}

// Translates the text in the correct language.
function tr(text)
{
    if (text)
        document.write(text);
    else
        document.write("Untranslated");
}

// Translates an entire resource.
// The elements to translate are in the node <script>tr(T.text)</script>.
// This method replaces these nodes by their translation.
// @brief resource : The html node of the resource to translate.
function translate(resource)
{
    var scripts = resource.getElementsByTagName("script");
    
    for (var i = 0; i < scripts.length; ++i)
    {
        var tr = scripts[i].innerHTML;
        if (tr.indexOf("tr(") == 0)
        {
            tr = tr.substring(5, tr.length - 1).split('.');
            var t = T;
            for (var j = 0; j < tr.length; ++j)
                t = t[tr[j]];
            var text = document.createElement("span");
            text.innerHTML = t;
            scripts[i].parentNode.insertBefore(text, scripts[i]);
        }
    }
}
