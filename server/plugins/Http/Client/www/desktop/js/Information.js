function Information(id)
{
	var window = gl_windows[id];
	if (window == undefined)
		return ;
	window.setObject(this);
	window.setTop(- C.Window.shadowTop);
	window.setLeft(gl_browserSize.width - 354 - C.Window.borderRight);
	window.setWidth(400);
	window.setHeight(gl_browserSize.height - C.Window.shadowTop - C.Window.borderBottom);
	var object = this;
	var className = getElementsByClassName;
	var information = className("information", window.getContentNode());
	var table = information.getElementsByTagName("table")[0];
	var menu = className("menu", information);
	var detailsTable = information.getElementsByTagName("table")[1];
	var details = className("details", information);
	var file = window.getArgument();
	var displayed = new Object();
	
	var func_informationUpdateImage;
	var func_informationDeleteFile;
	
	// Dispay the data on the file
	function display()
	{
		// Insert the name of the file
		table.insertRow(0);
		var row = table.rows[0];
		row.className = "even";
		row.insertCell(0);
		row.cells[0].colSpan = 2;
		var name = file.name;
		if (file.title)
			name = file.title;
		row.cells[0].innerHTML = "<a target=\"_blank\" href=\"/Client/" + file.name + "?id=" + file.id + "\">" + name + "</a>";
		row.cells[0].className = "name";
		
		// Add th preview
		table.insertRow(1);
		row = table.rows[1];
		row.className = "odd";
		setClassName(row, "preview");
		row.insertCell(0);
		row.cells[0].colSpan = 2;
		row.cells[0].className = "preview";
		var buttons = "<img src=\"/Client/Execute/Preview?id=" + file.id + "&amp;width=300\" width=\"300\" />";
		buttons += "<div class=\"menu\">" + menu.innerHTML + "</div>";
		row.cells[0].innerHTML = buttons;
		row.cells[0].getElementsByTagName("a")[0].href = "/Client/" + file.name + "?id=" + file.id + "&download=true";
		addEvent(row.cells[0].getElementsByTagName("img")[0], "load", function (e) {func_informationUpdateImage(getEventTarget(e))});
		addEvent(className("play", row.cells[0]), "click", function(event)
		{
			if (!(id = getWindowIdByType("Player")))
				openWindow("Lecteur", "Player", event, file);
			else
				gl_windows[id].getObject().setFile(file);
		});
		addEvent(className("remove", row.cells[0]), "click", function() { func_informationDeleteFile(); });
		
		// Add the control buttons
		func_informationUpdateImage = function (image)
		{
			var width = image.offsetWidth;
			var height = image.offsetHeight;
			if (height > C.Information.previewMaxHeight)
				height = C.Information.previewMaxHeight;
			var src = image.src;
			var parent = image.parentNode;
			removeClassName(parent.parentNode, "preview");
			parent.innerHTML = "<div>" + className("preview_controls", information).innerHTML + "</div>";
			var div = parent.getElementsByTagName("div")[0];
			div.style.backgroundImage = "url(\"" + src + "\")";
			div.style.width = width + "px";
			div.style.height = height + "px";
			div.className = "image";
			var view = className("view", div);
			view.style.height = height - C.Information.buttonsHeight + "px";
			var buttons = className("buttons", div);
			addEvent(div, "mouseover", function() { animation(buttons, 250, animationOpacity, true); });
			addEvent(div, "mouseout", function() { animation(buttons, 250, animationOpacity, false); });
			className("download", buttons).href = "/Client/" + file.name + "?id=" + file.id + "&download=true";
			var play = function(event)
			{
				if (!(id = getWindowIdByType("Player")))
					openWindow("Lecteur", "Player", event, file);
				else
					gl_windows[id].getObject().setFile(file);
			}
			addEvent(view, "click", play);
			addEvent(className("play", buttons), "click", play);
			addEvent(className("edit", buttons), "click", function() { alert("edit"); });
			addEvent(className("remove", buttons), "click", function() { func_informationDeleteFile(); });
		}
		
		// Display some important data on the file
		var data = informationGetImportantData(file);
		displayed = new Object();
		displayed["name"] = true;
		for (i in data)
			if ((value = file[data[i].index]))
			{
				if (data[i].value)
					value = data[i].value;
				informationAddRow(table, data[i].name, value);
				displayed[data[i].index] = true;
			}
	}
	display();
	
	// Display the details on the file when the details button is pressed
	detailsFunction = function ()
	{
		if (getClassName(details, "more"))
		{
			removeClassName(details, "more");
			setClassName(details, "less");
			for (key in file)
				if (!displayed[key])
					informationAddRow(detailsTable, key, file[key]);
		}
		else
		{
			removeClassName(details, "less");
			setClassName(details, "more");
			for (var i = 0, s = detailsTable.rows.length; i < s; ++i)
				detailsTable.deleteRow(0);
		}
	}
	addEvent(details, "click", detailsFunction);
	
	// Delete the current file after confirmation
	var idWindow = id;
	func_informationDeleteFile = function ()
	{
		var node = table.rows[1].cells[0];
		// A preview is displayed
		if (!node.getElementsByTagName("img")[0])
			node = className("buttons", node);
		// No preview is displayed
		else
			node = node.getElementsByTagName("div")[0];
		node.getElementsByTagName("div")[0].style.display = "none";
		// Display the confirmation buttons
		className("menu_context", node).innerHTML = "<div class=\"valid_delete\"><div></div></div><div class=\"cancel_delete\"><div></div></div>"
		// Valid
		addEvent(className("valid_delete", node), "click", function()
		{
			request("GET", "Execute/DeleteFile?id=" + file.id, function ()
			{
				var ids = getWindowsIdByType("Explorer");
				for (var i = 0; i < ids.length; ++i)
					gl_windows[ids[i]].getObject().update();
			});
			closeWindow(idWindow);
		});
		// Cancel
		addEvent(className("cancel_delete", node), "click", function()
		{
			node.getElementsByTagName("div")[0].style.display = "block";
			className("menu_context", node).innerHTML = "";
		});
	}
	
	// This method allows to change the content of the window (display the data of another file)
	this.setFile = function(fileData, focus, force)
	{
		// If the file is the same
		if (fileData.id == file.id && force != true)
			return ;
		// Initialise the page
		file = fileData;
		for (var i = 0, s = table.rows.length; i < s; ++i)
			table.deleteRow(0);
		for (var i = 0, s = detailsTable.rows.length; i < s; ++i)
			detailsTable.deleteRow(0);
		if (getClassName(details, "less"))
		{
			removeClassName(details, "less");
			setClassName(details, "more");
		}
		else
		{
			removeClassName(details, "more");
			setClassName(details, "less");
		}
		detailsFunction();
		// Display the new content
		display();
		if (focus != false)
			window.isFocus(true);
	};
}

