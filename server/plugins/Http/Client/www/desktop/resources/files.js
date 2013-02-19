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
        resource.fileNumber = 0; // The number of the current file of the playlist
        resource.fileIndex = 0; // The index of the current file of the playlist
        resource.files = new Array(); // The list of the files to display. Points to gl_files.
        
        // Fills the files list
        for (var i = 0; i < gl_files.list.length; ++i)
            resource.files[i] = i;
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
        resource.container.close();
        for (var key in resource)
            resource[key] = undefined;
    }

// Playlist interface
    {
        resource.getNumberFiles = function ()
        {
            return ({ fileNumber : resource.fileNumber,
                      numberOfFiles : resource.files.length,
                      fileIndex : resource.fileIndex });
        }
        
        resource.readyToPlay = function (fileInterface)
        {
            gl_player.playFile(resource, fileInterface);
        }
    }
    
// Manages the files list header.
function Header()
{
    var self = this;
    
    self.init = function ()
    {
        self.columns; // The list of the columns in the header
        self.sortColumn; // The name of the column by which the files are sorted
    
        self.add = $(resource.node.header).children(".add")[0];
        self.addColumn("type");
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
        column.addClass(name);
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
        // Updates the list of the columns
        self.columns = $(resource.node.header).children(".column");
        self.updateSeparators();
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
        // Updates the list of the columns
        self.columns = $(resource.node.header).children(".column");
    }
    
    // Returns the list of the columns in the header.
    self.getColumns = function ()
    {
        return (self.columns);
    }
    
    self.mouseDownHeader = function (e)
    {
        // Sorts the column
        if (e.which == 1)
        {
            // If the column is already sorted, we just reverse the files list
            if (self.sortColumn == e.currentTarget.originalName)
                resource.files.reverse();
            // Performs the actual sorting
            else
                resource.sortFiles(e.currentTarget.originalName);
            self.sortColumn = e.currentTarget.originalName;
            resource.container.onSort();
        }
        // Removes the column
        else if (e.which == 2)
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
        gl_desktop.drag.start(e, e.currentTarget, self, "mouseMoveSeparator", undefined, "updateSeparators", param);
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
            resource.layout.onResize(column[0].originalName, width);
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
    
    // Shortens the width of the separators next to a small column.
    self.updateSeparators = function ()
    {
        self.getColumns().each(function ()
        {
            var column = $(this);
            if (column.width() <= C.Files.headerSeparatorMinColumnWidth)
            {
                column.next().addClass("right");
                column.prev().addClass("left");
            }
            else
            {
                column.next().removeClass("right");
                column.prev().removeClass("left");
            }
        });
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
        self.top = $(self.list).children(".top");
        self.table = $(self.list).children("table")[0];
        self.bottom = $(self.list).children(".bottom");
        
        // Members
        self.listHeight; // The height of the file list
        self.selectedFiles = new Object(); // The list of the files selected by the user. The key is the global file index.
        self.lastFileSelected = { local: undefined, global: undefined }; // The last file selected or deselected
        self.oldTableLength; // The length of the table before its last modification
        
        // Events
        $(self.table).mousedown(function (e) { self.mouseDown(e); });
        $(self.table).dblclick(function (e) { self.dblClick(e); });
        $(self.list).scroll(function (e) { self.scroll(e); });
        
        self.updateColumns();
        // Initializes the type column
        resource.header.getColumns().filter(".type").children().addClass("hide");
    }
    
    // Closes the list.
    self.close = function ()
    {
        for (var key in self)
            resource[key] = undefined;
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
            if (cols.length <= i)
            {
                col = $("<col></col>");
                col[0].name = columns[i].originalName;
                col.appendTo(colgroup);
            }
            else
                col = $(cols[i]);
            // Updates the width
            col.width($(columns[i]).width() + C.Files.headerSeparatorWidth);
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
    
    // The files have been sorted, so we have to update the displayed rows.
    self.onSort = function (column)
    {
        var firstFileIndex = Math.floor(self.top.height() / C.Files.listRowHeight);
        
        for (var i = 0; i < self.table.rows.length; ++i)
            self.setFileInRow(firstFileIndex + i, self.table.rows[i]);
        // The local file index is no longer valid
        self.lastFileSelected.local = undefined;
    }
    
    // Resizes the list to fit into the task content height.
    self.onResize = function (left, top, width, height)
    {
        self.listHeight = height - C.Files.controlsHeight - C.Files.headerHeight - 2; // -2 is the border of the header
        self.list.style.height = self.listHeight + "px";
        // Updates the files list
        self.updateRows();
        self.scroll();
        // Hides the scroll when all the files can be seen at the same time
        if (self.listHeight / C.Files.listRowHeight >= resource.files.length)
        {
            $(self.list).removeClass("scroll");
            self.list.scrollTop = 0;
        }
        else
            $(self.list).addClass("scroll");
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
        var globalFileIndex = resource.files[file.fileIndex];
        // Selects or deselects one file
        if (e.ctrlKey == true && e.shiftKey == false)
        {
            // Selects the file
            if (!$(file).hasClass("selected"))
            {
                self.selectedFiles[globalFileIndex] = true;
                $(file).addClass("selected");
            }
            // Deselects the file
            else if (self.selectedFiles[globalFileIndex])
            {
                delete self.selectedFiles[globalFileIndex];
                $(file).removeClass("selected");
            }
        }
        // Selects / deselects multiple files
        else if (e.shiftKey == true)
        {
            // Deselects all the files when ctrl is not pressed
            if (e.ctrlKey == false)
            {
                self.selectedFiles = new Object();
                $(self.table.rows).removeClass("selected");
            }
            // Selects all the files between the selected file and the last file selected
            if (self.lastFileSelected != undefined)
            {
                if (self.lastFileSelected.local == undefined)
                    self.lastFileSelected.local = self.searchLocalFromGlobalFileIntex(self.lastFileSelected.global);
                var start = file.fileIndex;
                var end = self.lastFileSelected.local;
                var increment = (start > end ? -1 : 1);
                for (var i = 0; i <= Math.abs(start - end); i++)
                {
                    var rowIndex = file.rowIndex + i * increment;
                    if (rowIndex >= 0 && rowIndex < self.table.rows.length)
                        $(self.table.rows[rowIndex]).addClass("selected");
                    self.selectedFiles[resource.files[file.fileIndex + i * increment]] = true;
                }
                return ;
            }
            // Selects the file
            else
            {
                self.selectedFiles[globalFileIndex] = true;
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
            self.selectedFiles[globalFileIndex] = true;
            $(file).addClass("selected");
        }
        self.lastFileSelected = { local: file.fileIndex, global: globalFileIndex };
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
        if ($(file).hasClass("empty"))
            return ;
        resource.fileNumber = file.rowIndex + 1;
        resource.fileIndex = resource.files[file.fileIndex];
        // If it is an audio file we play it in the global player
        if (gl_files.list[resource.fileIndex].type == "audio")
            gl_player.openAudio(resource, resource.fileIndex);
        // Otherwise we open a new page to view the file
        else
            gl_desktop.openPage("view", { playlistInterface : resource, fileIndex : resource.fileIndex });
    }
    
    // Scrolls the file list.
    self.scroll = function ()
    {
        // Computes the top padding that simutates the scroll
        var top = Math.floor(self.list.scrollTop / C.Files.listRowHeight) * C.Files.listRowHeight;
        top = Math.max(0, Math.min(top, (resource.files.length - self.table.rows.length) * C.Files.listRowHeight));
        
        // Update the file list if needed
        if (top != self.top.height() || self.table.rows.length != self.oldTableLength)
        {
            var bottom = Math.max(Math.floor((resource.files.length * C.Files.listRowHeight - self.listHeight - top) / C.Files.listRowHeight - 1) * C.Files.listRowHeight, 0);
            self.top.height(top);
            self.bottom.height(bottom);
            
            var firstFileIndex = Math.floor(self.top.height() / C.Files.listRowHeight);
            var diff = firstFileIndex - self.table.rows[0].fileIndex;
            var row;
            
            // Scrolls up
            if (diff < 0)
            {
                var lastRow;
                for (var i = 0; i < self.table.rows.length && self.table.rows[i].fileIndex != firstFileIndex + i; ++i)
                {
                    row = self.table.rows[self.table.rows.length - 1];
                    lastRow ? $(row).insertAfter(lastRow) : $(row).prependTo(row.parentNode);
                    self.setFileInRow(firstFileIndex + i, row);
                    lastRow = row;
                }
                for (var i = self.table.rows.length - 1; i >= 0 && !(row = self.table.rows[i]).className; --i)
                    self.setFileInRow(firstFileIndex + i, row);
            }
            // Scrolls down
            if (diff > 0 || !diff)
            {
                for (var i = 0; i < self.table.rows.length && (row = self.table.rows[0]).fileIndex != firstFileIndex; ++i)
                    $(row).appendTo(row.parentNode);
                for (var i = self.table.rows.length - 1; i >= 0 && (row = self.table.rows[i]).fileIndex != firstFileIndex + i; --i)
                    self.setFileInRow(firstFileIndex + i, self.table.rows[i]);
            }
            self.oldTableLength = self.table.rows.length;
        }
    }
    
    // Creates / removes enouth rows to fill the visible part of the list.
    self.updateRows = function ()
    {
        var numberRows = Math.max(Math.ceil(self.listHeight / C.Files.listRowHeight) + 1, 1);
        var headerColumns = resource.header.getColumns();
        var j = 0;
        
        // There is enouth rows to display all the files
        if (resource.files.length <= numberRows - 1)
            numberRows--;
        // Adds the necessary rows
        if (self.table.rows.length < numberRows)
            while (self.table.rows.length < numberRows)
            {
                var row = self.table.insertRow(-1);
                // Creates the content of the row
                for (var i = 0; i < headerColumns.length; ++i)
                    $(row.insertCell(-1)).addClass(headerColumns[i].originalName);
                $(row.insertCell(-1));
            }
        // Removes the superfluous rows
        else
        {
            var lastRowDisplayed = (numberRows <= resource.files.length && $(self.list).hasClass("scroll") && !self.bottom.height());
            while (self.table.rows.length > numberRows)
                // If the last row is displayed we remove the top row
                if (lastRowDisplayed)
                {
                    self.top.height(self.top.height() + C.Files.listRowHeight);
                    self.table.deleteRow(0);
                }
                else
                    self.table.deleteRow(-1);
        }
    }
    
    // Fills the row with the given file informations.
    // fileIndex : The file to put in the row (can be out of range).
    // row : The row to fill.
    self.setFileInRow = function (fileIndex, row)
    {
        var columns = $(row).children();
        var headerColumns = resource.header.getColumns();
        
        row.className = (fileIndex % 2 ? "even" : "odd");
        // Sets the file to the row
        if (fileIndex < resource.files.length)
        {
            var globalFileIndex = resource.files[fileIndex];
            var file = gl_files.list[globalFileIndex];
            row.fileIndex = fileIndex;
            if (self.selectedFiles[globalFileIndex])
                $(row).addClass("selected");
            $(row).addClass("file").addClass(file.type);
            for (var i = 0; i < headerColumns.length; ++i)
            {
                var column = columns[i];
                column.name = headerColumns[i].originalName;
                column.originalText = resource.layout.convert(headerColumns[i].originalName, file[headerColumns[i].originalName]);
                column.innerHTML = column.originalText;
                if (headerColumns[i].align != "left")
                    column.style.textAlign = headerColumns[i].align;
            }
        }
        // Cleans the row
        else if (!$(row).hasClass("empty"))
        {
            delete row.fileIndex;
            for (var i = 0; i < headerColumns.length; ++i)
            {
                var column = columns[i];
                delete column.name;
                delete column.originalText;
                column.innerHTML = "";
                column.style.textAlign = "";
            }
            $(row).addClass("empty");
        }
    }
    
    // Searches and returns the local file index that corresponds to the given global index.
    self.searchLocalFromGlobalFileIntex = function (global)
    {
        for (var local = 0; local < resource.files.length; ++local)
            if (resource.files[local] == global)
                return (local);
        return (undefined);
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
        if (self.columns[key] && self.columns[key].convert)
        {
            if (self[self.columns[key].convert])
                return (self[self.columns[key].convert](value));
            else if (window[self.columns[key].convert])
                return (window[self.columns[key].convert](value));
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

    // Returns true if the column contains numerical values.
    self.isNumerical = function (column)
    {
        return (self.columns[column] && self.columns[column].numerical)
    }

    // Converts the generic type name (audio document image other video).
    self.convertType = function (type)
    {
        return ("<div></div><span>" + T.Files[type] + "</span>");
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
    
    // The column have been resized
    self.onResize = function (column, width)
    {
        if (resource.container.table && column == "type")
        {
            var table = $(resource.container.table);
            var isTypeText = table.hasClass("type_text");
            if (width > C.Files.columnTypeTextMinWidth)
            {
                if (!isTypeText)
                {
                    resource.header.getColumns().filter(".type").children().removeClass("hide");
                    table.addClass("type_text");
                }
            }
            else if (isTypeText)
            {
                resource.header.getColumns().filter(".type").children().addClass("hide");
                table.removeClass("type_text");
            }
            self.columns[column]
        }
    }
    
    // Stores the layout of the columns.
    self.columns = 
    {
        type:
        {
            minWidth: 10,
            defaultWidth: 10,
            convert: "convertType"
        },
        name:
        {
            defaultWidth: 300
        },
        size:
        {
            convert: "sizeToString",
            alignment: "right",
            minWidth: 50,
            defaultWidth: 70,
            numerical: true
        },
        created:
        {
            convert: "convertDate",
            minWidth: 67,
            defaultWidth: 110
        }
    };
    
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

// Sorts the files based on the given column, using the merge sort algorithm.
var MergeSort =
{
    main: function (column)
    {
        MergeSort.column = column;
        if (resource.layout.isNumerical(column))
            MergeSort.comparison = MergeSort.compareNumber;
        else
            MergeSort.comparison = MergeSort.compareString;
        resource.files = MergeSort.sort(resource.files);
    },

    sort: function (list)
    {
        if (list.length < 2)
            return (list);
        var middle = Math.ceil(list.length / 2);
        return (MergeSort.merge(MergeSort.sort(list.slice(0, middle)), MergeSort.sort(list.slice(middle))));
    },
    
    merge: function (left, right)
    {
        var result = new Array();
        
        while ((left.length > 0) && (right.length > 0))
        {
            if(MergeSort.comparison(left[0], right[0]))
                result.push(left.shift());
            else
                result.push(right.shift());
        }
        while (left.length > 0)
            result.push(left.shift());
        while (right.length > 0)
            result.push(right.shift());
        return (result);
    },
    
    compareNumber: function (a, b)
    {
        return (parseInt(gl_files.list[a][MergeSort.column]) <= parseInt(gl_files.list[b][MergeSort.column]));
    },
    
    compareString: function (a, b)
    {
        return (gl_files.list[a][MergeSort.column].toLowerCase() <= gl_files.list[b][MergeSort.column].toLowerCase());
    }
};
resource.sortFiles = MergeSort.main;

    resource.init();
    return (resource);
}

function initialize_resource_files(task) { return new ResourceFiles(task); }
gl_resources.jsLoaded("files");
