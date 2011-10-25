var gl_searchFormDefaultValue = "Recherche";

function Explorer(id)
{
	///////////////////////////////////////////////////////////////////////////
	////////////////////////////////// NODES //////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	var window = gl_windows[id];
	if (window == undefined)
		return ;
	window.setObject(this);
	window.setOverflow("hidden");
	window.setTop(- C.Window.shadowTop);
	window.setLeft(- C.Window.shadowLeft);
	window.setWidth(gl_browserSize.width - 330);
	window.setHeight(gl_browserSize.height - C.Window.shadowTop - C.Window.borderBottom);
	var className = getElementsByClassName;
	var explorer = className("explorer", window.getContentNode());
	
	// Buttons
	var buttons = className("buttons", explorer);
	var navigations = className("navigation", buttons);
	var navigations_left = className("left", navigations);
	var navigations_middle = className("middle", navigations);
	var navigations_right = className("right", navigations);
	
	var modes = className("modes", buttons);
	var modes_left = className("left", modes);
	var modes_middle = className("middle", modes);
	var modes_middleBis = className("middleBis", modes);
	var modes_right = className("right", modes);
	
	var display = className("display", buttons);
	var display_left = className("left", display);
	var display_right = className("right", display);
	
	var directory = className("directory", buttons);
	
	var search = className("search", buttons);
	var form = className("form", search);
	var advanced = className("advanced", search);
	
	// Headers
	var header = className("header", explorer).getElementsByTagName("table")[0];
	var sortBy = 0;			// The index of the sorted column
	var orderBy = "asc";	// The order of the sort (asc/desc)
	
	// Files
	var body = className("body", explorer);
	var table = body.getElementsByTagName("table")[0];
	var files;
	
	// Types
	var types = className("types", explorer);
	var audios = className("audios", types);
	var videos = className("videos", types);
	var images = className("images", types);
	var documents = className("documents", types);
	var others = className("others", types);
	
	// Manage the size of the explorer
	var e = C.Explorer;
	explorer.style.height = window.getHeight() - e.buttons - e.headers + "px";
	body.style.height = window.getHeight() - e.buttons - e.headers - e.types + "px";
	window.setOnResizeListenner(function()
	{
		var height = window.getHeight() - e.buttons - e.headers;
		if (height < 0)
			height = 0;
		explorer.style.height = height + "px";
		height -= e.types;
		if (height < 0)
			height = 0;
		body.style.height = height + "px";
	});
	
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////// BUTTONS /////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	setClassName(display_left, "on");
	setClassName(modes_left, "on");
	
	// On click events
	addEvent(modes_left, "click", function() { explorerActiveOneButton(modes_left, modes_middle, modes_middleBis, modes_right); });
	addEvent(modes_middle, "click", function() { explorerActiveOneButton(modes_middle, modes_left, modes_middleBis, modes_right); });
	addEvent(modes_middleBis, "click", function() { explorerActiveOneButton(modes_middleBis, modes_middle, modes_left, modes_right); });
	addEvent(modes_right, "click", function() { explorerActiveOneButton(modes_right, modes_middle, modes_middleBis, modes_left); });

	addEvent(display_left, "click", function() { explorerActiveOneButton(display_left, display_right); });
	addEvent(display_right, "click", function() { explorerActiveOneButton(display_right, display_left); });

	addEvent(directory, "click", function() { getClassName(directory, "on") ? removeClassName(directory, "on") : setClassName(directory, "on"); });
	
	addEvent(form, "mouseover", function() { setClassName(form, "hover"); setClassName(advanced, "hover"); });
	addEvent(form, "mouseout", function() { removeClassName(form, "hover"); removeClassName(advanced, "hover"); });
	addEvent(advanced, "mouseover", function() { setClassName(form, "hover"); setClassName(advanced, "hover"); });
	addEvent(advanced, "mouseout", function() { removeClassName(form, "hover"); removeClassName(advanced, "hover"); });
	
	form.style.width = 75 + "px";
	form.value = gl_searchFormDefaultValue;
	addEvent(form, "focus", function()
	{
		setClassName(form, "on"); setClassName(advanced, "on");
		if (form.value == gl_searchFormDefaultValue)
			form.value = "";
		animation(form, 300, animationSize, {w : 125}, null, 15);
	});
	addEvent(form, "blur", function()
	{
		removeClassName(form, "on"); removeClassName(advanced, "on");
		if (form.value == "")
			form.value = gl_searchFormDefaultValue;
		animation(form, 300, animationSize, {w : 75}, null, 15);
	});
	
	// The search
	addEvent(form, "keyup", function() { explorerApplyFilter() });
	
	///////////////////////////////////////////////////////////////////////////
	/////////////////////////////// FILE TABLE ////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Get the files and display them in the explorer
	
	function getFiles()
	{
		// Get the files
		request("GET", "Execute/Select", function(HttpRequest)
		{
		// Clean the tables
		body.innerHTML = "<table></table>";
		table = body.getElementsByTagName("table")[0];
		header.deleteRow(0);
		header.insertRow(0);
		
		// List the headers
		if (HttpRequest.status == 200 && HttpRequest.responseText != "")
		{
			files = eval('(' + HttpRequest.responseText + ')');
			files.columns = [ {"name" : "", "value" : "type"},
							  {"name" : "Nom", "value" : "name"},
							  {"name" : "Date d'ajout", "value" : "created"},
							  {"name" : "Taille", "value" : "size"},
							  {"name" : "Type", "value" : "type"} ];

		// Display the headers
		for (var i = 0, s = files.columns.length; i < s; ++i)
		{
			var th = document.createElement('th');
			th.innerHTML = "<div></div>" + files.columns[i].name;
			if (i == 0)
				setClassName(th, "icon");
			header.rows[0].appendChild(th);
		}

		// Manage the headers
		var headers = header.getElementsByTagName("th");
		
		setClassName(headers[sortBy], "asc");
		for (var i = 0, s = headers.length; i < s; ++i)
		{
			addEvent(headers[i], "click", function(event)
			{
				var node = getEventTarget(event, "th");
				if (getClassName(node, "asc"))
				{
					removeClassName(node, "asc");
					setClassName(node, "desc");
					orderBy = "desc";
				}
				else if (getClassName(node, "desc"))
				{
					removeClassName(node, "desc");
					setClassName(node, "asc");
					orderBy = "asc";
				}
				else
				{
					for (var j = 0, t = headers.length; j < t; ++j)
					{
						removeClassName(headers[j], "desc");
						removeClassName(headers[j], "asc");
					}
					setClassName(node, "asc");
					orderBy = "asc";
				}
				sortBy = node.cellIndex;
				explorerSort();
			});
		}
		explorerSort();

		///////////////////////////////////////////////////////////////////////////
		////////////////////////////// TYPE FILTERS ///////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		// The types div can be seen on the right side of the type filter,
		// So it has to be at the same color than others
		addEvent(others, "mouseover", function() { setClassName(types, "hover"); });
		addEvent(others, "mouseout", function() { removeClassName(types, "hover"); });
		
		// Add the onClick events
		addEvent(audios, "click", function() { getClassName(audios, "on") ? removeClassName(audios, "on") : setClassName(audios, "on"); explorerApplyFilter(); });
		addEvent(videos, "click", function() { getClassName(videos, "on") ? removeClassName(videos, "on") : setClassName(videos, "on"); explorerApplyFilter(); });
		addEvent(images, "click", function() { getClassName(images, "on") ? removeClassName(images, "on") : setClassName(images, "on"); explorerApplyFilter(); });
		addEvent(documents, "click", function() { getClassName(documents, "on") ? removeClassName(documents, "on") : setClassName(documents, "on"); explorerApplyFilter(); });
		addEvent(others, "click", function() {
			if (getClassName(others, "on")) { removeClassName(others, "on"); removeClassName(types, "on"); }
			else { setClassName(others, "on"); setClassName(types, "on"); }
			explorerApplyFilter();
		});
		}}); // !request on Execute/Select
	}
	getFiles();
	
	// Build the file table
	function explorerBuildTable()
	{
		// Erase the old table
		for (var i = 0, s = table.rows.length; i < s; ++i)
			table.deleteRow(0);
		for (var i = 0, s = files.length; i < s; ++i)
		{
			table.insertRow(table.rows.length);
			var row = table.rows[table.rows.length - 1];
			if (table.rows.length % 2)
				row.className = "odd";
			else
				row.className = "even";
			for (var j = 0, t = files.columns.length; j < t; ++j)
			{
				row.insertCell(j);
				if (j > 0)
					row.cells[j].innerHTML = files[i][files.columns[j].value];
			}
			row.cells[0].className = "icon";
			row.cells[0].innerHTML = "<div class=\"" + files[i].type + "\"></div>";
			row.file = i;
			// When the user click on a row, the information window is opened
			addEvent(row, "click", function(event)
			{
				if (!(id = getWindowIdByType("Information")))
					openWindow("Informations", "Information", event, files[getEventTarget(event, "tr").file]);
				else
					gl_windows[id].getObject().setFile(files[getEventTarget(event, "tr").file]);
			});
			// The double click event opens the player
			addEvent(row, "dblclick", function(event)
			{
				if (!(id = getWindowIdByType("Player")))
					openWindow("Lecteur", "Player", event, files[getEventTarget(event, "tr").file]);
				else
					gl_windows[id].getObject().setFile(files[getEventTarget(event, "tr").file]);
			});
		}
	}
	
	// Sort the files
	function explorerSort()
	{
		// Sort the column
		files.sort(function(a, b)
		{
			var result;
			a = a[files.columns[sortBy].value];
			b = b[files.columns[sortBy].value];
			if (isNaN(a) || isNaN(b))
				result = compare(a.toLowerCase(), b.toLowerCase());
			else
				result = Number(a) - Number(b);
			if (orderBy == "desc")
				result = -result;
			return (result);
		});
		explorerBuildTable();
		explorerApplyFilter();
	}
	
	// Apply all the filters on the files list
	function explorerApplyFilter()
	{
		// If all the filter are off, all the files are displayed
		var all = false;
		if (!getClassName(audios, "on") && !getClassName(videos, "on") && !getClassName(images, "on")
			&& !getClassName(documents, "on") && !getClassName(others, "on"))
			all = true;
		var l = 0;
		// Get the regexps from the search form
		var regexps = new Array();
		if (form.value != gl_searchFormDefaultValue && form.value.length > 0)
		{
			var parts = form.value.replace(".", "\\.").split(" ");
			for (var i = 0, s = parts.length; i < s; ++i)
				if (parts[i].length > 0)
					regexps.push(new RegExp(parts[i], "i"));
		}
		// Run through the files, and hide/display them according to the filters
		for (var i = 0, s = table.rows.length; i < s; ++i)
		{
			var row = table.rows[i];
			var display = true;
			
			// Test the search form
			if (regexps.length > 0)
				for (var j = 0, k = regexps.length; j < k && display; ++j)
					if (row.innerHTML.search(regexps[j]) < 0)
						display = false;
			// Test the filters
			if (!eval("getClassName(" + files[row.file].type + "s, \"on\")") && !all)
				display = false;
			if (display)
			{
				row.style.display = "";
				++l;
			}
			else
				row.style.display = "none";
			if (l % 2)
				row.className = "odd";
			else
				row.className = "even";
		}
	}
	
	this.update = function ()
	{
		getFiles();
	}
}

function explorerActiveOneButton(param1, param2, param3, param4)
{
	setClassName(param1, "on");
	if (param2 != undefined)
		removeClassName(param2, "on");
	if (param3 != undefined)
		removeClassName(param3, "on");
	if (param4 != undefined)
		removeClassName(param4, "on");
}

gl_resources.jsLoaded("Explorer");
function initializeExplorer(id) { new Explorer(id); }
