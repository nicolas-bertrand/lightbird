/* Manage all the animations */

var gl_animations = new Array(); // Contains all the informations on the animations currently played.
var gl_animationsNumber = 0;	 // Count the number of animations that are currently running. When it reach 0, all the animations are deleted of gl_animations.

// Allows to use an animation on a node.
// @node : The node to which the effet will be applied.
// @duration : The duration of the animation, in milliseconds.
// @endCallback : If defined, this method will be called at the end of the animation.
// @animationCallback : This method is called for each frame of the animation. If false is returned,
// the animation is stopped and the endcallback is called.
// @parameter : This parameter will be gived to the animationCallback.
// @delay : The time before the start of the animation, in milliseconds.
// @fps : The number of image per second (20 by default).
function animation(node, duration, animationCallback, parameter, endCallback, delay, fps)
{
	// Set the default value of the fps
	if (fps == null || fps < 1)
		fps = C.animationFPS;
	// Save the data on the animation
	var i = gl_animations.length;
	var anim = new Object();
	anim.node = node;
	anim.duration = duration;
	anim.remaining = duration;
	anim.fps = fps;
	anim.endCallback = endCallback;
	anim.animationCallback = animationCallback;
	anim.parameter = parameter;
	anim.percentage = 0;
	anim.ms = duration / (fps * duration / 1000);
	anim.run = true;
	gl_animationsNumber++;
	gl_animations.push(anim);
	// Initialize the animation
	if (anim.animationCallback != undefined)
		// If false is returned, the animation is stopped
		if (!anim.animationCallback(i, anim.parameter))
			return animationRemove(i);
	// Launch the animation
	if (delay == null)
		delay = anim.ms;
	anim.timeout = setTimeout("animationDo(" + i + ")", delay);
	//alert("node = " + anim.node + "\nduration = " + anim.duration + "\nremaining = " + anim.remaining + "\nfps = " + anim.fps + "\nendCallback = " + anim.endCallback + "\nanimationCallback = " + anim.animationCallback + "\npercentage = " + anim.percentage + "\nms = " + anim.ms + "\ntimeout = " + anim.timeout + "\ndelay = " + delay + "\nparameter = " + anim.parameter);
}

function animationDo(i)
{
	clearTimeout(gl_animations[i].timeout);
	// Update the state of the animation
	gl_animations[i].remaining -= gl_animations[i].ms;
	gl_animations[i].percentage = 100 - (100 * gl_animations[i].remaining) / gl_animations[i].duration;
	//alert("node = " + gl_animations[i].node + "\nduration = " + gl_animations[i].duration + "\nremaining = " + gl_animations[i].remaining + "\nfps = " + gl_animations[i].fps + "\nendCallback = " + gl_animations[i].endCallback + "\nanimationCallback = " + gl_animations[i].animationCallback + "\npercentage = " + gl_animations[i].percentage + "\nms = " + gl_animations[i].ms + "\ntimeout = " + gl_animations[i].timeout);
	// The animation is not finished
	if (gl_animations[i].remaining > 0)
	{
		gl_animations[i].timeout = setTimeout("animationDo(" + i + ")", gl_animations[i].ms);
		// Calls the animation callback
		if (gl_animations[i].animationCallback != undefined)
			// If false is returned, the animation is stopped
			if (!gl_animations[i].animationCallback(i, gl_animations[i].parameter))
				animationRemove(i);
	}
	else
	{
		gl_animations[i].percentage = 100;
		// Call the animation callback
		if (gl_animations[i].animationCallback != undefined)
			gl_animations[i].animationCallback(i, gl_animations[i].parameter);
		animationRemove(i);
	}
}

// Remove an animation, and clean the animation array is necessary.
function animationRemove(i)
{
	gl_animations[i].run = false;
	// Calls the end callback
	if (gl_animations[i].endCallback != undefined)
		gl_animations[i].endCallback(gl_animations[i].node);
	// Decrements the animation counter
	gl_animationsNumber--;
	// Removes all the animations if gl_animationsNumber reach 0
	if (gl_animationsNumber <= 0)
		gl_animations.splice(0, gl_animations.length);
}

