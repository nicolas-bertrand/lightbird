var gl_upload;

function Upload(id)
{
	var window = gl_windows[id];
	if (window == undefined)
		return ;
	window.setObject(this);
	var object = this;
	gl_upload = this;
	var className = getElementsByClassName;
	// Some nodes
	var node = className("upload", window.getContentNode());
	var template = className("template", node);
	var files = className("files", node);
	
	var rec
	rec = function ()
	{
		var idUpload = "upload" + Math.floor(Math.random() * 10000000000000000);
		var formNode = className("form", node);
		formNode.innerHTML = "<form method=\"post\" enctype=\"multipart/form-data\" target=\"" + idUpload + "\" ><input name=\"file\" type=\"file\" /></form>";
		
		// Add a hidden file, for the next upload
		file = document.createElement("div");
		file.className = "file hide";
		file.innerHTML = template.innerHTML;
		file.idUpload = idUpload;
		className("frame", file).innerHTML = "<iframe name=\"" + idUpload + "\" src=\"/Client/blank\" ></iframe>";
		var div = files.firstChild;
		while (div && div.tagName && div.tagName.toLowerCase() != "div")
			div = div.nextSibling;
		if (div)
			files.insertBefore(file, div);
		else
			files.appendChild(file);
		updateParity(files);
		addEvent(className("remove", file), "click", function (e)
		{
			files.removeChild(getEventTarget(e).parentNode.parentNode);
			updateParity(files);
		});
		
		// Get som nodes
		var input = formNode.getElementsByTagName("input")[0];
		var frame = file.getElementsByTagName("iframe")[0];
		var form = formNode.getElementsByTagName("form")[0];
		
		// When a user has selected a file
		addEvent(input, "change",  function ()
		{
			if (!input.value)
				return ;
			
			// Get the name of the file
			var name = input.value.replace(/\\/g, "/");
			name = name.substr(name.lastIndexOf("/") + 1, name.length);
			
			// Display the file in the upload list
			className("name", file).innerHTML = name;
			var date = new Date();
			var z = function (n) { return ((n < 10) ? ("0" + n) : (n)); }
			className("date", file).innerHTML = date.getFullYear() + "/" + z(date.getMonth()) + "/" + z(date.getDate()) + " " + z(date.getHours()) + ":" + z(date.getMinutes());
			removeClassName(file, "hide");
			
			// Set the action and submit the form
			form.action = "/Client/Execute/StartUpload?id=" + idUpload + "&name=" + escape(name);
			form.submit();
			
			// Update the progress bar
			setTimeout("gl_upload.progress(\"" + idUpload + "\")", 500);
			
			rec();
			
			// When the file has been uploaded
			addEvent(frame, "load",  function (e)
			{
				var file = getEventTarget(e).parentNode.parentNode;
				setClassName(file, "finished");
				className("percent", file).innerHTML = "100";
				className("state", file).innerHTML = "Termin&eacute;";
				// Update the explorer
				var ids = getWindowsIdByType("Explorer");
				for (var i = 0; i < ids.length; ++i)
					gl_windows[ids[i]].getObject().update()
			});
		});
	}
	rec();
	
	// Display the progress data on the upload
	this.progress = function (idUpload)
	{
		request("GET", "Execute/StateUpload?id=" + idUpload, function (HttpRequest)
		{
			for (var file = files.firstChild; file != undefined; file = file.nextSibling)
			{
				if (file.tagName && file.tagName.toLowerCase() == "div" && file.idUpload == idUpload)
				{
					// The upload is finished, so there is no progress
					if (getClassName(file, "finished"))
						return ;
					// Get the data on the progress of the upload
					var state = eval('(' + HttpRequest.responseText + ')');
					var percentage = (state.progress * 100) / state.size;
					// Check the state
					if (state.size > 0)
					{
						file.started = true;
						setTimeout("gl_upload.progress(\"" + idUpload + "\")", 500);
					}
					else if (state.size < 0 && file.started)
						percentage = 100;
					else
					{
						setTimeout("gl_upload.progress(\"" + idUpload + "\")", 500);
						return ;
					}
					// Update the display of the upload
					setClassName(className("bar_left", file), "done");
					if (state.size > 0)
					{
						className("size_info", file).innerHTML = updateParseSize(state.size);
						className("size", file).innerHTML = updateParseSize(state.size);
						className("uploaded", file).innerHTML = updateParseSize(state.progress);
						// Set the bit rate
						var time = ((new Date()).getTime() + (new Date()).getMilliseconds() / 1000) / 1000;
						var progress = parseInt(state.progress);
						var speed = Math.floor(((progress - file.lastUpdateProgress) / (time - file.lastUpdateTime)));
						if (speed)
							className("speed", file).innerHTML = updateParseSize(speed) + "/s";
						file.lastUpdateProgress = progress;
						file.lastUpdateTime = time;
						// Estimate the remaining time
						if (speed)
						{
							className("time", file).innerHTML = updateParseTime(Math.floor((parseInt(state.size) - progress) / speed));
						}
					}
					// The upload is finished
					if (percentage == 100)
					{
						setClassName(className("bar_right", file), "done");
						className("state", file).innerHTML = "V&eacute;rification";
						className("time", file).innerHTML = "Quelques instants";
						className("uploaded", file).innerHTML = className("size", file).innerHTML;
						className("speed", file).innerHTML = "0 Ko/s";
					}
					className("percent", file).innerHTML = Math.floor(percentage);
					className("progress_bar", file).style.width = percentage + "%";
				}
			}
		});
	}
}

function updateParseSize(size)
{
	size = size + "";
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

function updateParseTime(time)
{
	time = parseInt(time);
	if (!time)
		return ("Quelques instants");
	if (time < 60)
		return (time + " secondes");
	else if (time / 60 < 120)
		return (parseInt(time / 60) + " minutes");
	return (parseInt(time / 60 / 60) + " heures");
}

// Update the odd and even class name of the files
function updateParity(files)
{
	var i = 1;
	for (var div = files.firstChild; div != undefined; div = div.nextSibling)
		if (div.tagName && div.tagName.toLowerCase() == "div")
		{
			if (i++ % 2)
			{
				removeClassName(div, "odd");
				setClassName(div, "even");
			}
			else
			{
				removeClassName(div, "even");
				setClassName(div, "odd");
			}

		}
}

gl_resources.jsLoaded("Upload");
function initializeUpload(id) { new Upload(id); }
