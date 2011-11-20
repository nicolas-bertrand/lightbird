function Player(id)
{
	this.window = gl_windows[id];
	if (this.window == undefined)
		return ;
	this.window.setObject(this);
	this.window.setOverflow("hidden");
	this.window.setWidth(800);
	this.window.setHeight(400);
	this.window.setTop(100);
	this.window.setLeft(200);
	var className = getElementsByClassName;
	this.node = className("player", this.window.getContentNode());
	this.main = className("main", this.node);
	this.controls = className("controls", this.node);
	this.content = className("content", this.main);
	this.playlistNode = className("playlist", this.node);
	this.list = className("list", this.playlistNode);
	this.table = this.list.getElementsByTagName("table")[0];
	this.drag = className("drag", this.playlistNode);
	this.playlist = new Array();
	this.playlistWidth = C.Player.playlistWidth;
	// The padding bottom of the content (used for the documents)
	this.contentHeightPadding = 0;
	var player = this;
	// Some controls
	var previous = getElementsByClassName("previous", this.controls);
	var play = getElementsByClassName("play", this.controls);
	var next = getElementsByClassName("next", this.controls);
	var fullscreen = getElementsByClassName("fullscreen", this.controls);
	var random = getElementsByClassName("random", this.controls);
	var repeat = getElementsByClassName("repeat", this.controls);
	var quality = getElementsByClassName("quality", this.controls);
	var volume = getElementsByClassName("volume", this.controls);
	
	// Manage the size of the explorer
	this.onResizeListenner = function()
	{
		var paddingBottom = 0;
		if (getClassName(player.main, "document"))
			paddingBottom = player.contentHeightPadding;
		var playlistWidth = player.window.getWidth() - C.Window.borderLeft - C.Window.borderRight;
		if (player.playlistWidth < playlistWidth)
			playlistWidth = player.playlistWidth;
		else
			playlistWidth -= 1;
		player.node.style.height = player.window.getHeight() + "px";
		player.main.style.width = player.window.getWidth() - C.Window.borderLeft - C.Window.borderRight - playlistWidth + "px";
		player.content.style.height = player.window.getHeight() - paddingBottom + "px";
		player.controls.style.width = player.main.style.width;
		player.playlistNode.style.width = positive(playlistWidth) + "px";
		player.list.style.width = positive(playlistWidth - C.Player.dragWidth) + "px";
		if (player.onResize != undefined)
			player.onResize();
		player.onTimeUpdate();
	}
	this.window.setOnResizeListenner(this.onResizeListenner);
	this.onResizeListenner();
	
	// Manage the drag bar
	this.drag.style.width = C.Player.dragWidth + "px";
	var drag = new Object();
	var onMove = function (event)
	{
		player.playlistWidth = drag.width + (drag.mouse - mouseCoordinates(event).x);
		var max = player.window.getWidth() - C.Window.borderLeft - C.Window.borderRight - 1;
		if (player.playlistWidth < C.Player.dragWidth)
			player.playlistWidth = C.Player.dragWidth;
		else if (player.playlistWidth > max)
			player.playlistWidth = max;
		// Manage the display of the playlist
		if (player.playlistWidth < C.Player.playlistNoText)
		{
			setClassName(player.table, "notext");
			removeClassName(player.table, "text");
		}
		else if (player.playlistWidth < C.Player.playlistText)
		{
			setClassName(player.table, "text");
			removeClassName(player.table, "notext");
		}
		else
		{
			removeClassName(player.table, "text");
			removeClassName(player.table, "notext");
		}
		player.onResizeListenner();
	};
	var onMouseUp = function (event)
	{
		removeEvent(document.getElementsByTagName("body")[0], "mousemove", onMove);
		removeEvent(document.getElementsByTagName("body")[0], "mouseup", onMouseUp);
		removeClassName(player.drag, "move");
		disableSelection(true);
	};
	addEvent(this.drag, "mousedown", function (event)
	{
		drag.width = player.playlistWidth;
		drag.mouse = mouseCoordinates(event).x;
		addEvent(document.getElementsByTagName("body")[0], "mousemove", onMove);
		addEvent(document.getElementsByTagName("body")[0], "mouseup", onMouseUp);
		setClassName(player.drag, "move");
		disableSelection(false);
	});
	
	// Display / Hide the player controls
	var displayControls = function()
	{
		animation(player.controls, 250, animationOpacity, true);
		player.contentHeightPadding = C.Player.controlsHeight;
		player.onResizeListenner();
	};
	var hideControls = function()
	{
		animation(player.controls, 250, animationOpacity, false, function ()
		{
			player.contentHeightPadding = 0;
			player.onResizeListenner();
		}, 300);
	};
	addEvent(this.main, "mouseover", displayControls);
	addEvent(this.main, "mouseout", hideControls);
	addEvent(this.controls, "mouseover", displayControls);
	addEvent(this.controls, "mouseout", hideControls);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Controls //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	// Add the events on the controls
	addEvent(getElementsByClassName("previous", this.controls), "click", this.onClickControl);
	addEvent(getElementsByClassName("play", this.controls), "click", this.onClickControl);
	addEvent(getElementsByClassName("next", this.controls), "click", this.onClickControl);
	addEvent(getElementsByClassName("random", this.controls), "click", this.onClickControl);
	addEvent(getElementsByClassName("volume", this.controls), "click", this.onClickControl);
	
	// Bind the previous controls
	this.previousEvent = function ()
	{
		// Search the selected file
		for (i in player.table.rows)
			if (getClassName(player.table.rows[i], "selected"))
			{
				// If random is activated, a random file is selected
				if (getClassName(random, "selected") && player.table.rows.length > 1
					&& ((j = Math.floor(Math.random() * player.table.rows.length)) != i))
					i = j + 1;
				// Play the previous file
				if (i == 0)
					i = player.table.rows.length;
				player.play(player.table.rows[i - 1]);
				return ;
			}
	}
	// Bind the next controls
	this.nextEvent = function ()
	{
		// Search the selected file
		for (i in player.table.rows)
			if (getClassName(player.table.rows[i], "selected"))
			{
				// If random is activated, a random file is selected
				if (getClassName(random, "selected") && player.table.rows.length > 1
					&& ((j = Math.floor(Math.random() * player.table.rows.length)) != i))
					i = j - 1;
				// Play the next file
				if (i == player.table.rows.length - 1)
					i = -1;
				player.play(player.table.rows[parseInt(i) + 1]);
				return ;
			}
	}
	// Bind the random button
	this.randomEvent = function ()
	{
		if (getClassName(random, "selected"))
			removeClassName(random, "selected");
		else
			setClassName(random, "selected");
	}
	// Bind the volume button
	this.volumeEvent = function ()
	{
		if (!player.currentPlayer)
			return ;
		if (getClassName(volume, "muted"))
		{
			removeClassName(volume, "muted");
			player.currentPlayer.muted = false;
		}
		else
		{
			setClassName(volume, "muted");
			player.currentPlayer.muted = true;
		}
	}
	// Bind the seek bar. Seek to the position where the user has click.
	var seek_middle = getElementsByClassName("middle", this.controls);
	addEvent(seek_middle, "click", function (e)
	{
		if (!player.currentPlayer)
			return ;
		var mouse = mouseCoordinates(e);
		var min = player.window.getLeft() + C.Window.borderLeft + C.Player.controlsWidthLeft + C.Player.controlSeekBordersWidth;
		var mainWidth = parseInt(player.main.style.width);
		if (mainWidth < C.Player.playerMinWidth)
			mainWidth = C.Player.playerMinWidth;
		var max = min + (mainWidth - C.Player.controlsWidth);
		if (mouse.x < min)
			mouse.x = min;
		else if (mouse.x > max)
			mouse.x = max;
		//player.currentPlayer.currentTime = ((mouse.x - min) * player.file.duration) / (max - min);
		player.seek = parseInt(((mouse.x - min) * player.file.duration) / (max - min));
		player.play(player.row);
	});
	// Bind the quality button
	this.quality = 70;
	this.seek = 0;
	var player_quality = className("player_quality", this.node);
	addEvent(quality, "mouseover", function (e)
	{
		player_quality = getElementsByClassName("player_quality", this.node);
		var mainWidth = parseInt(player.main.style.width);
		if (mainWidth < C.Player.playerMinWidth)
			mainWidth = C.Player.playerMinWidth;
		player_quality.style.left = mainWidth - C.Player.qualityRightOffset + "px";
		player_quality.style.bottom = C.Player.qualityBottomOffset + "px";
		animation(player_quality, 200, animationOpacity, true);
		var quality = (90 * player.quality) / 100 + 5;
		className("quality_empty", player_quality).style.height = 100 - quality + "%";
		className("quality_full", player_quality).style.height = quality + "%";
	});
	addEvent(quality, "mouseout", function (e)
	{
		clearTimeout(player.qualityTimeout);
		player.qualityTimeout = setTimeout("getElementsByClassName(\"player_quality\", document.getElementById(\"" + player.window.getId() + "\")).style.display=\"none\";", 250);
	});
	// Manage the quality
	this.lockquality = true;
	addEvent(player_quality, "mouseover", displayControls);
	addEvent(player_quality, "mouseout", hideControls);
	addEvent(player_quality, "mouseout", function ()
	{
		player_quality.style.display = "none";
	});
	addEvent(player_quality, "mouseover", function ()
	{
		clearTimeout(player.qualityTimeout);
		player_quality.style.display = "block";
	});
	addEvent(player_quality, "mousemove", function (e)
	{
		var mouse = mouseCoordinates(e);
		var pos = player.window.getTop() + player.window.getHeight() + C.Window.borderTop - C.Player.qualityBottomOffset - 5;
		pos = pos - mouse.y;
		if (pos < 0)
			pos = 0;
		else if (pos > 90)
			pos = 90;
		pos += 5;
		className("quality_empty", player_quality).style.height = 100 - pos + "%";
		className("quality_full", player_quality).style.height = pos + "%";
	});
	addEvent(player_quality, "click", function (e)
	{
		var mouse = mouseCoordinates(e);
		var pos = player.window.getTop() + player.window.getHeight() + C.Window.borderTop - C.Player.qualityBottomOffset - 5;
		pos = pos - mouse.y;
		if (pos < 0)
			pos = 0;
		else if (pos > 90)
			pos = 90;
		player.quality = parseInt(100 * pos / 90);
		animation(player_quality, 200, animationOpacity, false);
		if (player.currentPlayer)
		{
			player.currentPlayer.pause();
			player.seek += parseInt(player.currentPlayer.currentTime);
		}
		player.play(player.row);
	});
	
	if (this.window.getArgument())
		this.setFile(this.window.getArgument());
}

