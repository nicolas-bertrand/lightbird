/* Handle the context menu, which is accessible only for idenfified users. */

var contextMenuActivate = true;

// Called when the mouse is pressed, and display/hide the context menu according to the event.
function contextMenu(event)
{
	return ;
	var context = document.getElementById("context");

	// If the user is not identified, the context menu is not displayed
	if (!gl_identified || !contextMenuActivate)
		return ;
	// Display the context menu where the user had clicked (right click)
	if (event.button == 2)
	{
		context.style.left = event.clientX + 5 + "px";
		context.style.top = event.clientY + 5 + "px";
		if (context.style.display != "block")
		{
			context.style.display = "block";
			animation(context, 500, animationOpacity, true);
		}
	}
	// Hide the context menu (left click)
	else if (context.style.display == "block")
		animation(context, 250, animationOpacity, false, function() {context.style.display = "none"});
}

function disactivateContextMenu(event)
{
	contextMenu(event);
	contextMenuActivate = false;
	document.oncontextmenu = function() { return (true); };
}