// Fade in or out the node using its opacity.
// @param appear : If true, the node will appear.
function animationOpacity(i, appear)
{
	// Check if an animation is already running on this node
	var j;
	if (gl_animations[i].percentage == 0)
		for (j = 0; j < gl_animations.length; ++j)
			if (gl_animations[i].node == gl_animations[j].node && j != i && gl_animations[j].run && appear != gl_animations[j].parameter)
			{
				// Update the existing animation and destroy the new one
				gl_animations[j].remaining = (gl_animations[j].percentage * gl_animations[i].duration) / 100;
				gl_animations[j].percentage = 100 - gl_animations[j].percentage;
				gl_animations[j].parameter = appear;
				gl_animations[j].duration = gl_animations[i].duration;
				gl_animations[j].ms = gl_animations[i].ms;
				gl_animations[j].fps = gl_animations[i].fps;
				gl_animations[j].endCallback = gl_animations[i].endCallback;
				gl_animations[i].endCallback = undefined;
				return (false);
			}
	// Display the node
	if (gl_animations[i].percentage == 0 && appear)
		gl_animations[i].node.style.display = "block";
	// Hide the node
	if (gl_animations[i].percentage == 100 && !appear)
		gl_animations[i].node.style.display = "none";
	// If the opacity is not by the browser, the animation is stopped
	if (gl_animations[i].node.style.opacity == undefined)
	{
		if (!appear)
			gl_animations[i].node.style.display = "none";
		return (false);
	}
	// Fade in
	if (appear)
		changeOpacity(gl_animations[i].node, gl_animations[i].percentage / 100);
	// Fade out
	else
		changeOpacity(gl_animations[i].node, (100 - gl_animations[i].percentage) / 100);
	return (true);
}

// Move the node to the given position.
// @param position : The relative position, where the node has to be moved.
function animationMove(i, position)
{
	// Save the original position of the node
	if (gl_animations[i].percentage == 0)
	{
		gl_animations[i].x = toNumber(gl_animations[i].node.style.left);
		gl_animations[i].y = toNumber(gl_animations[i].node.style.top);
	}
	// Put the node at its final position
	if (gl_animations[i].percentage == 100)
	{
		gl_animations[i].node.style.left = (gl_animations[i].x + position.x) + "px";
		gl_animations[i].node.style.top = (gl_animations[i].y + position.y) + "px";
		return (true);
	}
	// Compute the current position
	gl_animations[i].node.style.left = ((gl_animations[i].x + (position.x * gl_animations[i].percentage) / 100) + "px");
	gl_animations[i].node.style.top = ((gl_animations[i].y + (position.y * gl_animations[i].percentage) / 100) + "px");
	return (true);
}

// Change the size of the node.
// @param size : The new size of the element.
function animationSize(i, size)
{
	// Save the original size of the node
	if (gl_animations[i].percentage == 0)
	{
		if (size.w != undefined)
			gl_animations[i].w = toNumber(gl_animations[i].node.style.width);
		if (size.h != undefined)
			gl_animations[i].h = toNumber(gl_animations[i].node.style.height);
	}
	// Put the node at its final size
	if (gl_animations[i].percentage == 100)
	{
		if (size.w != undefined)
			gl_animations[i].node.style.width = size.w + "px";
		if (size.h != undefined)
			gl_animations[i].node.style.height = size.h + "px";
		return (true);
	}
	// Compute the current size
	if (size.w != undefined)
		gl_animations[i].node.style.width = ((gl_animations[i].w + ((size.w - gl_animations[i].w) * (Math.pow(gl_animations[i].percentage / 100, 2)))) + "px");
	if (size.h != undefined)
		gl_animations[i].node.style.height = ((gl_animations[i].h + ((size.h - gl_animations[i].h) * (Math.pow(gl_animations[i].percentage / 100, 2)))) + "px");
	return (true);
}