// Change the file that is beeing played, and add it to the playlist
Player.prototype.setFile = function(file, focus)
{
	this.playlist.push(file);
	this.table.insertRow(0);
	var row = this.table.rows[0];
	row.file = file;
	this.updateRowClassName();
	// When the user click on a file in the playlist
	var player = this;
	addEvent(row, "click", function (event)
	{
		if (getEventTarget(event).className == "remove")
			return ;
		if (!(id = getWindowIdByType("Information")))
			openWindow("Informations", "Information", event, getEventTarget(event, "tr").file, false);
		else
			gl_windows[id].getObject().setFile(getEventTarget(event, "tr").file, false);
		player.play(getEventTarget(event, "tr"));
	});
	
	// Display the preview
	row.insertCell(0);
	row.cells[0].className = "image";
	var content = "<img src=\"/Client/Execute/Preview?id=" + file.id + "&amp;width=300\" onload=\"this.parentNode.removeChild(this.parentNode.getElementsByTagName('img')[1]); this.className='visible'\" onerror=\"this.parentNode.removeChild(this)\" />";
	content += "<img src=\"/Client/images/icons/" + file.type + ".png\" class=\"visible\">";
	row.cells[0].innerHTML = content;
	// If the preview failed to load, a default image if displayed
	addEvent(row.cells[0].getElementsByTagName("img")[0], "error", function(event)
	{
		var tr = getEventTarget(event, "tr");
		if (tr != undefined)
			getEventTarget(event).src = "/Client/images/icons/" + getEventTarget(event, "tr").file.type + ".png";
	});
	
	// Add some information
	row.insertCell(1);
	row.cells[1].className = "data";
	var content;
	content = "<div class=\"remove\"></div>";
	if (file.title)
		content += "<b>" + file.title + "</b>";
	else
		content += "<b>" + file.name + "</b>";
	if (file.duration)
		content += "<br /><i>" + informationParseDuration(file.duration) + "</i>";
	if (file.artist)
		content += "<br />" + file.artist;
	row.cells[1].innerHTML = content;
	addEvent(row.cells[1].getElementsByTagName("div")[0], "click", function (event)
	{
		getEventTarget(event, "table").deleteRow(getEventTarget(event, "tr").rowIndex);
		player.updateRowClassName();
	});
	if (focus != false)
		this.window.isFocus(true);
		
	// Display the file in the player
	this.play(row);
};

