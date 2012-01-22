/* Disable or enable the text selection of the body */
  
function disableSelection(enable, element)
{
	var done = false;

	if (element == undefined)
		element = document.body;
	if (typeof(element.onselectstart) != "undefined")
	{
		element.onselectstart = function() { return (enable); };
		done = true;
	}
	if (typeof(element.style.userSelect) != "undefined")
	{
		if (enable)
			element.style.userSelect = "inherit";
		else
			element.style.userSelect = "none";
		done = true;
	}
	if (typeof(element.style.MozUserSelect) != "undefined")
	{
		if (enable)
			element.style.MozUserSelect = "inherit";
		else
			element.style.MozUserSelect = "none";
		done = true;
	}
	if (typeof(element.style.WebkitUserSelect) != "undefined")
	{
		if (enable)
			element.style.WebkitUserSelect = "inherit";
		else
			element.style.WebkitUserSelect = "none";
		done = true;
	}
    element.onmousedown = function(event) { contextMenu(event); return (enable); };
}
