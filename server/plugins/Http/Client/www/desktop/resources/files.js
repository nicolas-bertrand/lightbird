function ResourceFiles(task)
{
    var resource = this;
    
    resource.init = function ()
    {
        // Nodes
        resource.node = new Object();
        resource.node.element = $(task.content).children(".files")[0];
        resource.node.controls = $(resource.node.element).children(".controls")[0];
        resource.node.header = $(resource.node.element).children(".header")[0];
        resource.node.list = $(resource.node.element).children(".list")[0];
        
        // Members
        resource.icons = new Icons(); // Generates the SVG icons
        resource.layout = new Layout(); // Manages the layout of the columns
        resource.header = new Header(); // The header of the files list
        resource.container = new List(); // The container handles the display of the files list. Each container have its own layout.
        resource.fileNumber = 0; // The number of the last file played in the playlist
        
        // Sets the resource instance to the task, so that it can call close and onResize
        task.setResource(resource);
    }
    
    // The task have been resized.
    resource.onResize = function (left, top, width, height)
    {
        resource.container.onResize(left, top, width, height);
    }
    
    // Closes the resource.
    resource.close = function ()
    {
        gl_player.closePlaylist(resource);
        for (var key in resource)
            resource[key] = undefined;
    }

// Playlist interface
    {
        resource.getNumberFiles = function ()
        {
            return ({ numerator : resource.fileNumber, denominator : gl_files.list.length });
        }
        
        resource.readyToPlay = function (playerInterface, fileIndex)
        {
            gl_player.playFile(resource, playerInterface, fileIndex);
        }
    }
    
// Manages the files list header.
function Header()
{
    var self = this;
    
    self.init = function ()
    {
        self.add = $(resource.node.header).children(".add")[0];
        self.addColumn("name");
        self.addColumn("size");
        self.addColumn("created");
    }
    
    // Adds a column to the header.
    // @param name : The name corresponds to the property of the file associated with the column.
    self.addColumn = function (name)
    {
        var column = $("<div class='column'></div>");
        var text = $("<div></div>");
        var translated = name;
        if (T.Files[name])
            translated = T.Files[name];
        text[0].innerHTML = translated;
        text.appendTo(column);
        column.width(resource.layout.getDefaultWidth(name));
        column.mousedown(function (e) { self.mouseDownHeader(e); });
        column.insertBefore(self.add);
        // Adds the separator
        var separator = $("<div class='separator'></div>");
        var inner = $("<div></div>");
        inner.appendTo(separator);
        separator.insertAfter(column);
        separator.mousedown(function (e) { self.mouseDownSeparator(e); });
        // Saves some information in the column
        column[0].originalName = name;
        column[0].align = resource.layout.getAlignment(name);
        column.addClass(column[0].align);
    }
    
    // Removes a column from the header and the container.
    self.removeColumn = function (column)
    {
        // Notifies the container
        resource.container.removeColumn(column);
        // Removes the separator
        $(column).next().remove();
        // And the column
        $(column).remove();
    }
    
    // Returns the list of the columns in the header.
    self.getColumns = function ()
    {
        return ($(resource.node.header).children(".column"));
    }
    
    self.mouseDownHeader = function (e)
    {
        if (e.which == 2)
            self.removeColumn(e.currentTarget);
    }
    
    // Starts to resize the comumn.
    self.mouseDownSeparator = function (e)
    {
        if (e.which != 1)
            return ;
        var param = new Object();
        param.separator = e.currentTarget;
        // Saves the width of the previous columns
        param.width = new Array();
        for (var p = $(e.currentTarget).prev(); p.length > 0; p = p.prev().prev())
            param.width.push(p.width());
        gl_desktop.drag.start(e, e.currentTarget, self, "mouseMoveSeparator", undefined, undefined, param);
    }
    
    // Resizes the column.
    self.mouseMoveSeparator = function (e, p)
    {
        var mouse = gl_desktop.drag.getMouse();
        var element = gl_desktop.drag.getElement();
        var diff = e.pageX - (mouse.x + element.x);
        
        // Updates the width of the columns
        var column = $(p.separator).prev();
        var flag = true;
        for (var i = 0; p.width.length > i && flag; ++i, column = column.prev().prev())
        {
            var width = diff + p.width[i];
            var minWidth = resource.layout.getMinWidth(column[0].originalName);
            // The column can absorb the shift
            if (width > minWidth)
                flag = false;
            // All the space of this column have been used, so we go to the previous sibling
            else
            {
                width = minWidth;
                if (p.width.length > 1)
                    diff += (p.width[i] - minWidth);
            }
            column.width(width);
        }
        column = $(p.separator).prev();
        // Ensures that the previous column have their original width
        if (i < p.i)
            for (var j = 0; p.width.length > j; ++j, column = column.prev().prev())
                if (j >= i)
                    column.width(p.width[j]);
        p.i = i;
        resource.container.updateColumns();
    }

    self.init();
    return (self);
}

// Manages the files list.
function List()
{
    var self = this;
    
    self.init = function ()
    {
        // Nodes
        self.list = resource.node.list;
        self.table = $(self.list).children("table")[0];
        self.top = $(self.list).children(".top")[0];
        self.bottom = $(self.list).children(".bottom")[0];
        
        // Members
        self.listHeight; // The height of the file list
        self.selectedFiles = new Object(); // The list of the files selected by the user
        self.lastFileSelected; // The last file selected or deselected
        self.index = 0; // Used to add an element at the end of the selectedFiles list
        
        // Events
        $(self.table).mousedown(function (e) { self.mouseDown(e); });
        $(self.table).dblclick(function (e) { self.dblClick(e); });
        
        self.updateColumns();
        
        // Hides the scroll if all the files can be seen at the same time
        if (self.listHeight / C.Files.listRowHeight >= gl_files.list.length)
            $(self.list).removeClass("scroll");
        else
            $(self.list).addClass("scroll");
        self.list.scrollTop = 0;
		// Displays the files
        for (var i = 0; i < gl_files.list.length; i++)
            self.addRow(i, i);
        self.addEmptyRows();
    }
    
    // Updates the width of the columns.
    self.updateColumns = function ()
    {
        var columns = resource.header.getColumns();
        var colgroup = $(self.table).children("colgroup");
        var cols = colgroup.children("col");
        for (var i = 0; i < columns.length; ++i)
        {
            var col;
            // The columns doesn't exists yet
            if (cols.length <= i + 1)
            {
                col = $("<col></col>");
                col[0].name = columns[i].originalName;
                col.appendTo(colgroup);
            }
            else
                col = $(cols[i + 1]);
            // Updates the width
            if (i != 0)
                col.width($(columns[i]).width() + C.Files.headerSeparatorWidth);
            else
                col.width($(columns[i]).width() + C.Files.headerSeparatorWidth * 2);
        }
    }
    
    // Removes a column from the list.
    self.removeColumn = function (column)
    {
        // Removes the col element
        var colgroup = $(self.table).children("colgroup");
        var cols = colgroup.children("col");
        for (var i = 0; i < cols.length; ++i)
            if (cols[i].name == column.originalName)
                $(cols[i]).remove();
        // Removes the column
        var rows = self.table.rows;
        var cells;
        for (var i = 0; i < rows.length; ++i)
            for (var j = 0, cells = rows[i].cells; j < cells.length; ++j)
                if (cells[j].name == column.originalName)
                    $(cells[j]).remove();
    }
    
    // Resizes the list to fit into the task content height.
    self.onResize = function (left, top, width, height)
    {
        self.listHeight = height - C.Files.controlsHeight - C.Files.headerHeight - 2; // -2 is the border of the header
        self.list.style.height = self.listHeight + "px";
        // Hides the scroll when all the files can be seen at the same time
        if (self.listHeight / C.Files.listRowHeight >= gl_files.list.length)
        {
            $(self.list).removeClass("scroll");
            self.list.scrollTop = 0;
        }
        else
            $(self.list).addClass("scroll");
        // Ensures that the list is always filled with rows
        self.addEmptyRows();
    }
    
    // Adds a row at the given position.
    // @param relative : The position of the new row in the table (-1 for the end of the list).
    // @param absolute : The absolute position of the row in the list, which includes the rows that are not visible.
    self.addRow = function (relative, absolute)
    {
        var isFile = absolute < gl_files.list.length;
        
        // Creates the row
        var row = self.table.insertRow(relative);
        $(row).addClass(absolute % 2 ? "even" : "odd");
        // The first cell is the type of the file
        if (isFile)
        {
            var type = gl_files.list[absolute].type;
            var firstCell = 0;
            if (!row.previousSibling || !$(row.previousSibling).hasClass(type))
            {
                var cell = $(row.insertCell(-1)).addClass("type")[0];
                for (var i = absolute; i < gl_files.list.length && gl_files.list[i].type == type; ++i)
                    ;
                cell.rowSpan = i - absolute;
                firstCell = 1;
            }
        }
        else
        {
            $(row).addClass("empty");
            $(row.insertCell(-1));
        }
        // Builds the other cells
        var columns = resource.header.getColumns();
        for (var i = 0; i <= columns.length; ++i)
            row.insertCell(-1);
        // If the row represents a file we fill its cells
        if (isFile)
        {
            row.fileIndex = absolute;
            $(row).addClass(gl_files.list[absolute].type);
            $(row.cells[firstCell]).addClass("first");
            for (var i = 0; i < columns.length; ++i)
            {
                var cell = row.cells[i + firstCell];
                cell.name = columns[i].originalName;
                cell.originalText = resource.layout.convert(columns[i].originalName, gl_files.list[absolute][columns[i].originalName]);
                cell.innerHTML = cell.originalText;
                if (columns[i].align != "left")
                    cell.align = columns[i].align;
            }
        }
    }

    // Adds / removes a file from the selection, depending if the keys shift or ctrl are pressed.
    self.mouseDown = function (e)
    {
        if (e.which != 1)
            return ;
        // Gets the file selected by the user
        var file = e.target;
        for (var i = 0; i < 3 && file.tagName.toLowerCase() != "tr"; ++i, file = file.parentNode)
            ;
        // If the event is directly on the table we use an other way to get the selected file
        if (e.target == self.table)
        {
            var n = Math.floor(((e.pageY - $(self.table).offset().top) / C.Files.listRowHeight));
            if (n < self.table.rows.length)
                file = self.table.rows[n];
            else
                return ;
        }
        // The row is empty
        if ($(file).hasClass("empty"))
        {
            if (e.ctrlKey == false && e.shiftKey == false)
            {
                self.selectedFiles = new Object();
                $(self.table.rows).removeClass("selected");
            }
            return ;
        }
        // Selects or deselects one file
        else if (e.ctrlKey == true && e.shiftKey == false)
        {
            // Selects the file
            if (!$(file).hasClass("selected"))
            {
                self.selectedFiles[self.index++] = file;
                $(file).addClass("selected");
            }
            // Deselects the file
            else
                for (i in self.selectedFiles)
                    if (self.selectedFiles[i] == file)
                    {
                        delete self.selectedFiles[i];
                        $(file).removeClass("selected");
                        break ;
                    }
        }
        // Selects / deselects multiple files
        else if (e.shiftKey == true && e.ctrlKey == false)
        {
            // Deselects all the files
            self.selectedFiles = new Object();
            $(self.table.rows).removeClass("selected");
            // Selects all the files between the selected file and the last file
            if (self.lastFileSelected)
            {
                var start = self.lastFileSelected;
                var end = file;
                if (self.lastFileSelected.rowIndex > file.rowIndex)
                {
                    start = file;
                    end = self.lastFileSelected;
                }
                for (var row = start; row != end.nextSibling; row = row.nextSibling)
                    if (!$(row).hasClass("selected"))
                        {
                            self.selectedFiles[self.index++] = row;
                            $(row).addClass("selected");
                        }
                return ;
            }
            // Selects the file
            else
            {
                self.selectedFiles[self.index++] = file;
                $(file).addClass("selected");
            }
        }
        // Selects multiple files
        else if (e.shiftKey == true && e.ctrlKey == true)
        {
            // No file selected
            for (var f in self.selectedFiles)
                break ;
            // Selects all the files between the selected file and the last file
            if (f || self.lastFileSelected)
            {
                var start = self.lastFileSelected;
                var end = file;
                if (self.lastFileSelected.rowIndex > file.rowIndex)
                {
                    start = file;
                    end = self.lastFileSelected;
                }
                for (var row = start; row != end.nextSibling; row = row.nextSibling)
                    if (!$(row).hasClass("selected"))
                        {
                            self.selectedFiles[self.index++] = row;
                            $(row).addClass("selected");
                        }
                return ;
            }
            // Selects the file
            else
            {
                self.selectedFiles[self.index++] = file;
                $(file).addClass("selected");
            }
        }
        // Selects one file
        else
        {
            // Deselects all the files
            $(self.table.rows).removeClass("selected");
            self.selectedFiles = new Object();
            // Selects the file
            self.index = 0;
            self.selectedFiles[self.index++] = file;
            $(file).addClass("selected");
        }
        self.lastFileSelected = file;
    }
    
    // Opens the file.
    self.dblClick = function (e)
    {
        if (e.which != 1 || e.ctrlKey || e.shiftKey)
            return ;
        var file;
        // Gets the file selected by the user
        if (e.target != self.table)
            file = $(e.target).parents("tr")[0];
        // If the event is directly on the table we use an other way to get the selected file
        else
        {
            var n = Math.floor(((e.pageY - $(self.table).offset().top) / C.Files.listRowHeight));
            if (n >= self.table.rows.length)
                return ;
            file = self.table.rows[n];
        }
        resource.fileNumber = file.rowIndex + 1;
        // If it is an audio file we play it in the global player
        if (gl_files.list[file.fileIndex].type == "audio")
            gl_player.openAudio(resource, file.fileIndex);
        // Otherwise we open a new page to view the file
        else
            gl_desktop.openPage("view", { playlistInterface : resource, fileIndex : file.fileIndex });
    }
    
    // Ensures that the list is allways filled with rows.
    self.addEmptyRows = function ()
    {
        var totalRows = self.listHeight / C.Files.listRowHeight;
        
        // Adds empty rows
        if (totalRows > gl_files.list.length)
        {
            for (var i = self.table.rows.length; self.table.rows.length < totalRows; i++)
                self.addRow(-1, i);
        }
        // Remove all the empty rows
        else
            while (self.table.rows.length > gl_files.list.length)
                self.table.deleteRow(-1);
    }
    
    self.init();
    return (self);
}

// Manages the layout of the columns.
function Layout()
{
    var self = this;
    
    self.init = function ()
    {
    }
    
    // Converts the value depending on the key.
    self.convert = function (key, value)
    {
        if (self.columns[key] && self.columns[key].method)
        {
            if (self[self.columns[key].method])
                return (self[self.columns[key].method](value));
            else if (window[self.columns[key].method])
                return (window[self.columns[key].method](value));
        }
        return (value);
    }
    
    // Returns the alignment of the column.
    self.getAlignment = function (column)
    {
        if (self.columns[column] && self.columns[column].alignment)
            return (self.columns[column].alignment);
        return ("left");
    }
    
    // Returns the min-width of the column.
    self.getMinWidth = function (column)
    {
        if (self.columns[column] && self.columns[column].minWidth)
            return (self.columns[column].minWidth);
        return (C.Files.headerMinWidth);
    }
    
    // Returns the min-width of the column.
    self.getDefaultWidth = function (column)
    {
        if (self.columns[column] && self.columns[column].defaultWidth)
            return (self.columns[column].defaultWidth);
        return (C.Files.headerDefaultWidth);
    }

    // Makes the date more readable.
    self.convertDate = function (date)
    {
        var d = T.Files.dateFormat;
        date = date.split(" ");
        date[0] = date[0].split("-");
        date[1] = date[1].split(":");
        d = d.replace("y", date[0][0]);
        d = d.replace("M", date[0][1]);
        d = d.replace("d", date[0][2]);
        d = d.replace("h", date[1][0]);
        d = d.replace("m", date[1][1]);
        return (d);
    }
    
    // Stores the layout of the columns.
    self.columns = 
    {
        size :
        {
            method : "sizeToString",
            alignment : "right",
            minWidth : 50,
            defaultWidth : 70,
            
        },
        created :
        {
            method : "convertDate",
            minWidth : 67,
            defaultWidth : 110,
        },
        name :
        {
            defaultWidth : 300,
        },
    }
    
    self.init();
    return (self);
}

// Generates and manages the SVG icons.
function Icons()
{
    var self = this;
    
    self.init = function ()
    {
        for (var type in self.icons.types)
            self.typeIcons($(resource.node.controls).children("." + type)[0], self.icons.types[type]);
        for (var control in self.icons.controls)
            self.controlIcons($(resource.node.controls).children("." + control)[0], self.icons.controls[control]);
        self.headerIcon($(resource.node.header).children(".add")[0], self.icons.add);
    }
    
    // Generates the type icons.
    self.typeIcons = function (destination, icon)
    {
        var height = C.Files.controlsHeight;
        var paper = Raphael(destination, icon.width, height);
        var background = paper.rect(0, 0, icon.width, height - 5);
        background.attr("fill", icon.background);
        background.attr("stroke", "none");
        background.hide();
        var path = paper.path(icon.path);
        path.attr("fill", "90-#6e7d8f-#929daa");
        path.attr("stroke", "none");
        path.translate(icon.width / 2, height / 2);
        var line = paper.rect(0, height - 5, icon.width, 5);
        line.attr("fill", icon.line);
        line.attr("stroke", "none");
        line.hide();
        if (icon == self.icons.types.image)
        {
            icon.border = paper.rect(0, 0, 1, height - 5);
            icon.border.attr("fill", "#bee8fd");
            icon.border.attr("stroke", "none");
            icon.border.hide();
        }
        $(destination).mouseenter(function ()
        {
            path.translate(-icon.width / 2, - height / 2);
            path.attr("fill", icon.gradient);
            path.translate(icon.width / 2, height / 2);
            background.show();
            line.show();
            if (icon.border)
                icon.border.show();
        });
        $(destination).mouseleave(function ()
        {
            path.translate(-icon.width / 2, - height / 2);
            path.attr("fill", "90-#6e7d8f-#929daa");
            path.translate(icon.width / 2, height / 2);
            background.hide();
            line.hide();
            if (icon.border)
                icon.border.hide();
        });
    }
    
    // Generates the control icons.
    self.controlIcons = function (destination, icon)
    {
        var height = C.Files.controlsHeight;
        var path = Raphael(destination, icon.width, height).path(icon.path);
        var color = "90-#6e7d8f-#929daa";
        if (icon.selected)
            color = "90-#4cc3ff-#4cd8ff";
        path.attr("fill", color);
        path.attr("stroke", "none");
        path.translate(icon.width / 2, height / 2);
        $(destination).mouseenter(function ()
        {
            path.translate(-icon.width / 2, - height / 2);
            path.attr("fill", "90-#ffb233-#ffcc33");
            path.translate(icon.width / 2, height / 2);
        });
        $(destination).mouseleave(function ()
        {
            path.translate(-icon.width / 2, - height / 2);
            path.attr("fill", color);
            path.translate(icon.width / 2, height / 2);
        });
    }
    
    // Generates the header icon.
    self.headerIcon = function (destination, icon)
    {
        var height = C.Files.headerHeight;
        var path = Raphael(destination, icon.width, height).path(icon.path);
        path.attr("fill", "#8191a5 ");
        path.attr("stroke", "none");
        path.translate(icon.width / 2, height / 2);
        path.glow({ width : 5, color : "white" });
        $(destination).mouseenter(function ()
        {
            path.attr("fill", "#4cc3ff");
        });
        $(destination).mouseleave(function ()
        {
            path.attr("fill", "#8b9db2");
        });
    }
    
    // Stores the icons data.
    self.icons = 
    {
        types :
        {
            image :
            {
                path : gl_svg.ResourceFiles.image,
                width : 47,
                gradient : "90-#33bbff-#66ccff",
                line : "#33bbff",
                background : "90-#d6f1ff-#edf9ff",
            },
            music : 
            {
                path : gl_svg.ResourceFiles.music,
                width : 40,
                gradient : "90-#46da57-#73e380",
                line : "#46da57",
                background : "90-#daf8dd-#effcf0",
            },
            video : 
            {
                path : gl_svg.ResourceFiles.video,
                width : 41,
                gradient : "90-#ff7733-#ff9865",
                line : "#ff7733",
                background : "90-#ffe4d6 -#fff3ed",
            },
            document : 
            {
                path : gl_svg.ResourceFiles.document,
                width : 36,
                gradient : "90-#ffc000-#ffd040",
                line : "#ffc000",
                background : "90-#fff2cc-#fff9e8",
            },
            other : 
            {
                path : gl_svg.ResourceFiles.other,
                width : 40,
                gradient : "90-#808080-#9e9e9e",
                line : "#808080",
                background : "90-#e2e5e9-#f2f4f5",
            },
        },
        controls :
        {
            folder : 
            {
                path : gl_svg.ResourceFiles.folder,
                width : 18,
            },
            resize : 
            {
                path : gl_svg.ResourceFiles.resize,
                width : 14,
            },
            list : 
            {
                path : gl_svg.ResourceFiles.list,
                width : 18,
                selected : true,
            },
            hierarchy : 
            {
                path : gl_svg.ResourceFiles.hierarchy,
                width : 18,
            },
            split : 
            {
                path : gl_svg.ResourceFiles.split,
                width : 18,
            },
            block : 
            {
                path : gl_svg.ResourceFiles.block,
                width : 18,
            },
        },
        add : 
        {
            path : gl_svg.ResourceFiles.add,
            width : 20,
        },
    };

    self.init();
    return (self);
}
 
    resource.init();
    return (resource);
}

function initialize_resource_files(task) { return new ResourceFiles(task); }
gl_resources.jsLoaded("files");