// Play the file in the player
Player.prototype.play = function(row)
{
	var player = this;
	this.file = row.file;
	this.row = row;
	var file = row.file;
	var previous = getElementsByClassName("previous", this.controls);
	var play = getElementsByClassName("play", this.controls);
	var next = getElementsByClassName("next", this.controls);
	var fullscreen = getElementsByClassName("fullscreen", this.controls);
	var random = getElementsByClassName("random", this.controls);
	var repeat = getElementsByClassName("repeat", this.controls);
	var quality = getElementsByClassName("quality", this.controls);
	var volume = getElementsByClassName("volume", this.controls);
	
	// Set the title of the window
	if (file.title)
		this.window.setTitle(file.title);
	else
		this.window.setTitle(file.name);
	
	// Select the image
	for (var i = 0, s = this.table.rows.length; i < s; ++i)
		removeClassName(this.table.rows[i], "selected");
	setClassName(row, "selected");
	
	// Stop the previous stream
	if (this.streamId)
		request("GET", "Execute/StopStream?streamId=" + this.streamId);
	
	// Initialize the content of the player
	removeClassName(this.content, "image");
	removeClassName(this.content, "video");
	removeClassName(this.content, "audio");
	removeClassName(this.content, "document");
	removeClassName(this.content, "other");
	removeClassName(this.controls, "display");
	removeClassName(play, "pause");
	this.removePlayer();
	this.content.innerHTML = "";
	removeClassName(this.main, "document");
	if (this.HttpRequest && this.HttpRequest.readyState != 4)
		this.HttpRequest.abort();
	this.HttpRequest = null;
	this.streamId = null;
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// IMAGES //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	if (file.type == "image")
	{
		// Hide the unused controls
		setClassName(this.controls, "display");
		// Put the image in the content
		this.content.innerHTML = "<img src='/Client/" + file.name + "?id=" + file.id + "' />";
		// Resize the image so that it will be entirely visible at every moment
		var image = this.content.getElementsByTagName("img")[0];
		image.style.maxWidth = file.width + "px";
		image.style.maxHeight = file.height + "px";
		image.style.width = "100%";
		image.style.height = "auto";
		this.onResize = function ()
		{
			if (image.offsetWidth == undefined || image.offsetWidth <= 0)
				return ;
			if ((image.style.height == "auto" && image.offsetHeight > player.window.getHeight()))
			{
				image.style.height = player.window.getHeight() + 1 + "px";
				image.style.width = "auto";
			}
			if (image.style.height != "auto")
			{
				var widthMax = parseInt(player.main.style.width);
				if (image.offsetWidth >= widthMax)
				{
					image.style.width = "100%";
					image.style.height = "auto";
					image.style.maxWidth = file.width + "px";
				}
				else
				{
					image.style.height = player.window.getHeight() + 1 + "px";
					if (file.width > widthMax)
						image.style.maxWidth = widthMax + "px";
					else
						image.style.maxWidth = file.width + "px";
				}
			}
		}
		addEvent(image, "load", this.onResize);
		// The browser is unable to display this image
		addEvent(image, "error", function () { player.content.innerHTML = ""; setClassName(player.content, "image"); });
	}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// AUDIOS //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	else if (file.type == "audio")
	{
		this.streamId = Math.random();
		this.content.innerHTML = "<audio autobuffer>\
		  <source src=\"/Client/Execute/Audio.mp3?id=" + file.id + "&amp;streamId=" + this.streamId + "&amp;quality=" + this.quality + "&amp;seek=" + this.seek + "\" type=\"audio/mpeg\" />\
		  <source src=\"/Client/Execute/Audio.ogg?id=" + file.id + "&amp;streamId=" + this.streamId + "&amp;quality=" + this.quality + "&amp;seek=" + this.seek + "\" type=\"audio/ogg\" />\
		</audio>";
		this.currentPlayer = this.content.getElementsByTagName("audio")[0];
		setClassName(this.content, "audio");
		this.currentPlayer.play();
		this.playEvent = function ()
		{
			if (getClassName(play, "pause"))
			{
				removeClassName(play, "pause");
				player.currentPlayer.play();
			}
			else
			{
				setClassName(play, "pause");
				player.currentPlayer.pause();
			}
		}
		addEvent(this.currentPlayer, "timeupdate", this.onTimeUpdate);
	}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// VIDEOS //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	else if (file.type == "video")
	{
		this.streamId = Math.random();
		this.content.innerHTML = "<video autobuffer>\
			<source src=\"/Client/Execute/Video.webm?id=" + file.id + "&amp;streamId=" + this.streamId + "&amp;quality=" + this.quality + "&amp;seek=" + this.seek + "\" type='video/webm; codecs=\"vp8, vorbis\"'>\
			<source src=\"/Client/Execute/Video.ogv?id=" + file.id + "&amp;streamId=" + this.streamId + "&amp;quality=" + this.quality + "&amp;seek=" + this.seek + "\"  type='video/ogg; codecs=\"theora, vorbis\"'>\
		</video>";
		//<source src=\"/Client/Execute/Video.mp4?id=" + file.id + "&amp;streamId=" + this.streamId + "\"  type='video/mp4; codecs=\"avc1.42E01E, mp4a.40.2\"'>\
		this.currentPlayer = this.content.getElementsByTagName("video")[0];
		setClassName(this.content, "video");
		this.currentPlayer.play();
		this.playEvent = function ()
		{
			if (getClassName(play, "pause"))
			{
				removeClassName(play, "pause");
				player.currentPlayer.play();
			}
			else
			{
				setClassName(play, "pause");
				player.currentPlayer.pause();
			}
		}
		addEvent(this.currentPlayer, "timeupdate", this.onTimeUpdate);
		this.currentPlayer.poster = "/Client/Execute/Preview?id=" + row.file.id + "&width=300";
		
		// When the user move over the seek bar of a video, a preview image is displayed above the cursor
		var player_preview = getElementsByClassName("player_preview", this.node);
		seek_middle = getElementsByClassName("seek_middle", this.node);
		addEvent(seek_middle, "mousemove", function (e)
		{
			if (player.file.type != "video")
				return ;
			var mouse = mouseCoordinates(e);
			var min = player.window.getLeft() + C.Window.borderLeft + C.Player.controlsWidthLeft + C.Player.controlSeekBordersWidth;
			var mainWidth = parseInt(player.main.style.width);
			if (mainWidth < C.Player.playerMinWidth)
				mainWidth = C.Player.playerMinWidth;
			var max = min + (mainWidth - C.Player.controlsWidth);
			if (mouse.x < min || mouse.x > max)
				return ;
			var time = ((mouse.x - min) * file.duration) / (max - min);
			player_preview.style.left = mouse.x - player.window.getLeft() - C.Window.borderLeft - C.Player.previewWidth / 2 + "px";
			var position = Math.floor(file.duration / C.Player.previews);
			position = Math.floor(time / position) * position;
			position += Math.floor((file.duration / C.Player.previews) / 2);
			if (player_preview.position != position)
			{
				player_preview.position = position;
				player_preview.innerHTML = "<img src=\"/Client/Execute/Preview?id=" + row.file.id + "&width=" + C.Player.previewWidth + "&position=" + position + "\" />";
				var preview = player_preview.getElementsByTagName("img")[0];
				addEvent(preview, "load", function ()
				{
					player_preview.style.top = parseInt(player.node.style.height) - ((C.Player.previewWidth * file.height) / file.width) - C.Player.previewHeight + "px";
					preview.style.visibility = "visible";
				});
			}
		});
		addEvent(seek_middle, "mouseover", function () { if (player.file.type == "video") animation(player_preview, 250, animationOpacity, true); });
		addEvent(seek_middle, "mouseout", function () { animation(player_preview, 250, animationOpacity, false); });
	}
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// DOCUMENTS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	else if (file.type == "document")
	{
		// Hide the unused controls
		setClassName(this.controls, "display");
		setClassName(this.content, "document");
		setClassName(this.main, "document");
		// Check if the document can be displayed
		for (var key in PlayerDocumentExtensionSHJS)
		{
			var regexp = new RegExp(key + "$", "i");
			if (file.name.match(regexp))
			{
				// Get the content of the document
				this.HttpRequest = request("GET", file.name + "?id=" + file.id, function(HttpRequest)
				{
					// The document is hightlighted if possible
					player.content.innerHTML = "<pre class=\"" + PlayerDocumentExtensionSHJS[key] + "\">" + HttpRequest.responseText.replace(/</g, "&lt;") + "</pre>";
					sh_highlightDocument("Client/libs/SHJS/", ".js");
				});
				removeClassName(player.content, "document");
				break ;
			}
		}
	}
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// OTHERS /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	// An icon is displayed for the other files
	else if (file.type == "other")
	{
		// Hide the unused controls
		setClassName(this.controls, "display");
		setClassName(this.content, "other");
	}
	this.onResizeListenner();
}