function informationAddRow(table, key, value)
{
	table.insertRow(table.rows.length);
	row = table.rows[table.rows.length - 1];
	row.insertCell(0);
	row.insertCell(1);
	row.cells[0].innerHTML = "<b>" + key + "</b>";
	row.cells[1].innerHTML = value;
	if (table.rows.length % 2)
		row.className = "even";
	else
		row.className = "odd";
	row.cells[0].style.width = "33%";
}

function informationParseSize(size)
{
	if (!size)
		return ;
	var n = toNumber(size);
	var unit = [ "octets", "Ko", "Mo", "Go", "To" ];

	n = n / Math.pow(1024, parseInt((size.length - 1) / 3));
	unit = unit[parseInt((size.length - 1) / 3)];
	n = (n + "").replace(".", ",").substr(0, 4);
	if (n.indexOf(",") == 3)
		n = n.substr(0, 3);
	return (n + " " + unit);
}

function informationParseDuration(duration)
{
	if (!duration)
		return ;
	var hours = parseInt(duration / (3600));
	var minutes = parseInt((duration - hours * 3600) / 60);
	var seconds = duration - hours * 3600 - minutes * 60;
	if (seconds < 10)
		seconds = "0" + seconds;
	if (minutes < 10 && hours > 0)
		minutes = "0" + minutes;
	if (hours > 0)
		return (hours + ":" + minutes + ":" + seconds);
	else if (minutes > 0)
		return (minutes + ":" + seconds);
	else
		return (seconds);
}

function informationParseType(t)
{
	var type = { "image" : "Image", "audio" : "Audio", "video" : "Vid&eacute;o", "document" : "Document", "" : "Autre" };
	if (type[t])
		return (type[t])
	return (type[""]);
}

function informationGetImportantData(file)
{
	return ([
		{
			"index" : "size",
			"name" : "Taille",
			"value" : informationParseSize(file.size)
		},
		{
			"index" : "duration",
			"name" : "Dur&eacute;e",
			"value" : informationParseDuration(file.duration)
		},
		{
			"index" : "title",
			"name" : "Titre"
		},
		{
			"index" : "artist",
			"name" : "Artiste"
		},
		{
			"index" : "album",
			"name" : "Album"
		},
		{
			"index" : "genre",
			"name" : "Genre"
		},
		{
			"index" : "track",
			"name" : "Piste"
		},
		{
			"index" : "type",
			"name" : "Type de fichier",
			"value" : informationParseType(file.type)
		},
		{
			"index" : "format",
			"name" : "Format"
		},
		{
			"index" : "width",
			"name" : "Largeur",
			"value" : file.width + " pixels"
		},
		{
			"index" : "height",
			"name" : "Hauteur",
			"value" : file.height + " pixels"
		},
	]);
}

gl_resources.jsLoaded("Information");
function initializeInformation(id) { new Information(id); }
