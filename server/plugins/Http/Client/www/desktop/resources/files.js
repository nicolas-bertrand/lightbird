function ResourceFiles(task)
{
    var resource = this;
    
    resource.init = function ()
    {
        // Nodes
        resource.node = new Object();
        resource.node.element = task.content.children(".files")[0];
        resource.node.controls = $(resource.node.element).children(".controls")[0];
        resource.node.types = $(resource.node.controls).children(".types")[0];
        resource.node.other = $(resource.node.controls).children(".other")[0];
        resource.node.columns = $(resource.node.element).children(".columns");
        resource.node.list = $(resource.node.element).children(".list")[0];
        resource.node.icon = task.getIconNodes();
        
        // Members
        resource.icons = new Icons(); // Generates the SVG icons
        resource.layout = new Layout(); // Manages the layout of the columns
        resource.columns = new Columns(); // The columns of the files list
        resource.container = new List(); // The container handles the display of the files list. Each container have its own layout.
        resource.fileNumber = 0; // The number of the current file of the playlist
        resource.currentFile; // The current file of the playlist
        resource.files = new Files(); // The list of the files to display.
        
        // Default values
        resource.node.icon.content.css("padding", 5);
        resource.updateIcon();
        gl_files.bindOnAdd(resource, resource.onAdd);
        gl_files.bindOnUpdate(resource, resource.onUpdate);
        gl_files.bindOnDelete(resource, resource.onDelete);
        gl_uploads.bindOnUpload(resource, resource.onUpload);
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
        gl_files.unbindOnAdd(resource);
        gl_files.unbindOnUpdate(resource);
        gl_files.unbindOnDelete(resource);
        gl_uploads.unbindOnUpload(resource);
        resource.container.close();
        for (var key in resource)
            resource[key] = undefined;
    }

    // Updates the content of the icon based on the files displayed
    resource.updateIcon = function ()
    {
        // Total number of files
        resource.node.icon.title.html(resource.files.length + " " + T.Files.files);
        // Number of files of each type
        var filesPerType = {audio: 0, document: 0, image: 0, other: 0, video: 0} ;
        for (var i = 0; i < resource.files.length; ++i)
            filesPerType[resource.files[i].file.info.type]++;
        var content = "";
        for (var t in filesPerType)
            if (filesPerType[t])
                content += filesPerType[t] + " " + t + "<br />";
        if (content == "")
            content = T.Files.empty;
        resource.node.icon.content.html(content);
        task.updateIconHeight();
    }
    
    // Opens a file.
    resource.open = function (file)
    {
        for (var fileNumber = 0; fileNumber < resource.files.length; ++fileNumber)
            if (resource.files[fileNumber].file == file)
                break;
        resource.fileNumber = fileNumber + 1;
        resource.currentFile = file;
        // If it is an audio file we play it in the global player
        if (file.info.type == "audio")
            gl_player.openAudio(resource, resource.currentFile);
        // Otherwise we open a new page to view the file
        else
            gl_desktop.openPage("view", {playlistInterface: resource, file: file});
    }
    
    // Called when files have been added.
    resource.onAdd = function (files)
    {
        resource.files.onAdd(files);
    }
    
    // Called when files have been updated.
    resource.onUpdate = function (files)
    {
        resource.files.onUpdate(files);
    }
    
    // Called when files have been deleted.
    resource.onDelete = function (files)
    {
        resource.files.onDelete(files);
    }

    // Called while files are being uploaded.
    resource.onUpload = function ()
    {
        resource.container.onUpload();
    }

    // Converts the name into a valid class name.
    resource.encodeClassName = function (name)
    {
        return encodeURI(name).replace(/%/g, "_");
    }
    
// Playlist interface
    {
        resource.getFile = function ()
        {
            return (resource.file);
        }
        
        resource.getNumberFiles = function ()
        {
            return ({ fileNumber : resource.fileNumber,
                      numberOfFiles : resource.files.length});
        }
        
        resource.readyToPlay = function (fileInterface)
        {
            gl_player.playFile(resource, fileInterface);
        }
    }

// Manages the files. This class inherit from Array.
function Files()
{
    var self = new Array();
    
    self.init = function ()
    {
        // Represents a file in the array.
        var File =
        {
            file: {}, // The gl_files.File object
            selectedId: 0, // > 0 if the file is selected. Represents the id of the selection.
        };
        self.raw = new Array(); // The unfiltered list of the files
        self.types = new Object(); // The types that don't have to be displayed
        self.sortedColumn; // The name of the column used to sort the files
        self.filesSelected = {}; // The list of the files selected by the user
        self.nextFileSelectedId = 1; // The id of the next file selected, used to retrieve it in filesSelected
        self.lastFileSelected = { fileIndex: undefined, file: undefined }; // The last file selected or deselected
        resource.files = self;
    
        // Fills the list
        for (var i = 0; i < gl_files.length; ++i)
            self.raw.push({file: gl_files[i], selectedId: 0});
        self.applyFilters();
    }
    
    // Recreates the filtered list.
    self.applyFilters = function ()
    {
        self.length = 0;
        for (var i = 0; i < self.raw.length; ++i)
        {
            var file = self.raw[i];
            if (!self.types[file.file.info.type])
                self.push(file);
        }
        resource.container.onFilesChange();
        resource.updateIcon();
    }
    
    // The files of this type will not be displayed.
    self.addFilterType = function (type)
    {
        self.types[type] = true;
    }
    
    // Displays the files of this type.
    self.removeFilterType = function (type)
    {
        self.types[type] = undefined;
    }
    
    // Sorts the files.
    self.sort = function (column)
    {
        self.sortedColumn = column;
        if (resource.layout.isNumerical(column))
            self.raw.sort(self._compareNumber);
        else
            self.raw.sort(self._compareString);
        self.applyFilters();
    }
    
    // Reverses the list.
    self.reverse = function()
    {
        self.raw.reverse();
        self.applyFilters();
    }
    
    // Called when files have been added.
    self.onAdd = function (added)
    {
        for (var i = 0; i < added.length; ++i)
            self.raw.push({file: added[i], selectedId: 0});
        resource.files.applyFilters();
    }
    
    // Called when files have been updates.
    self.onUpdate = function (updated)
    {
        resource.files.applyFilters();
    }
    
    // Called when files have been deleted.
    self.onDelete = function (deleted)
    {
        // Removes the deleted file from raw
        for (var i = 0; i < self.raw.length; ++i)
            for (var j = deleted.length - 1; j >= 0; --j)
                if (self.raw[i].file == deleted[j])
                {
                    self.raw.splice(i--, 1);
                    break ;
                }
        
        // Removes the deleted files from the selection
        for (var i = 0; i < deleted.length; ++i)
            for (var f in self.filesSelected)
            {
                var file = self.filesSelected[f];
                if (file.file == deleted[i])
                {
                    self.deselect(file);
                    break;
                }
            }
        
        // Updates lastFileSelected
        if (self.lastFileSelected.file)
            for (var i = 0; i < deleted.length; ++i)
                if (self.lastFileSelected.file.file == deleted[i])
                {
                    self.lastFileSelected = { fileIndex: undefined, file: undefined };
                    break ;
                }
        resource.files.applyFilters();
    }
    
    // Returns the local file index that corresponds to the given file.
    self.getLocalFileId = function (file)
    {
        for (var i = 0; i < self.length; ++i)
            if (self[i] == file)
                return (i);
        return (undefined);
    }
    
    // Selection
    {
        self.select = function (file, node)
        {
            self.filesSelected[self.nextFileSelectedId] = file;
            file.selectedId = self.nextFileSelectedId;
            self.nextFileSelectedId++;
            if (node)
                $(node).addClass("selected");
        }
        
        self.deselect = function (file, node)
        {
            delete self.filesSelected[file.selectedId];
            file.selectedId = 0;
            if (node)
                $(node).removeClass("selected");
        }
        
        self.deselectAll = function (node)
        {
            for (var f in self.filesSelected)
                self.filesSelected[f].selectedId = 0;
            self.filesSelected = {};
            if (node)
                $(node).removeClass("selected");
        }
        
        // Returns true if more than one file is selected.
        self.isMultipleFilesSelected = function ()
        {
            var n = 0;
            for (var f in self.filesSelected)
                if (++n > 1)
                    return true;
            return false;
        }
        
        // Returns the list of the files selected and not filtered.
        self.getFilesSelected = function ()
        {
            var filesSelected = [];
            
            for (var f in self.filesSelected)
            {
                var file = self.filesSelected[f];
                for (var i = 0; i < self.length; ++i)
                    if (self[i] == file)
                    {
                        filesSelected.push(file.file);
                        break ;
                    }
            }
            return filesSelected;
        }
    }
    
    // Private
    {
        // Numerical column comparison.
        self._compareNumber = function (a, b)
        {
            return (parseInt(a.file.info[self.sortedColumn]) - parseInt(b.file.info[self.sortedColumn]));
        }
        
        // Alphabetical column comparison.
        self._compareString = function (i, j)
        {
            var a = i.file.info[self.sortedColumn];
            var b = j.file.info[self.sortedColumn];
            a = a ? a : "";
            b = b ? b : "";
            return a.localeCompare(b);
        }
    }
    
    self.init();
    return (self);
}

// Manages the columns.
function Columns()
{
    var self = this;
    
    self.init = function ()
    {
        // Members
        self.C = C.R.Files.Columns; // The configuration of the columns
        self.sortedColumn = 10000; // The index of the column by which the files are sorted
        self.paper; // The SVG paper on which the columns are drawn
        self.height = self.C.height; // The height of the columns
        self.slope = self.C.slopeRatio * self.height; // The slope of the columns
        self.context = new ColumnsContext(); // Manages the context menu
        self.background; // The background of the columns.
        // The list of the columns
        self.columns = new Array();
        var column = {
            left: 0, // The left position of the column
            width: 0, // The width of the column
            newWidth: 0, // The new with that is going to be applied
            background: 0, // The background of the column
            separator: 0, // The right separator
            index: 0, // The index of the column
            align: 0, // The alignment of the column (left / right)
            name: 0, // The name of the column
            text: 0, // The text node
            translation: 0, // The translated text
            shortened: 0, // True while the text is shortened
            offset: 0, // The offset applied to the text (while the column is moved)
            textWidth: 0, // The original width of the text
            currentTextWidth: 0 // The current width of the (may be shortened) text
        };
        // Stores the values used to move a column
        self.moveColumn = {
            element: 0, // The position of the columns
            mouse: 0, // The position of the mouse when the move started
            resistance: 0, // False when the resistance has been broken and the move started
            positions: 0, // The positions where the columns are swapped
            paper: 0, // A clone of the moved column, which follows the mouse
            background: 0, // The clone of the background
            target: 0, // A path that shows the space that the column will occupy
            left: 0 // The left position of the column
        }; 
    
        // Default values
        resource.node.columns.height(self.height);
        self.paper = Raphael(resource.node.columns[0], "100%", "100%");
        self.background = self.paper.rect(0, 0, 1000000, self.height).attr(self.C.backgroundAttr);
        $(self.background.node).mousedown(self.mouseDownBackground);
        self.addColumn("type");
        self.addColumn("name");
        self.addColumn("size");
        self.addColumn("created");
    }
    
    // Adds a column.
    // @param name : The name corresponds to the property of the file associated with the column.
    self.addColumn = function (name)
    {
        var c = new Object();
        
        // Coordinates
        c.left = self.C.separatorWidth + (self.columns.length ? (self.columns[self.columns.length - 1].left + self.columns[self.columns.length - 1].width) : 0);
        c.newWidth = resource.layout.getDefaultWidth(name);
        c.offset = 0;
        // Background
        c.background = self.paper.path();
        c.background.attr(self.columns.length % 2 ? self.C.evenAttr : self.C.oddAttr);
        c.background.node.setAttribute("class", "column");
        $(c.background.node).mousedown(function (e) { self.mouseDownColumn(e, c); });
        $(c.background.node).mouseup(function (e) { self.clickColumn(e, c); });
        // Text
        c.name = name;
        c.translation = self.translateName(name);
        c.align = resource.layout.getAlignment(name);
        c.text = $("<span></span>").addClass("text").appendTo(resource.node.columns);
        c.text.html(c.translation);
        c.text.mousedown(function (e) { self.mouseDownColumn(e, c); });
        c.text.mouseup(function (e) { self.clickColumn(e, c); });
        c.textWidth = c.text.width();
        c.currentTextWidth = c.textWidth;
        // Separator
        c.separator = self.paper.path();
        self.drawPath(c.separator, self.C.separatorWidth, self.columns.length);
        c.separator.attr(self.C.separatorAttr);
        c.separator.node.setAttribute("class", "separator");
        $(c.separator.node).mousedown(function (e) { self.mouseDownSeparator(e, c); });
        // Adds the column
        c.index = self.columns.length;
        self.columns.push(c);
        self.updateColumns();
        return c;
    }
    
    // Removes a column.
    self.removeColumn = function (column)
    {
        if (self.columns.length == 1)
            return ;
        if (self.sortedColumn == column.index)
            self.sortedColumn = 10000;
        // Notifies the container
        resource.container.removeColumn(column);
        // Removes the column
        column.text.remove();
        column.background.remove();
        column.separator.remove();
        self.columns.splice(column.index, 1);
        // Updates the index of the other columns
        for (var i = column.index; i < self.columns.length; ++i)
            self.columns[i].index = i;
        // Ensures that the width of the first column is updated
        if (column.index == 0)
            self.columns[0].newWidth = self.columns[0].width;
        // Updates positions of the other columns
        self.updateSeparators();
        self.updateColumns();
        delete column;
    }
    
    // Returns the list of the columns.
    self.getColumns = function ()
    {
        return (self.columns);
    }
    
    // Returns the column.
    self.getColumn = function (name)
    {
        for (var i = 0; i < self.columns.length; ++i)
            if (self.columns[i].name == name)
                return self.columns[i];
    }
    
    // Starts to move a column / displays the context menu.
    self.mouseDownColumn = function (e, column)
    {
        if (e.which == 1)
        {
            gl_desktop.drag.start(e, resource.node.columns, self, "mouseMoveColumn", "", "mouseUpColumn", column);
            gl_desktop.drag.setCursor("pointer");
            self.moveColumn.element = gl_desktop.drag.getElement();
            self.moveColumn.mouse = gl_desktop.drag.getMouse();
            self.moveColumn.resistance = true;
        }
        // Displays the context menu
        else if (e.which == 3)
            self.context.display(e, column);
    }
    
    // Moves a column
    self.mouseMoveColumn = function (e, column)
    {
        var move = self.moveColumn;
        // Starts to move the column as soon as the resistance breaks
        if (move.resistance && Math.abs(e.pageX - move.mouse.x - move.element.x) > self.C.moveResistance)
        {
            gl_desktop.drag.setCursor("move");
            move.resistance = false;
            move.mouse.x += e.pageX - move.mouse.x - move.element.x;
            // Creates a clone of the column which will move with the mouse 
            move.node = $("<div></div>").addClass("move_column").appendTo(resource.node.columns);
            move.paper = Raphael(move.node[0], "100%", "100%");
            move.target = move.paper.path();
            move.background = move.paper.path();
            self.drawPath(move.background, column.width + self.C.separatorWidth - (column.index == self.sortedColumn ? self.slope : 0), column.index, true, true);
            move.background.attr(self.C.moveAttr);
            move.target.attr(move.background.attr());
            move.target.attr(self.C.moveBackgroundAttr);
            move.target.transform("T" + Math.floor(column.left - (column.index > 0 ? self.C.separatorWidth / 2 : 0)) + ",0");
            move.left = column.left;
            // Saves the current positions of the columns
            self.moveColumn.positions = new Array();
            for (var i = 0; i < self.columns.length; ++i)
                if (i != column.index)
                    self.moveColumn.positions.push({
                        left: self.columns[i].left + (self.columns[i].width + self.C.separatorWidth) / 2,
                        column: i
                    });
            // Updates the actual column
            column.text.addClass("move_text");
            column.background.attr(self.C.moveBackgroundAttr);
            resource.container.moveStart(column.name);
        }
        // Moves the column
        if (!move.resistance)
        {
            column.offset = e.pageX - move.mouse.x - move.element.x;
            var left = move.left + column.offset;
            // Moves the actual column
            if (column.offset > 0)
                left += column.width + self.C.separatorWidth;
            for (var destination = 0; destination < move.positions.length && move.positions[destination].left < left; ++destination)
                ;
            // Changes the order of the columns
            if (destination != column.index)
            {
                resource.container.moveColumn(column.index, destination);
                self.columns.splice(column.index, 1);
                self.columns.splice(destination, 0, column);
                var sortedColumn = self.sortedColumn;
                for (var j = 0; j < self.columns.length; ++j)
                {
                    if (self.columns[j].index == sortedColumn)
                        self.sortedColumn = j;
                    self.columns[j].index = j;
                }
                self.updateSeparators();
                // Updates the paths of the column moved
                self.drawPath(move.background, column.width + self.C.separatorWidth - (column.index == self.sortedColumn ? self.slope : 0), column.index, true, true);
                move.target.attr(move.background.attr());
                move.target.attr(self.C.moveBackgroundAttr);
                column.background.attr(self.C.moveBackgroundAttr);
                var updateTargetPosition = true;
            }
            // Updates the other columns
            column.left = -1;
            self.updateColumns();
            // Moves the clone of the column to follow the position of the mouse
            var left = move.left + column.offset - Math.floor(column.index > 0 ? self.C.separatorWidth / 2 : 0);
            move.background.transform("T" + left + ",0");
            column.text.css("left", column.text.position().left + move.left - column.left);
            resource.container.moveUpdate(column.offset + move.left - column.left);
            // Moves the target that shows where the column will be moved
            if (updateTargetPosition)
                move.target.transform("T" + Math.floor(column.left - (column.index > 0 ? self.C.separatorWidth / 2 : 0)) + ",0");
        }
    }
    
    // Sorts the column, or stops its movement.
    self.mouseUpColumn = function (e, column)
    {
        if (e.which != 1)
            return ;
        // Sorts the column
        if (self.moveColumn.resistance)
        {
            // If the column is already sorted, we just reverse the files list
            if (self.sortedColumn == column.index)
            {
                resource.files.reverse();
                self.slope = -self.slope;
            }
            // Performs the actual sorting
            else
            {
                resource.files.sort(column.name);
                self.slope = self.C.slopeRatio * self.height;
            }
            self.sortedColumn = column.index;
            // Updates the paths of the columns
            self.updateSeparators();
            self.updateColumns();
        }
        // Stops the column movement
        else
        {
            column.background.attr(column.index % 2 ? self.C.evenAttr : self.C.oddAttr);
            if (column.index == self.sortedColumn)
                column.background.attr(self.C.sortAttr);
            self.moveColumn.background.remove();
            self.moveColumn.target.remove();
            self.moveColumn.paper.remove();
            self.moveColumn.node.remove();
            self.moveColumn = new Object();
            column.offset = 0;
            column.text.removeClass("move_text");
            column.left = -1;
            self.updateColumns();
            resource.container.moveStop();
        }
    }
    
    // Removes a column.
    self.clickColumn = function (e, column)
    {
        if (e.which == 2)
            self.removeColumn(column);
    }
    
    // Resizes a column / displays the context menu.
    self.mouseDownSeparator = function (e, column)
    {
        // Starts to resize the column.
        if (e.which == 1)
        {
            var param = new Object();
            param.column = column;
            // Saves the width of the previous columns
            param.width = new Array();
            for (var i = column.index; i >= 0; --i)
                param.width.push(self.columns[i].width);
            gl_desktop.drag.start(e, e.currentTarget, self, "mouseMoveSeparator", undefined, "mouseUpSeparator", param);
            gl_desktop.drag.setCursor("e-resize");
        }
        // Displays the context menu
        else if (e.which == 3)
            self.context.displaySeparator(e, column);
    }
    
    // Resizes the column.
    self.mouseMoveSeparator = function (e, p)
    {
        var mouse = gl_desktop.drag.getMouse();
        var element = gl_desktop.drag.getElement();
        var diff = e.pageX - (mouse.x + element.x);
        
        // Updates the width of the columns
        var column = p.column;
        var flag = true;
        for (var i = 0; p.width.length > i && flag; ++i, column = self.columns[column.index - 1])
        {
            var width = diff + p.width[i];
            var minWidth = resource.layout.getMinWidth(column.name);
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
            column.newWidth = width;
            resource.layout.onResize(column.name, width);
        }
        column = p.column;
        // Ensures that the previous column have their original width
        if (i < p.i)
            for (var j = 0; p.width.length > j; ++j, column = self.columns[column.index - 1])
                if (j >= i)
                    column.newWidth = p.width[j];
        p.i = i;
        self.updateColumns();
        resource.container.updateColumns();
    }

    // Resizes the column.
    self.mouseUpSeparator = function ()
    {
        task.updateMouseOverContent();
    }
    
    // Updates the separators based on the sorted column.
    // updateColumns() have to be called after.
    self.updateSeparators = function ()
    {
        for (var i = 0; i < self.columns.length; ++i)
        {
            var c = self.columns[i];
            // The separators adjacent to the sorted column have a special attribute
            if (i == self.sortedColumn || i == self.sortedColumn - 1)
                c.separator.attr(self.C.sortSeparatorAttr);
            else
                c.separator.attr(self.C.separatorAttr);
            // Updates the odd / even attributes of the backgrounds
            if (i == self.sortedColumn)
                c.background.attr(self.C.sortAttr);
            else
                c.background.attr(i % 2 ? self.C.evenAttr : self.C.oddAttr);
            // Updates the path of the saparators
            self.drawPath(c.separator, self.C.separatorWidth, c.index);
            // Ensures that the column will be redrawn by updateColumns
            c.newWidth = c.width;
        }
    }
    
    // Updates the position and the width of the columns.
    self.updateColumns = function ()
    {
        var left = 0;
        for (var i = 0; i < self.columns.length; ++i)
        {
            var column = self.columns[i];
            
            // The width of the column has changed
            if (column.newWidth != undefined)
            {
                // Shortens the text if it is bigger than the column
                var diff = column.newWidth - column.textWidth - self.C.columnMargin - self.slope / 3;
                if ((diff < 0 || column.shortened) && column.newWidth != column.width)
                {
                    var text = column.translation;
                    var numberLetters = (1 + diff / column.textWidth) * text.length;
                    // Not enough space to display anything
                    if (numberLetters < 1)
                        column.text.html(text = "");
                    // Displays as much text as possible
                    else
                    {
                        text = text.substr(0, numberLetters);
                        column.text.html(text + (text.length > 1 ? self.C.shortenString : ""));
                        // Ensures that the approximation is correct
                        while (column.newWidth - column.text.width() - self.C.columnMargin - self.slope / 3 < 0 && text.length)
                            column.text.html((text = text.substr(0, text.length - 1)) + (text.length > 1 ? self.C.shortenString : ""));
                    }
                    // The text can be fully displayed
                    if (text.length == column.translation.length)
                    {
                        column.shortened = false;
                        column.text.html(column.translation);
                    }
                    else
                        column.shortened = true;
                    column.currentTextWidth = column.text.width();
                }
                // Updates the width of the column
                column.width = column.newWidth;
                self.drawPath(column.background, column.width, column.index, true);
                delete column.newWidth;
                column.left = -1;
            }
            // The left position of the column has changed
            if (left != column.left)
            {
                column.left = left;
                if (column.align != "right")
                    column.text.css("left", column.left + self.C.separatorWidth / 2 + self.C.columnMargin - self.C.separatorWidth / 2 + column.offset);
                // Right align the text
                else
                    column.text.css("left", column.left + column.width + self.C.separatorWidth - column.currentTextWidth - self.C.separatorWidth / 2 - self.C.columnMargin - self.C.separatorWidth / 2 + column.offset);
                var shiftFirstColumn = (i == 0 ? Math.abs(self.slope / 2) : 0);
                var slope = self.slope * (column.index > self.sortedColumn ? -1 : 1);
                column.background.transform("T" + (left - slope / 2 - shiftFirstColumn) + ",0");
                slope = self.slope * (column.index >= self.sortedColumn ? -1 : 1);
                column.separator.transform("T" + (left - slope / 2 + column.width) + ",0");
            }
            left += column.width + self.C.separatorWidth;
        }
    }
    
    // Displays the context menu.
    self.mouseDownBackground = function (e)
    {
        if (e.which == 3)
            self.context.display(e);
    }
    
    // Translates the name of the column.
    self.translateName = function (name)
    {
        var translation = T.Files.Columns[name];
        if (translation)
            return translation;
        return name.charAt(0).toUpperCase() + name.slice(1);
    }
    
    // Updates the horizontal scroll of the columns.
    self.scrollHorizontal = function (left)
    {
        resource.node.columns.css("left", -left);
    }
    
    // Adjusts the size of the column to the minimum size of its content.
    self.adjustColumn = function (column)
    {
        // Gets the minimum width necessary to display all the elements of the column
        var oldWidth = column.width;
        var width = column.textWidth;
        if (!resource.layout.isStaticMinWidth(column.name))
        {
            var containerMinWidth;
            if (resource.container.getMinWidth)
                width = Math.max(width, containerMinWidth = resource.container.getMinWidth(column));
            width = Math.max(width + self.C.columnMargin * 2, resource.layout.getMinWidth(column.name));
            // If the column was already adjusted, we don't take the column name into account this time
            if (width == oldWidth && containerMinWidth)
                width = Math.max(containerMinWidth + self.C.columnMargin * 2, resource.layout.getMinWidth(column.name));
        }
        else
            width = resource.layout.getMinWidth(column.name);
        // Makes sure the column name is not shortened
        if (width == column.textWidth + self.C.columnMargin * 2)
        {
            column.shortened = false;
            column.text.html(column.translation);
        }
        
        // Updates the width of the column
        column.newWidth = width;
        resource.layout.onResize(column.name, width);
        self.updateColumns();
        resource.container.updateColumns();
    }
    
    // Adjusts the size of all the columns to the minimum size of their contents.
    self.adjustAllColumns = function ()
    {
        for (var i = 0; i < self.columns.length; ++i)
            self.adjustColumn(self.columns[i]);
    }
    
    // Draws a background or a separator.
    // @param path: The Raphael target path.
    // @param width: The width of the path.
    // @param index: The index of the column drawn.
    // @param background: True for a background and false for a separator.
    // @param exactWidth: True if the width should not be modified.
    self.drawPath = function (path, width, index, background, exactWidth)
    {
        // Changes the direction of the slope after the sorted column
        var slope = self.slope;
        if (index > self.sortedColumn || (index == self.sortedColumn && (!background || index == 0)))
            slope = -self.slope;
        width += slope;
        // Ensures that the backgrounds and the separator overlap correctly (closes the gap between them)
        width += (!exactWidth || index == 0 ? 1 : 0);
        // The first column has a vertical left side
        if (background && index == 0)
            path.attr("path", "M0," + self.height + "L0 0,H" + (width + Math.abs(self.slope / 2) * (!exactWidth ? 1 : -1) - (index == self.sortedColumn ? slope : 0)) + ",l" + -slope + " " + self.height + "z");
        // Normal column
        else if (index != self.sortedColumn || !background)
            path.attr("path", "M0," + self.height + "L" + slope + " 0,H" + width + ",l" + -slope + " " + self.height + "z");
        // The sorted column background is a frustum
        else 
            path.attr("path", "M0," + self.height + "L" + slope + " 0,H" + (width - slope) + ",l" + slope + " " + self.height + "z");
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
        self.oldTableLength; // The length of the table before its last modification
        self.filesContext = new FilesContext(); // Manages the context menu of the files
        // Allows to move a column
        self.move = {
            updateColumns: 0, // True if the columns have to be updates the next time moveUpdate is called
            style: 0, // CSS rule used to move the column
            sheet: 0 // The style sheet node that contains the CSS rule
        }
        
        // Creates the CSS sheet that manage the uploads
        self.uploadsCss = new Object();
        self.uploadsCss.sheet = $("<style></style>").appendTo($("head"));
        var css = self.uploadsCss.sheet[0].sheet;
        css.insertRule("#tasks>#" + task.getContentId() + ">.files>.list>table tr.upload td:last-child>div.progress { }", css.length);
        self.uploadsCss.style = css.cssRules[0].style;
        self.uploadsCss.style.width = "100px";
        
        // Events
        $(self.table).mousedown(self.mouseDown);
        $(self.table).dblclick(self.dblClick);
        $(self.list).scroll(self.scroll);
        
        self.updateColumns();
    }
    
    // Closes the list.
    self.close = function ()
    {
        self.uploadsCss.sheet.remove();
        for (var key in self)
            resource[key] = undefined;
    }
    
    // Updates the width of the columns.
    self.updateColumns = function ()
    {
        var columns = resource.columns.getColumns();
        var colgroup = $(self.table).children("colgroup");
        var cols = colgroup.children("col");
        for (var i = 0; i < columns.length; ++i)
        {
            var col;
            // The columns doesn't exists yet
            if (cols.length <= i)
            {
                col = $("<col></col>");
                col[0].name = columns[i].name;
                col.appendTo(colgroup);
            }
            else
                col = $(cols[i]);
            // Updates the width
            col.width(columns[i].width + C.R.Files.Columns.separatorWidth - (i == 0 ? C.R.Files.Columns.separatorWidth / 2 : 0));
        }
    }
    
    // Adds a column to the list.
    self.addColumn = function (column)
    {
        // Creates the <col>
        self.updateColumns();
        var firstRowFileNumber = Math.floor(self.top.height() / C.R.Files.listRowHeight);
        var rows = self.table.rows;
        // Adds the cell to the row
        for (var i = 0; i < rows.length; ++i)
        {
            var row = rows[i];
            $(row.insertCell(column.index)).addClass(resource.encodeClassName(column.name));
            if (!$(row).hasClass("empty"))
            {
                var cell = row.cells[column.index];
                cell.originalText = resource.layout.convert(column.name, resource.files[firstRowFileNumber + i].file.info[column.name]);
                cell.innerHTML = cell.originalText;
                if (column.align != "left")
                    cell.style.textAlign = column.align;
            }
        }
    }
    
    // Removes a column from the list.
    self.removeColumn = function (column)
    {
        // Removes the col element
        var colgroup = $(self.table).children("colgroup");
        var cols = colgroup.children("col");
        $(cols[column.index]).remove();
        // Removes the column
        var rows = self.table.rows;
        for (var i = 0; i < rows.length; ++i)
            $(rows[i].cells[column.index]).remove();
    }
    
    // Adds / removes a file from the selection, depending if the keys shift or ctrl are pressed.
    self.mouseDown = function (e)
    {
        if (e.which != 1 && e.which != 3)
            return ;
        var rightButton = (e.which == 3);
        // Gets the file selected by the user
        var row = e.target;
        for (var i = 0; i < 3 && row.tagName.toLowerCase() != "tr"; ++i, row = row.parentNode)
            ;
        // If the event is directly on the table we use an other way to get the selected file
        if (e.target == self.table)
        {
            var n = Math.floor(((e.pageY - $(self.table).offset().top) / C.R.Files.listRowHeight));
            if (n < self.table.rows.length)
                row = self.table.rows[n];
            else
                return ;
        }
        // The row is empty
        if ($(row).hasClass("empty"))
        {
            if (e.ctrlKey == false && e.shiftKey == false)
                resource.files.deselectAll(self.table.rows);
            if (rightButton)
                self.filesContext.display(e);
            return ;
        }
        var fileIndex = row.fileIndex;
        var file = resource.files[fileIndex];
        // Selects or deselects one file
        if (e.ctrlKey == true && e.shiftKey == false)
        {
            // Selects the file
            if (!file.selectedId)
                resource.files.select(file, row);
            // Deselects the file
            else if (!rightButton || resource.files.isMultipleFilesSelected())
                resource.files.deselect(file, row);
        }
        // Selects / deselects multiple files
        else if (e.shiftKey == true)
        {
            // Deselects all the files when ctrl is not pressed
            if (e.ctrlKey == false)
                resource.files.deselectAll(self.table.rows);
            // Selects all the files between the selected file and the last file selected
            if (resource.files.lastFileSelected.fileIndex === undefined)
                if (!resource.files.lastFileSelected.file || (resource.files.lastFileSelected.fileIndex = resource.files.getLocalFileId(resource.files.lastFileSelected.file)) === undefined)
                    resource.files.lastFileSelected = { fileIndex: row.fileIndex, file: file };
            var start = row.fileIndex;
            var end = resource.files.lastFileSelected.fileIndex;
            var increment = (start > end ? -1 : 1);
            for (var i = 0; i <= Math.abs(start - end); i++)
            {
                var rowIndex = row.rowIndex + i * increment;
                if (rowIndex >= 0 && rowIndex < self.table.rows.length)
                    $(self.table.rows[rowIndex]).addClass("selected");
                resource.files.select(resource.files[row.fileIndex + i * increment]);
            }
            if (rightButton)
                self.filesContext.display(e);
            return ;
        }
        // Selects the file if it was not selected (right button)
        else if (rightButton)
        {
            if (!$(row).hasClass("selected"))
            {
                resource.files.deselectAll(self.table.rows);
                resource.files.select(file, row);
            }
        }
        // Selects / Deselects one file
        else
        {
            var fileSelected = $(row).hasClass("selected");
            var multipleFilesSelected = resource.files.isMultipleFilesSelected();
            // Deselects all the files
            resource.files.deselectAll(self.table.rows);
            // Selects the file if it was not selected
            if (!fileSelected || multipleFilesSelected)
                resource.files.select(file, row);
        }
        resource.files.lastFileSelected = { fileIndex: fileIndex, file: file };
        if (rightButton)
            self.filesContext.display(e);
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
            var n = Math.floor(((e.pageY - $(self.table).offset().top) / C.R.Files.listRowHeight));
            if (n >= self.table.rows.length)
                return ;
            file = self.table.rows[n];
        }
        if ($(file).hasClass("empty"))
            return ;
        var firstRowFileNumber = Math.floor(self.top.height() / C.R.Files.listRowHeight);
        resource.open(resource.files[file.fileIndex].file);
    }
    
    // Scrolls the file list.
    self.scroll = function ()
    {
        // Computes the top padding that simutates the scroll
        var top = Math.floor(self.list.scrollTop / C.R.Files.listRowHeight) * C.R.Files.listRowHeight;
        top = Math.max(0, Math.min(top, (resource.files.length - self.table.rows.length) * C.R.Files.listRowHeight));
        
        // Update the file list if needed
        if (top != self.top.height() || self.table.rows.length != self.oldTableLength)
        {
            var bottom = Math.max(Math.floor((resource.files.length * C.R.Files.listRowHeight - self.listHeight - top) / C.R.Files.listRowHeight - 1) * C.R.Files.listRowHeight, 0);
            self.top.height(top);
            self.bottom.height(bottom);
            
            var firstFileIndex = Math.floor(self.top.height() / C.R.Files.listRowHeight);
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
        // Updates the horizontal scroll of the columns
        resource.columns.scrollHorizontal(self.list.scrollLeft);
        // Updates the uploads css
        self.uploadsCss.style.right = (self.table.offsetWidth - self.top.width() - self.list.scrollLeft) + "px";
    }
    
    // Creates / removes enough rows to fill the visible part of the list.
    self.updateRows = function ()
    {
        var numberRows = Math.max(Math.ceil(self.listHeight / C.R.Files.listRowHeight) + 1, 1);
        var columns = resource.columns.getColumns();
        var j = 0;
        
        // There is enough rows to display all the files
        if (resource.files.length <= numberRows - 1)
            numberRows--;
        // Adds the necessary rows
        if (self.table.rows.length < numberRows)
            while (self.table.rows.length < numberRows)
            {
                var row = self.table.insertRow(-1);
                // Creates the content of the row
                for (var i = 0; i < columns.length; ++i)
                    $(row.insertCell(-1)).addClass(resource.encodeClassName(columns[i].name));
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
                    self.top.height(self.top.height() + C.R.Files.listRowHeight);
                    self.table.deleteRow(0);
                }
                else
                    self.table.deleteRow(-1);
        }
    }
    
    // Fills the row with the given file informations.
    // @param fileIndex : The file to put in the row (can be out of range).
    // @param row : The row to fill.
    self.setFileInRow = function (fileIndex, row)
    {
        var columns = $(row).children();
        var headerColumns = resource.columns.getColumns();
        
        row.className = (fileIndex % 2 ? "even" : "odd");
        // Sets the file to the row
        if (fileIndex < resource.files.length)
        {
            var file = resource.files[fileIndex];
            row.fileIndex = fileIndex;
            if (file.selectedId)
                $(row).addClass("selected");
            $(row).addClass(file.file.info.type);
            for (var i = 0; i < headerColumns.length; ++i)
            {
                var column = columns[i];
                column.originalText = resource.layout.convert(headerColumns[i].name, file.file.info[headerColumns[i].name]);
                column.innerHTML = column.originalText;
                if (headerColumns[i].align != "left")
                    column.style.textAlign = headerColumns[i].align;
            }
            // The file is being uploaded
            if (file.file.upload || !file.file.info.id)
            {
                $(row).addClass("upload");
                columns[columns.length - 1].innerHTML = "";
                var progress = $('<div class="progress" />').appendTo(columns[columns.length - 1]);
                progress.height(C.R.Files.listRowHeight);
                var upload = file.file.upload;
                $('<div />').appendTo(progress).width((upload ? upload.percent : 0) + "%");
            }
        }
        // Cleans the row
        else if (!$(row).hasClass("empty"))
        {
            delete row.fileIndex;
            for (var i = 0; i < headerColumns.length; ++i)
            {
                var column = columns[i];
                delete column.originalText;
                column.innerHTML = "<span />";
                column.style.textAlign = "";
            }
            $(row).addClass("empty");
        }
    }
    
    // Returns the minimum width necessary to display all the elements of the column.
    self.getMinWidth = function (column)
    {
        var rows = self.table.rows;
        var minWidth = Math.max(0, $(rows[0].cells[column.index].firstChild).width());
        for (var i = 1; i < rows.length; ++i)
            minWidth = Math.max(minWidth, $(rows[i].cells[column.index].firstChild).width());
        return minWidth;
    }
    
    // Called while files are being uploaded.
    self.onUpload = function ()
    {
        for (var i = 0; i < self.table.rows.length && i < resource.files.length; ++i)
        {
            var row = self.table.rows[i];
            var upload = resource.files[row.fileIndex].file.upload;
            if (upload)
            {
                // Updates the progress bar
                if (!upload.finished)
                    $(row.cells[row.cells.length - 1].firstChild.firstChild).css("width", upload.percent + "%");
                // Ends the upload
                else
                {
                    $(row).removeClass("upload");
                    $(row).addClass("uploaded");
                    $(row).bind("mousemove", function (e) {
                        $(e.currentTarget).removeClass("uploaded");
                        $(e.currentTarget).unbind("mousemove");
                    });
                }
            }
        }
    }
    
    // Moves a column
    {
        self.moveStart = function (name)
        {
            self.move = new Object();
            self.move.updateColumns = false;
            self.move.sheet = $("<style></style>").appendTo($("head"));
            var css = self.move.sheet[0].sheet;
            css.insertRule("#tasks>.task>.files>.list>table td." + resource.encodeClassName(name) + "{ }", css.length);
            self.move.style = css.cssRules[0].style;
            self.move.style.position = "relative";
            self.move.style.left = "0px";
            self.move.style.opacity = "0.9";
            $(self.table).addClass("move_column");
        }
        
        self.moveUpdate = function (offset)
        {
            self.move.style.left = offset + "px";
            if (self.move.updateColumns)
                self.updateColumns();
        }
        
        self.moveColumn = function (source, destination)
        {
            // Moves the col element
            var cols = $(self.table).children("colgroup").children("col");
            if (source < destination)
                $(cols[source]).insertAfter(cols[destination]);
            else
                $(cols[source]).insertBefore(cols[destination]);
            // Moves the column
            var rows = self.table.rows;
            var cells;
            for (var i = 0; i < rows.length; ++i)
            {
                cells = rows[i].cells;
                if (source < destination)
                    $(cells[source]).insertAfter(cells[destination]);
                else
                    $(cells[source]).insertBefore(cells[destination]);
            }
            self.move.updateColumns = true;
            // Resets the style for Firefox
            self.move.style.left = "";
        }
        
        self.moveStop = function ()
        {
            self.move.sheet.remove();
            $(self.table).removeClass("move_column");
        }
    }
    
    // Container API
    {
        // Resizes the list to fit into the task content height.
        self.onResize = function (left, top, width, height)
        {
            self.listHeight = height - C.R.Files.controlsHeight - C.R.Files.Columns.height;
            self.list.style.height = self.listHeight + "px";
            // Updates the files list
            self.updateRows();
            self.scroll();
            // Hides the scroll when all the files can be seen at the same time
            if (self.listHeight / C.R.Files.listRowHeight >= resource.files.length)
            {
                $(self.list).removeClass("scroll");
                self.list.scrollTop = 0;
            }
            else
                $(self.list).addClass("scroll");
            // Updates the uploads css
            self.uploadsCss.style.width = self.top.width() + "px";
            self.uploadsCss.style.right = (self.table.offsetWidth - self.top.width() - self.list.scrollLeft) + "px";
        }
        
        // The files have been sorted or filtered, so we have to update the displayed rows.
        self.onFilesChange = function (column)
        {
            // The file index is no longer valid
            resource.files.lastFileSelected.fileIndex = undefined;
            // Updates the scroll padding
            var top = Math.floor(self.list.scrollTop / C.R.Files.listRowHeight) * C.R.Files.listRowHeight;
            top = Math.max(0, Math.min(top, (resource.files.length - self.table.rows.length) * C.R.Files.listRowHeight));
            var bottom = Math.max(Math.floor((resource.files.length * C.R.Files.listRowHeight - self.listHeight - top) / C.R.Files.listRowHeight - 1) * C.R.Files.listRowHeight, 0);
            self.top.height(top);
            self.bottom.height(bottom);
            self.updateRows();
            // Hides the scroll if all the files can be seen at the same time
            if (self.listHeight / C.R.Files.listRowHeight >= resource.files.length)
            {
                $(self.list).removeClass("scroll");
                self.list.scrollTop = 0;
            }
            else
                $(self.list).addClass("scroll");
            // Updates the content of the rows
            var firstRowFileNumber = Math.floor(self.top.height() / C.R.Files.listRowHeight);
            for (var i = 0; i < self.table.rows.length; ++i)
                self.setFileInRow(firstRowFileNumber + i, self.table.rows[i]);
        }
        
        // Hides the files that have the given type.
        // If type is empty, all the files are displayed back.
        self.filterTypesPreview = function (type)
        {
            if (type)
                $(resource.node.list).addClass("filter_" + type);
            else
                for (var type in resource.icons.icons.types)
                    $(resource.node.list).removeClass("filter_" + type);
        }
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
        if (value == undefined)
            return "";
        var result;
        if (self.columns[key] && self.columns[key].convert)
        {
            if (self[self.columns[key].convert])
                result = self[self.columns[key].convert](value);
            else if (F[self.columns[key].convert])
                result = F[self.columns[key].convert](value);
            if (!self.columns[key].noSpan)
                result = "<span>" + result + "</span>";
        }
        else
            result = "<span>" + value + "</span>";
        return (result);
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
        if (self.columns[column] && self.columns[column].minWidth != undefined)
            return (self.columns[column].minWidth);
        return (C.R.Files.Columns.defaultMinWidth);
    }
    
    // Returns the min-width of the column.
    self.getDefaultWidth = function (column)
    {
        if (self.columns[column] && self.columns[column].defaultWidth != undefined)
            return (self.columns[column].defaultWidth);
        return (C.R.Files.Columns.defaultWidth);
    }

    // Returns true if the column contains numerical values.
    self.isNumerical = function (column)
    {
        return (self.columns[column] && self.columns[column].numerical)
    }

    // Returns true if the column has a static minimum width.
    self.isStaticMinWidth = function (column)
    {
        return (self.columns[column] && self.columns[column].staticMinWidth === true);
    }
    
    // Converts the generic type name (image audio video document other).
    self.convertType = function (type)
    {
        return ("<span>" + T.Files[type] + "</span><div></div>");
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
            if (width > C.R.Files.Columns.typeTextMinWidth)
            {
                if (!isTypeText)
                    table.addClass("type_text");
            }
            else if (isTypeText)
                table.removeClass("type_text");
            self.columns[column]
        }
    }
    
    // Stores the layout of the columns.
    self.columns = 
    {
        type:
        {
            minWidth: 0,
            defaultWidth: 1,
            convert: "convertType",
            noSpan: true,
            staticMinWidth: true,
        },
        name:
        {
            defaultWidth: 300,
        },
        size:
        {
            convert: "sizeToString",
            alignment: "right",
            minWidth: 60,
            defaultWidth: 80,
            numerical: true,
        },
        created:
        {
            convert: "convertDate",
            minWidth: 80,
            defaultWidth: 130,
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
            self.typeIcons($(resource.node.types).children("." + type)[0], type);
        for (var control in self.icons.controls)
            self.controlIcons($(resource.node.other).children("." + control)[0], self.icons.controls[control]);
    }
    
    // Generates the type icons.
    self.typeIcons = function (destination, type)
    {
        var icon = self.icons.types[type];
        var height = C.R.Files.controlsHeight;
        var paper = Raphael(destination, icon.width, height);
        var path = paper.path(icon.path);
        path.attr("fill", self.icons.type.selected_color);
        path.attr("stroke", "none");
        path.translate(icon.width / 2, height / 2);
        var line = paper.rect(0, height - 5, icon.width, 5);
        line.attr("fill", icon.line);
        line.attr("stroke", "none");
        line.hide();
        
        // Associates the node with the SVG
        var object = new Object();
        destination.object = object;
        object.isSelected = true; // True is the type is selected
        object.type = type; // The name of the type
        // Selects / deselects the type
        object.selectType = function (select)
        {
            object.isSelected = select;
            path.translate(-icon.width / 2, - height / 2);
            if (select)
            {
                path.attr("fill", self.icons.type.selected_color);
                resource.files.removeFilterType(type);
            }
            else
            {
                path.attr("fill", self.icons.type.deselected_color);
                resource.files.addFilterType(type);
            }
            path.translate(icon.width / 2, height / 2);
        };
        
        // Events
        var types = $(resource.node.types).children();
        $(destination).mouseenter(function ()
        {
            var selected = 0;
            types.each(function () { if (this.object.isSelected) selected++; });
            if (selected == types.length)
                types.not(destination).each(function () { resource.container.filterTypesPreview(this.object.type); });
            else if (selected > 1 || !object.isSelected)
                resource.container.filterTypesPreview(type);
            line.show();
        });
        $(destination).mouseleave(function ()
        {
            line.hide();
            resource.container.filterTypesPreview();
        });
        $(destination).mousedown(function ()
        {
            var selected = 0;
            types.each(function () { if (this.object.isSelected) selected++; });
            // If all the types are selected, we deselect all the other types
            if (selected == types.length)
                types.not(destination).each(function () { this.object.selectType(false); });
            else
            {
                selected = 0;
                object.isSelected = !object.isSelected;
                types.each(function () { if (this.object.isSelected) selected++; });
                // If no type is selected, we select all the other types
                if (selected == 0)
                {
                    resource.container.filterTypesPreview();
                    types.not(destination).each(function () { this.object.selectType(true); resource.container.filterTypesPreview(this.object.type); });
                    object.isSelected = true;
                }
                else
                {
                    object.selectType(object.isSelected);
                    // If all types are now selected, we display the filter preview of all the other types
                    if (selected == types.length)
                    {
                        resource.container.filterTypesPreview();
                        types.not(destination).each(function () { resource.container.filterTypesPreview(this.object.type); });
                    }
                }
            }
            resource.files.applyFilters();
        });
    }
    
    // Generates the control icons.
    self.controlIcons = function (destination, icon)
    {
        var height = C.R.Files.controlsHeight;
        var path = Raphael(destination, icon.width, height).path(icon.path);
        var color = "90-#606a76-#888f98";
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
    
    // Generates the columns icon.
    self.columnsIcon = function (destination, icon)
    {
        var height = C.R.Files.Columns.height;
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
        types:
        {
            image:
            {
                path: gl_svg.ResourceFiles.image,
                width: 47,
                line: "#33bbff",
            },
            audio: 
            {
                path: gl_svg.ResourceFiles.audio,
                width: 40,
                line: "#46da57",
            },
            video: 
            {
                path: gl_svg.ResourceFiles.video,
                width: 41,
                line: "#ff7733",
            },
            document: 
            {
                path: gl_svg.ResourceFiles.document,
                width: 36,
                line: "#ffc000",
            },
            other: 
            {
                path: gl_svg.ResourceFiles.other,
                width: 40,
                line: "#808080",
            },
        },
        type:
        {
            selected_color: "90-#606a76-#888f98",
            deselected_color: "90-#b1bac5-#c7cdd4",
        },
        controls:
        {
            folder: 
            {
                path: gl_svg.ResourceFiles.folder,
                width: 18,
            },
            resize: 
            {
                path: gl_svg.ResourceFiles.resize,
                width: 14,
            },
            list: 
            {
                path: gl_svg.ResourceFiles.list,
                width: 18,
            },
            hierarchy: 
            {
                path: gl_svg.ResourceFiles.hierarchy,
                width: 18,
            },
            split: 
            {
                path: gl_svg.ResourceFiles.split,
                width: 18,
            },
            block: 
            {
                path: gl_svg.ResourceFiles.block,
                width: 18,
            },
        },
        add: 
        {
            path: gl_svg.ResourceFiles.add,
            width: 20,
        },
    };

    self.init();
    return (self);
}

// Manages the context menu of the files.
function FilesContext()
{
    var self = this;
    
    self.init = function ()
    {
    }
    
    // Displays the context menu.
    self.display = function (e)
    {
        // Creates the actions
        var add;
        var numberFilesSelected = resource.files.getFilesSelected().length;
        var actions =
            [ gl_context.createAction(T.Files.Context.open, self.open)
            , gl_context.createAction(T.Files.Context.rename, self.rename)
            , gl_context.createAction(T.Files.Context.delete + (numberFilesSelected > 1 ? " " + numberFilesSelected : ""), self.delete)];
        if (numberFilesSelected == 0)
            actions = [];
        if (numberFilesSelected <= 1)
            actions.push(add = gl_context.createAction(T.Files.Context.add).addClass("files_add"));
        gl_context.display(e.pageX, e.pageY, actions);
        
        // Creates the input of the add action
        if (add)
        {
            add.width(add.width());
            add.height(add.height());
            add.css("position", "relative");
            var input = $('<input type="file" name="upload" multiple="true" />');
            add.append(input);
            input.change(function (e)
            {
                gl_uploads.add(this.files);
            });
        }
    }
    
    // Plays the selected files.
    self.open = function ()
    {
        var filesSelected = resource.files.getFilesSelected();
        
        if (filesSelected.length == 0)
            console.log("No file selected");
        else if (filesSelected.length == 1)
            resource.open(filesSelected[0]);
        else
            resource.open(filesSelected[0]);
    }
    
    // Renames the selected files.
    self.rename = function ()
    {
        console.log("Rename");
    }
    
    // Deletes the selected files.
    self.delete = function ()
    {
        gl_files.delete(resource.files.getFilesSelected());
    }
    
    self.init();
    return (self);
}

// Manages the context menu of the columns.
function ColumnsContext()
{
    var self = this;
    
    self.init = function ()
    {
    }
    
    // Displays the context menu.
    self.display = function (e, currentColumn)
    {
        var actions = [];
        var action;
        var columns = {};
        
        // Gets all the columns names
        for (var i = 0; i < resource.files.length; ++i)
        {
            var file = resource.files[i].file.info;
            for (var column in file)
                if (!columns[column])
                    columns[column] = true;
        }
        
        // Adds the current column to the action
        if (currentColumn)
        {
            delete columns[currentColumn.name];
            actions.push(action = gl_context.createAction(currentColumn.translation, self.select, currentColumn.name, C.R.Files.Columns.actionHideContext));
            action.addClass("selected");
        }
        // Then adds the columns displayed, from left to right
        for (var i = 0; i < resource.columns.columns.length; ++i)
        {
            var column = resource.columns.columns[i];
            if (currentColumn && currentColumn.name == column.name)
                continue;
            delete columns[column.name];
            actions.push(action = gl_context.createAction(column.translation, self.select, column.name, C.R.Files.Columns.actionHideContext));
            action.addClass("selected");
        }
        // And finally adds the rest if the columns of the files filtered, sorted by name
        var c = [];
        for (var column in columns)
            c.push({name: column, translation: resource.columns.translateName(column)});
        c.sort(function (a,b) { return a.translation.localeCompare(b.translation); });
        for (var i = 0; i < c.length; ++i)
            actions.push(gl_context.createAction(c[i].translation, self.select, c[i].name, C.R.Files.Columns.actionHideContext));
        
        // Displays the context menu
        gl_context.display(e.pageX, e.pageY, actions, true);
        gl_context.getRoot().addClass("files_columns");
    }
    
    // Selects / unselects a column.
    self.select = function (name, action)
    {
        if (action.hasClass("selected"))
        {
            if (resource.columns.columns.length == 1)
                return ;
            action.removeClass("selected");
            resource.columns.removeColumn(resource.columns.getColumn(name));
        }
        else
        {
            action.addClass("selected");
            var column = resource.columns.addColumn(name);
            resource.container.addColumn(column);
        }
    }
    
    // Displays the adjust columns actions.
    self.displaySeparator = function (e, column)
    {
        // Displays the normal actions if adjust column is not enabled
        if (!C.R.Files.Columns.adjustContext)
            return self.display(e, column);
        // Creates the adjust actions
        var actions = [gl_context.createAction(T.Files.Context.adjustColumn + ' "' + resource.columns.translateName(column.name) + '"', resource.columns.adjustColumn, column),
                       gl_context.createAction(T.Files.Context.adjustAllColumns, resource.columns.adjustAllColumns)];
        gl_context.display(e.pageX, e.pageY, actions);
    }
    
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
        return (MergeSort.sort(resource.files));
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
        return (parseInt(a.file.info[MergeSort.column]) <= parseInt(b.file.info[MergeSort.column]));
    },
    
    compareString: function (a, b)
    {
        return (a.file.info[MergeSort.column].toLowerCase() <= b.file.info[MergeSort.column].toLowerCase());
    }
};

    resource.init();
    return (resource);
}

function initialize_resource_files(task) { return (new ResourceFiles(task)); }
gl_resources.jsLoaded("files");