// Call the callbacks methods of the controls buttons of the player
Player.prototype.onClickControl = function (event)
{
	var node = getEventTarget(event);
	var player = gl_windows[getWindowId(node)].getObject();
	if (getClassName(node, "previous"))
		((player.previousEvent != undefined) ? (player.previousEvent()) : (false));
	else if (getClassName(node, "play"))
		((player.playEvent != undefined) ? (player.playEvent()) : (false));
	else if (getClassName(node, "next"))
		((player.nextEvent != undefined) ? (player.nextEvent()) : (false));
	else if (getClassName(node, "random"))
		((player.randomEvent != undefined) ? (player.randomEvent()) : (false));
	else if (getClassName(node, "volume"))
		((player.volumeEvent != undefined) ? (player.volumeEvent()) : (false));
}

// Update the odd / even class
Player.prototype.updateRowClassName = function ()
{
	for (i in this.table.rows)
		if (this.table.rows[i] != undefined)
		{
			if (i % 2)
			{
				removeClassName(this.table.rows[i], "odd");
				setClassName(this.table.rows[i], "even");
			}
			else
			{
				removeClassName(this.table.rows[i], "even");
				setClassName(this.table.rows[i], "odd");
			}
		}
}

Player.prototype.removePlayer = function ()
{
	if (this.currentPlayer)
	{
		this.currentPlayer.pause();
		this.currentPlayer.parentNode.removeChild(this.currentPlayer);
		this.currentPlayer = null;
	}
}

