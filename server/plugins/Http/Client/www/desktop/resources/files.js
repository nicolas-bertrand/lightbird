function Files(task)
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
        resource.files = new Array(); // The list of the files on the server
        resource.icons = new Icons(); // Generates the SVG icons
        resource.layout = new Layout(); // Manages the layout of the columns
        resource.header = new Header(); // The header of the files list
        resource.container = new List(); // The container handles the display of the files list. Each container have its own layout.
        
        // Download the files list
        resource.getFiles();
        // Sets the resource instance to the task, so that it can call close and onResize
        task.setResource(resource);
    }
    
    // Gets the list of the files from the server, and gives it to the container which will display them.
    resource.getFiles = function ()
    {
        // Gets the files
        request("GET", "Execute/FilesGet", function (HttpRequest)
        {
            if (HttpRequest.status == 200)
            {
                resource.files = jsonParse(HttpRequest.responseText);
                resource.container.initialize();
            }
        });
    }
    
    // The task have been resized.
    resource.onResize = function (left, top, width, height)
    {
        resource.container.onResize(left, top, width, height);
    }
    
    // Closes the resource.
    resource.close = function ()
    {
        for (var key in resource)
            resource[key] = undefined;
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
        
        self.updateColumns();
    }
    
    // Initializes the files list.
    self.initialize = function ()
    {
        // Hides the scroll if all the files can be seen at the same time
        if (self.listHeight / C.Files.listRowHeight >= resource.files.length)
            $(self.list).removeClass("scroll");
        else
            $(self.list).addClass("scroll");
        self.list.scrollTop = 0;
		// Displays the files
        for (var i = 0; i < resource.files.length; i++)
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
        if (self.listHeight / C.Files.listRowHeight >= resource.files.length)
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
        var isFile = absolute < resource.files.length;
        
        // Creates the row
        var row = self.table.insertRow(relative);
        $(row).addClass(absolute % 2 ? "even" : "odd");
        // The first cell is the type of the file
        if (isFile)
        {
            var type = resource.files[absolute].type;
            var firstCell = 0;
            if (!row.previousSibling || !$(row.previousSibling).hasClass(type))
            {
                var cell = $(row.insertCell(-1)).addClass("type")[0];
                for (var i = absolute; i < resource.files.length && resource.files[i].type == type; ++i)
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
            $(row).addClass(resource.files[absolute].type);
            $(row.cells[firstCell]).addClass("first");
            for (var i = 0; i < columns.length; ++i)
            {
                var cell = row.cells[i + firstCell];
                cell.name = columns[i].originalName;
                cell.originalText = resource.layout.convert(columns[i].originalName, resource.files[absolute][columns[i].originalName]);
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
    
    // Ensures that the list is allways filled with rows.
    self.addEmptyRows = function ()
    {
        var totalRows = self.listHeight / C.Files.listRowHeight;
        
        // Adds empty rows
        if (totalRows > resource.files.length)
        {
            for (var i = self.table.rows.length; self.table.rows.length < totalRows; i++)
                self.addRow(-1, i);
        }
        // Remove all the empty rows
        else
            while (self.table.rows.length > resource.files.length)
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
                path : "M11.974,10h-23.948c-0.552,0-1.526-0.974-1.526-1.526V-8.474c0-0.552,0.974-1.526,1.526-1.526h23.948c0.552,0,1.526,0.974,1.526,1.526V8.474C13.5,9.026,12.525,10,11.974,10z M11.5-8h-23V7.938h23V-8z M-7.796-3.171c-0.005-0.13-0.157-0.244-0.296-0.269S-8.311-3.647-7.99-3.767C-7.497-3.95-7.212-3.704-7.082-3.86c0.117-0.141,0.045-0.434,0.212-0.573c3.025-2.521,8.073-1.77,8.073-1.77c-4.396,0.55-7.299,2.925-6.849,3.136c0.45,0.211,1.929,1.084,2.149,1.296c0.22,0.212,0.015,0.391-0.349,0.339C-4.21-1.484-5.806-2.507-6.285-2.479C-6.87-2.445-6.882-2.435-7.457-1.486c-0.808,1.333-0.779,3.335-0.971,4.55C-9.03,1.791-8.748-2.051-8.666-2.424C-8.583-2.797-7.792-3.041-7.796-3.171z M-6.05,2.467c0.516,0,1.764,1.506,2.108,1.506s4.388-5.378,4.948-5.378c0.559,0,0.473,1.162,1.291,1.162c0.816,0,2.666-3.399,3.527-3.356c0.859,0.043,4.646,6.97,4.646,6.97l0.043,3.571L-8.502,6.984C-8.502,6.984-6.566,2.467-6.05,2.467z",
                width : 47,
                gradient : "90-#33bbff-#66ccff",
                line : "#33bbff",
                background : "90-#d6f1ff-#edf9ff",
            },
            music : 
            {
                path : "M9.996,4.847c0.006,0.072,0.027,0.142,0.027,0.214c0,0.076-0.02,0.147-0.027,0.222v0.702H9.823C9.347,7.151,8.011,8.026,6.339,8.026c-2.092,0-4.362-1.047-4.362-2.648s2.204-3.368,4.296-3.368c0.629,0,1.209,0.143,1.726,0.37v-8.213l-9.99,1.478V6.955c0.002,0.036,0.014,0.071,0.014,0.107c0,0.038-0.012,0.073-0.014,0.111v0.591h-0.102c-0.387,1.277-1.791,2.262-3.568,2.262c-2.092,0-4.362-1.047-4.362-2.648s2.203-3.368,4.296-3.368c0.635,0,1.22,0.146,1.741,0.376v-8.447l-0.002,0v-3.896l13.987-2.069v3.896l-0.002,0V4.847z",
                width : 40,
                gradient : "90-#46da57-#73e380",
                line : "#46da57",
                background : "90-#daf8dd-#effcf0",
            },
            video : 
            {
                path : "M8.776,10.023H-8.776c-0.738,0-1.725-0.987-1.725-1.725V2.064h20.984L10.5,8.298C10.5,9.036,9.514,10.023,8.776,10.023z M8.495,4.009H-8.5v1.083H8.495V4.009z M8.495,7.047H-8.5v1.007H8.495V7.047z M9.513-1.958h0.88v3.005H6.507L9.513-1.958z M5.639-9.243l3.806-0.78l0.604,2.944L9.187-6.902L5.639-9.243z M5.878-1.958L2.873,1.047h-3.078L2.8-1.958H5.878z M-0.937-7.894l3.015-0.618l3.548,2.34L2.611-5.554L-0.937-7.894z M-0.835-1.958l-3.005,3.005h-3.061l3.005-3.005H-0.835z M-7.497-6.549l2.999-0.615l3.548,2.34l-2.999,0.615L-7.497-6.549z M-10.496,1.007v-2.965h2.965L-10.496,1.007z M-10.496-3.205v-2.244l2.986,1.97l-2.92,0.599L-10.496-3.205z",
                width : 41,
                gradient : "90-#ff7733-#ff9865",
                line : "#ff7733",
                background : "90-#ffe4d6 -#fff3ed",
            },
            document : 
            {
                path : "M8.003,9.966L-8.003,9.974l0-12.943l7.06-7.005l8.947,0.04L8.003,9.966z M6.005-8.042H0.017l-0.038,6.063L-6.017-2.01v9.994H6.005V-8.042z",
                width : 36,
                gradient : "90-#ffc000-#ffd040",
                line : "#ffc000",
                background : "90-#fff2cc-#fff9e8",
            },
            other : 
            {
                path : "M8.455,1.767H6.267c-0.135,0.481-0.339,0.93-0.572,1.36l1.59,1.59c0.616,0.616,0.616,1.615,0,2.231L6.969,7.262c-0.616,0.616-1.615,0.616-2.231,0L3.186,5.709C2.768,5.947,2.318,6.133,1.85,6.276v2.172c0,0.871-0.706,1.577-1.577,1.577h-0.446c-0.871,0-1.577-0.706-1.577-1.577V6.304C-2.244,6.168-2.703,5.96-3.143,5.721L-4.7,7.277c-0.616,0.616-1.615,0.616-2.23,0l-0.315-0.315c-0.616-0.616-0.616-1.615,0-2.23l1.521-1.521C-5.969,2.78-6.16,2.317-6.304,1.834h-2.151c-0.871,0-1.578-0.706-1.578-1.577v-0.446c0-0.871,0.707-1.577,1.578-1.577h2.145c0.14-0.491,0.341-0.954,0.585-1.39l-1.56-1.56c-0.616-0.616-0.616-1.615,0-2.23l0.315-0.315c0.616-0.616,1.615-0.616,2.231,0l1.567,1.567c0.416-0.23,0.854-0.425,1.32-0.561v-2.191c0-0.871,0.707-1.578,1.578-1.578h0.446c0.871,0,1.577,0.707,1.577,1.578v2.173c0.484,0.136,0.935,0.343,1.368,0.58L4.7-7.277c0.616-0.616,1.615-0.616,2.23,0l0.315,0.315c0.616,0.616,0.616,1.615,0,2.231L5.693-3.178c0.238,0.42,0.423,0.873,0.564,1.344h2.197c0.872,0,1.578,0.706,1.578,1.577v0.446C10.032,1.06,9.326,1.767,8.455,1.767z M-0.021-3.808c-2.108,0-3.817,1.709-3.817,3.817c0,2.108,1.709,3.818,3.817,3.818S3.796,2.118,3.796,0.01C3.796-2.098,2.087-3.808-0.021-3.808z",
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
                path : "M7,6.984H-7c-1.104,0-2-0.896-2-2V0.966L9,0.904v4.081C9,6.089,8.104,6.984,7,6.984z M8.936-1.111L-9-1.074v-3.911c0-1.104,0.896-2,2-2h4c1.104,0,1.967,0.894,2,2c-0.026,0.63,0.404,0.942,1.008,0.976c0.604,0.033,6.538-0.039,6.961-0.039C8.108-4.016,9-3.124,9-2.062C8.84-2.064,8.936-1.111,8.936-1.111z",
                width : 18,
            },
            resize : 
            {
                path : "M4.99,7.031V7.01H1.988V5H4.99V1.957H7V5v2.01v0.021H4.99zM4.759-3.303L1.711-0.289L0.283-1.715l3.033-3L1-7l6,0.031L6.969-1.031L4.759-3.303z M-4.99-1.957H-7V-5v-2.01v-0.021h2.01v0.021h3.002V-5H-4.99V-1.957z M-4.76,3.272l3.042-3.008l1.427,1.427l-3.025,2.992L-1,6.969l-6-0.031L-6.969,1L-4.76,3.272z",
                width : 14,
            },
            list : 
            {
                path : "M-9.011,6.989V5.022H9.011v1.966H-9.011z M-9.011,1.021H9.011v1.966H-9.011V1.021z M-9.011-2.978H9.011v1.966H-9.011V-2.978z M-9.011-6.989H9.011v1.966H-9.011V-6.989z",
                width : 18,
                selected : true,
            },
            hierarchy : 
            {
                path : "M-9.011-5.023v-1.966H9.011v1.966H-9.011z M9.008-1.012H-5.014v-1.966H9.008V-1.012z M9.005,1.021v1.966H-1.007V1.021H9.005z M9.008,6.989H-5.009V5.022H9.008V6.989z",
                width : 18,
            },
            split : 
            {
                path : "M3.012,6.962V4.996h5.992v1.966H3.012z M3.012,0.996h5.992v1.966H3.012V0.996z M3.012-3.004h5.992v1.966H3.012V-3.004z M3.012-7.004h5.992v1.966H3.012V-7.004z M-1.002-6.979h1.996V7.004h-1.996V-6.979z M-9.004,4.996h5.988v1.966h-5.988V4.996z M-9.004,0.996h5.988v1.966h-5.988V0.996z M-9.004-3.004h5.988v1.966h-5.988V-3.004z M-9.004-7.004h5.988v1.966h-5.988V-7.004z",
                width : 18,
            },
            block : 
            {
                path : "M0.999,7.061V0.939h8.003v6.121H0.999z M7.018,3.041H3.013v1.976h4.005V3.041z M0.999-7.061h8.003v6.121H0.999V-7.061z M3.013-2.983h4.005v-1.976H3.013V-2.983z M-9.001,0.939h8.003v6.121h-8.003V0.939z M-6.987,5.017h4.005V3.041h-4.005V5.017z M-9.001-7.061h8.003v6.121h-8.003V-7.061z M-6.987-2.983h4.005v-1.976h-4.005V-2.983z",
                width : 18,
            },
        },
        add : 
        {
            path : "M4.999,1.042H1.016v3.954h-2.022V1.042h-3.988v-2.085h3.988v-3.954h2.022v3.954h3.982V1.042z",
            width : 20,
        },
    };

    self.init();
    return (self);
}
 
    resource.init();
    return (resource);
}

function initialize_files(task) { return new Files(task); }
gl_resources.jsLoaded("files");