// Update the seek bar
Player.prototype.onTimeUpdate = function (event)
{
	var player = this;
	if (event)
	{
		var id = getWindowId(getEventTarget(event));
		if (gl_windows[id])
			player = gl_windows[id].getObject();
	}
	if (!player.currentPlayer || !player.file)
		return ;
	var position = player.currentPlayer.currentTime + player.seek;
	var duration = parseInt(player.file.duration);
	if (position > duration)
		position = duration;
	getElementsByClassName("seek_position", player.controls).innerHTML = player.durationToText(position);
	getElementsByClassName("seek_remaining", player.controls).innerHTML = player.durationToText(duration - position);
	var mainWidth = parseInt(player.main.style.width);
	if (mainWidth < C.Player.playerMinWidth)
		mainWidth = C.Player.playerMinWidth;
	var left = (position / duration) * (mainWidth - C.Player.controlsWidth) - C.Player.controlSeekBordersWidth;
	if (left > (mainWidth - C.Player.controlsWidth - C.Player.controlSeekBordersWidth))
		left = (mainWidth - C.Player.controlsWidth - C.Player.controlSeekBordersWidth);
	getElementsByClassName("seeker", player.controls).style.left = left + "px";
	getElementsByClassName("seek_read_middle", player.controls).style.width = left + C.Player.controlSeekBordersWidth + "px";
}

// Called when the window is classed
Player.prototype.onClose = function ()
{
	this.removePlayer();
	// Stop the streaming
	if (this.streamId)
		request("GET", "Execute/StopStream?streamId=" + this.streamId);
}

Player.prototype.durationToText = function (duration)
{
	duration = informationParseDuration(parseInt(duration)) + "";
	if (!duration || duration == "undefined")
		return ("0:00");
	if (duration.length < 2)
		return ("0:0" + duration);
	else if (duration.length < 3)
		return ("0:" + duration);
	return (duration);
}

var PlayerDocumentExtensionSHJS = {
	".c" : "sh_c",
	".h" : "sh_cpp",
	".cpp" : "sh_cpp",
	".cc" : "sh_cpp",
	".hpp" : "sh_cpp",
	".caml" : "sh_caml",
	".cs" : "sh_csharp",
	".css" : "sh_css",
	".diff" : "sh_diff",
	".html" : "sh_html",
	".java" : "sh_java",
	".js" : "sh_javascript",
	".json" : "sh_javascript",
	".latex" : "sh_latex",
	".ldap" : "sh_ldap",
	".log" : "sh_log",
	"makefile" : "sh_makefile",
	".pas" : "sh_pascal",
	".pl" : "sh_perl",
	".pm" : "sh_perl",
	".php" : "sh_php",
	".php3" : "sh_php",
	".php4" : "sh_php",
	".py" : "sh_python",
	".pyw" : "sh_python",
	".pyc" : "sh_python",
	".pyo" : "sh_python",
	".pyd" : "sh_python",
	".rpm" : "sh_spec",
	".rb" : "sh_ruby",
	".sh" : "sh_sh",
	".sql" : "sh_sql",
	".sml" : "sh_sml",
	".tcl" : "sh_tcl",
	".xml" : "sh_xml",
	".xorg" : "sh_xorg",
	".txt" : "",
	".vb" : ""
};

gl_resources.jsLoaded("Player");
function initializePlayer(id) {	new Player(id); }
