// Manages the file player and the playlist.
var gl_player;

function Player(task)
{
    var self = this;
    gl_player = self;
    
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.bottom = $("#desktop>.bottom");
        self.node.timeline = self.node.bottom.children(".timeline");
        self.node.seek = self.node.timeline.children(".seek");
        self.node.preview = self.node.seek.children(".preview");
        self.node.seek_time = self.node.seek.children(".time");
        self.node.activeArea = self.node.timeline.children(".active_area");
        self.node.player = self.node.bottom.children(".player");
        self.node.play = self.node.player.children(".play");
        self.node.settings = self.node.player.children(".settings");
        self.node.numerator = self.node.play.find(".numerator"); // The number of the current file played
        self.node.denominator = self.node.play.find(".denominator"); // The total number of files
        self.node.time = self.node.play.children(".time");
        self.node.current_time = self.node.play.find(".current_time"); // The current time of the file played
        self.node.duration = self.node.play.find(".duration"); // The duration of the file
        self.node.filename = self.node.player.children(".name");
        self.node.audio = self.node.bottom.children(".audio");
        // Icons
        self.node.icon = new Object();
        self.node.icon.play = $(self.node.play).children(".play");
        self.node.icon.pause = $(self.node.play).children(".pause");
        self.node.icon.previous = $(self.node.play).children(".previous");
        self.node.icon.next = $(self.node.play).children(".next");
        self.node.icon.volume = $(self.node.settings).children(".volume");
        self.node.icon.mute = $(self.node.settings).children(".mute");
        self.node.icon.settings = $(self.node.settings).children(".settings");
        self.node.icon.repeat = $(self.node.settings).children(".repeat");
        self.node.icon.repeatOne = $(self.node.settings).children(".repeat_one");
        self.node.icon.noRepeat = $(self.node.settings).children(".no_repeat");
        self.node.icon.random = $(self.node.settings).children(".random");
        self.node.icon.linear = $(self.node.settings).children(".linear");
        self.node.icon.fullscreen = $(self.node.settings).children(".fullscreen");
        self.node.icon.normalScreen = $(self.node.settings).children(".normal_screen");
        // Playlist
        self.node.playlist = self.node.bottom.children(".playlist");
        self.node.header = self.node.playlist.children(".header");
        self.node.tabs = self.node.header.children(".tabs");
        self.node.add = self.node.header.children(".add");
        self.node.pin = self.node.header.children(".pin");
        self.node.list = self.node.playlist.children(".list");

        // Members
        self.timeLine; // Manages the time line
        self.playlist; // Manages the playlist
        self.header; // Manages the header of the playlist
        self.audio; // Manages the audio player
        self.mouseLeaveTimeout = 0; // Delays the effect of the mouse leave
        self.mouseOverPlayer; // True while the mouse is over the player
        self.playerInterface; // The object that is playing the current file
        self.fileIndex; // The index of the file currently played
        
        // Default values
        C.Desktop.bottomHeight = C.Player.defaultHeight;
        self.node.bottom.height(C.Desktop.bottomHeight);
        self.node.numerator.html("0");
        self.node.denominator.html("0");
        self.node.current_time.html("0:00");
        self.node.duration.html("0:00");
        self.node.filename.html("");
        self.timeLine = new self.TimeLine(self);
        self.playlist = new self.Playlist(self);
        self.header = new self.Header(self);
        self.audio = new self.Audio(self);
        self.generateIcons();
        
        // Events
        self.node.bottom.mouseenter(function (e) { self.mouseEnter(e); });
        self.node.bottom.mouseleave(function (e) { self.mouseLeave(e); });
        self.node.play.children(".number").mousedown(function (e) { self.mouseDown(e); });
        self.node.player.children(".name").mousedown(function (e) { self.mouseDown(e); });
    }
    
    // Resizes the player when the browser size changes.
    self.onResize = function (width, height)
    {
        self.timeLine.onResize(width);
        self.playlist.onResize(width, height);
    }
    
    // The mouse entered the player area.
    self.mouseEnter = function (e)
    {
        self.mouseOverPlayer = true;
        // The player is already expanded
        if (self.mouseLeaveTimeout)
        {
            clearTimeout(self.mouseLeaveTimeout);
            self.mouseLeaveTimeout = 0;
        }
        // Expands the timeline
        else if (!self.playlist.isDisplayed() && !gl_desktop.drag.isDragging())
            self.timeLine.expand();
    }
    
    // The mouse leaved the player area.
    self.mouseLeave = function (e)
    {
        self.mouseOverPlayer = false;
        // If the playlist is not displayed, we retract the time line after the delay
        if (!self.playlist.isDisplayed() && self.timeLine.isExpanded())
        {
            self.mouseLeaveTimeout = setTimeout(function ()
            {
                self.timeLine.retract();
                self.mouseLeaveTimeout = 0;
            }, C.Player.mouseLeaveTimeout);
        }
        // Otherwise we wait a mouse down event outside the player to retract it and hide the playlist
        else
        {
            $("body").unbind("mousedown");
            $("body").mousedown(function (e)
            {
                $("body").unbind(e);
                if (!self.mouseOverPlayer && !self.playlist.isPinned())
                {
                    self.playlist.hide();
                    self.timeLine.retract();
                }
            });
        }
    }
    
    // Generates the SVG icons of the player.
    self.generateIcons = function ()
    {
        self.drawIcon(self.node.icon.play.children("div")[0], 16, gl_svg.Player.play, 0, 0.3);
        self.drawIcon(self.node.icon.pause.children("div")[0], 16, gl_svg.Player.pause);
        self.drawIcon(self.node.icon.previous.children("div")[0], 14, gl_svg.Player.previous, 0.5);
        self.drawIcon(self.node.icon.next.children("div")[0], 14, gl_svg.Player.next, -0.1);
        self.drawIcon(self.node.icon.volume.children("div")[0], 25, gl_svg.Player.volume);
        self.drawIcon(self.node.icon.mute.children("div")[0], 25, gl_svg.Player.mute);
        self.drawIcon(self.node.icon.settings.children("div")[0], 21, gl_svg.Player.settings);
        self.drawIcon(self.node.icon.repeat.children("div")[0], 27, gl_svg.Player.repeat, 0.2);
        self.drawIcon(self.node.icon.repeatOne.children("div")[0], 27, gl_svg.Player.repeatOne, 0.2, 3.2);
        self.drawIcon(self.node.icon.noRepeat.children("div")[0], 27, gl_svg.Player.noRepeat, 0.2);
        self.drawIcon(self.node.icon.random.children("div")[0], 27, gl_svg.Player.random);
        self.drawIcon(self.node.icon.linear.children("div")[0], 27, gl_svg.Player.linear);
        self.drawIcon(self.node.icon.fullscreen.children("div")[0], 25, gl_svg.Player.fullscreen);
        self.drawIcon(self.node.icon.normalScreen.children("div")[0], 25, gl_svg.Player.normalScreen);
    }

    // Draws an SVG icon.
    self.drawIcon = function (destination, width, path, left, top)
    {
        var paper = Raphael(destination, width, 20);
        var icon = paper.path(path);
        icon.translate(left, top);
        icon.attr("fill", "#eeeeee");
        icon.attr("stroke", "none");
        // Events
        $(destination.parentNode).mouseenter(function (e)
        {
            icon.attr("fill", "#444444");
        });
        $(destination.parentNode).mouseleave(function (e)
        {
            icon.attr("fill", "#eeeeee");
        });
        $(destination.parentNode).click(function (e) { self.mouseDown(e); });
    }
    
    // Mouse down on a button of the player.
    self.mouseDown = function (e)
    {
        var target = $(e.delegateTarget);
    
        if (e.which != 1)
            return ;
        if (target.hasClass("play") && self.playerInterface)
        {
            self.playerInterface.play();
            self.play();
        }
        else if (target.hasClass("pause") && self.playerInterface)
        {
            self.playerInterface.pause();
            self.pause();
        }
        // Opens / closes the playlist
        else if (target.hasClass("number") || target.hasClass("name"))
        {
            if (!self.playlist.isPinned())
            {
                if (!self.playlist.isDisplayed())
                    self.playlist.display();
                else
                    self.playlist.hide();
            }
            // If the playlist is pinned we resize it to the minimal height instead of hidding it
            else
            {
                if (self.playlist.height <= C.Player.headerHeight + 1)
                {
                    if (!self.oldPlaylistHeight)
                        self.oldPlaylistHeight = C.Player.playlistHeight;
                    self.playlist.setHeight(self.oldPlaylistHeight);
                }
                else
                {
                    self.oldPlaylistHeight = self.playlist.height;
                    self.playlist.setHeight(0);
                }
            }
        }
    }

    // Converts the time in seconds to a string (0:00).
    self.timeToString = function (seconds)
    {
        var second = Math.floor(seconds % 60);
        var minute = Math.floor(seconds / 60);
        var hour = Math.floor(minute / 60);
        var minute = Math.floor(minute % 60);
        var result = minute + ":" + (second < 10 ? "0" + second : second);
        if (hour)
            result = hour + ":" + (minute < 10 ? "0" : "") + result;
        return (result);
    }
    
// Player interface
    
    // Adds a file to the playlist.
    // @param playerInterface : The object that is playing the file. Must implement the player interface.
    self.addFile = function (fileIndex, playerInterface)
    {
        self.playlist.addFile(fileIndex);
        self.fileIndex = fileIndex;
        self.playerInterface = playerInterface;
        var file = gl_files.list[self.fileIndex];
        self.node.filename.html(file.name);
        // The audio files have to use our player
        if (file.type == "audio")
            (self.playerInterface = self.audio).setFile(self.fileIndex);
        else
            self.audio.clear();
        // Displays the time and initializes the time line if the file is a media
        if (file.type == "audio" || file.type == "video")
        {
            var media = self.playerInterface.getMedia();
            self.timeLine.newMedia(media);
            self.node.time.addClass("display");
        }
        else
        {
            self.timeLine.clear();
            self.node.time.removeClass("display");
        }
    }
    
    // Starts to play the file.
    self.play = function ()
    {
        self.node.icon.play.addClass("hide");
        self.node.icon.pause.removeClass("hide");
    }
    
    self.pause = function ()
    {
        self.node.icon.play.removeClass("hide");
        self.node.icon.pause.addClass("hide");
    }
    

// Manages the time line.
self.TimeLine = function (player)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.media; // The media currently played
        self.duration; // The duration of the media currently played
        self.mouseLeaveTimeout = 0; // Delays the effect of the mouse leaving the active area
        self.before = 0; // The percentage of the before part of the timeline
        self.played = 0; // The percentage of the media played
        self.buffered = 0; // The percentage of the media buffered
        self.previewHeight; // The height of the preview if it is displayed.
        self.currentPreviewTime; // The time of the preview currently displayed.
        
        // Default values
        self.drawTimeLine();
        self.drawSeek();
        
        // Events
        node.activeArea.mouseenter(function (e) { self.mouseEnter(e); });
        node.activeArea.mouseleave(function (e) { self.mouseLeave(e); });
        node.activeArea.mousemove(function (e) { self.mouseMove(e); });
        node.activeArea.mousedown(function (e) { self.mouseDown(e); });
    }
    
    // Draws the time line in SVG.
    self.drawTimeLine = function ()
    {
        var height = C.Player.timelineHeight + C.Player.timelineExpandHeight;
        var paper = Raphael(node.timeline[0], gl_browserSize.width, C.Player.timelineHeight);
        var svg = $(node.timeline).children("svg");
        
        svg.css("position", "absolute");
        var before = paper.rect(0, 0, 1, height);
        before.attr("stroke", "none");
        before.attr("fill", "#4a4a4a");
        var played = paper.rect(0, 0, 1, height);
        played.attr("stroke", "none");
        played.attr("fill", "#b91f1f");
        var buffered = paper.rect(0, 0, 1, height);
        buffered.attr("stroke", "none");
        buffered.attr("fill", "#cccccc");
        var after = paper.rect(0, 0, 1, height);
        after.attr("stroke", "none");
        after.attr("fill", "#4a4a4a");
        
        // Updates the time line SVG.
        self.updateTimeLine = function ()
        {
            var width = gl_browserSize.width;
            var translate = 0;
            var scale = self.before;

            paper.setSize(width);
            before.transform("S" + scale + ",1,0,0T" + translate + ",0");
            translate += scale;
            scale = width * self.played;
            played.transform("S" + scale + ",1,0,0T" + translate + ",0");
            translate += scale;
            scale = width * self.buffered;
            buffered.transform("S" + scale + ",1,0,0T" + translate + ",0");
            translate += scale;
            scale = width - translate;
            after.transform("S" + scale + ",1,0,0T" + translate + ",0");
        }
        
        // Expands the time line.
        self.expandTimeLine = function ()
        {
            paper.setSize(undefined, height);
            svg.css("top", "-" + C.Player.timelineExpandHeight + "px");
        }
        
        // Retracts the time line.
        self.retractTimeLine = function ()
        {
            paper.setSize(undefined, C.Player.timelineHeight);
            svg.css("top", "0px");
        }
        
        // Changes the color of the played part based on the file type.
        self.setTimeLineType = function (type)
        {
            if (type == "audio")
                played.attr("fill", "#20ba2f");
            else if (type == "video")
                played.attr("fill", "#b91f1f");
        }
        
        self.updateTimeLine();
        self.retractTimeLine();
    }
    
    // Draws the seek box.
    self.drawSeek = function ()
    {
        var border = C.Player.Seek.border;
        var height = C.Player.Seek.height;
        var attrSettings = { opacity : 0.9, fill : "#222222", stroke : "none" };
        var glowSettings = { width : 1, color : "white", opacity : 0.2 };
        var offset = 1000; // Random large number
        var middleWidth = 30; // The width of the middle area
        var currentWidth, currentPosition, currentHeight;
        
        // Right
        {
            var width = 10;
            var rightPaper = Raphael(node.seek[0], width + border, height + border * 2);
            var rightRect = rightPaper.rect(0, border, offset, height, C.Player.Seek.radius);
            rightRect.attr(attrSettings);
            var rightGlow = rightRect.glow(glowSettings);
        }
        // Middle
        {
            var middle = $("<div class=\"middle\"></div>");
            node.seek.prepend(middle);
            middle.width(middleWidth);
            // Bottom (down arrow)
            {
                /*var paper = Raphael(middle[0], middleWidth, 16);
                var path = paper.path(gl_svg.Player.seek);
                path.attr(attrSettings);
                path.transform("T-10,-9");
                path.glow(glowSettings);
                $(path.node.parentNode).css({ position : "absolute",
                                              top : (height + border - 1) + "px",
                                              left : "0px" });*/
            }
            // Middle top
            {
                var middlePaper = Raphael(middle[0], middleWidth, height + border * 2);
                var middleRect = middlePaper.rect(-5, border, middleWidth + 10, height);
                middleRect.attr(attrSettings);
                middleGlow = middleRect.glow(glowSettings);
            }
        }
        // Left
        {
            var width = 10;
            var leftPaper = Raphael(node.seek[0], width + border * 2, height + border * 2);
            var leftRect = leftPaper.rect(border, border, offset, height, C.Player.Seek.radius);
            leftRect.attr(attrSettings);
            var leftGlow = leftRect.glow(glowSettings);
        }
        // Top. Holds the preview when it is displayed.
        {
            var topPaper = Raphael(node.preview[0], middleWidth + 20, height);
            var topRect = topPaper.rect(border, border, 1, 1);
            topRect.attr(attrSettings);
            topGlow = topRect.glow(glowSettings);
        }
        // Adjusts the positions
        node.seek.css("top", -(C.Player.Seek.topPosition + C.Player.Seek.height + C.Player.Seek.border) + "px");
        node.seek_time.css("top", border + "px");
        node.seek_time.css("left", border + "px");
        
        // Updates the layout of the SVG depending on the width of the text,
        // its position on the time line, and the height of the preview if there is one.
        // @param width : The width of the text and its padding.
        // @param position : "left" or "right" if the cursor is on the sides of the time line.
        // @param height : The height of the preview above the seek box (if displayed).
        self.updateSeek = function (width, position, height)
        {
            // Do nothing if there is no change
            if (width == currentWidth && position == currentPosition && height == currentHeight)
                return ;
            // Updates the height of the SVG
            if (height != currentHeight)
            {
                // Default values
                var newHeight = C.Player.Seek.height
                var newTop = border;
                node.seek_time.css("left", border + "px");
                node.preview.removeClass("display");
                // Displays the preview
                if (height)
                {
                    newHeight = C.Player.Seek.height + height;
                    newTop = border - height;
                    node.preview.addClass("display");
                    node.preview.css("top", -height + "px");
                    topPaper.setSize(width + border * 2, height);
                    node.seek_time.css("left", Math.round(width / 2 - node.seek_time[0].offsetWidth / 2 + border) + "px");
                }
                // We need to recreate the rectangles with the new height
                // Right
                rightRect.remove();
                rightRect = rightPaper.rect(0, newTop, offset, newHeight, C.Player.Seek.radius);
                rightRect.attr(attrSettings);
                // Middle
                middleRect.remove();
                middleRect = middlePaper.rect(-5, newTop, middleWidth + 10, newHeight);
                middleRect.attr(attrSettings);
                middleGlow.remove();
                middleGlow = middleRect.glow(glowSettings);
                // Left
                leftRect.remove();
                leftRect = leftPaper.rect(border, newTop, offset, newHeight, C.Player.Seek.radius);
                leftRect.attr(attrSettings);
            }
            // Saves the new width, position and height
            currentWidth = width;
            currentPosition = position;
            currentHeight = height;
            // Updates the width of the SVGs
            width -= middleWidth;
            var halfWidth = Math.floor(width / 2);
            leftPaper.setSize(halfWidth + width % 2 + border);
            leftRect.transform("T0,0");
            rightPaper.setSize(halfWidth + border);
            rightRect.transform("T" + (halfWidth - offset) + ",0");
            topPaper.setSize(currentWidth + border * 2);
            topRect.remove();
            topRect = topPaper.rect(border, border, currentWidth, height + border);
            topRect.attr(attrSettings);
            // Manages the left and right cases
            if (position == "left")
                leftRect.transform("T-" + (offset / 2) + ",0");
            else if (position == "right")
                rightRect.transform("T-" + (offset / 2) + ",0");
            // Updates the glows
            leftGlow.remove();
            rightGlow.remove();
            topGlow.remove();
            leftGlow = leftRect.glow(glowSettings);
            rightGlow = rightRect.glow(glowSettings);
            topGlow = topRect.glow(glowSettings);
        }
        
        // Sets the preview to display in the seek box.
        self.setPreview = function (url)
        {
            var image = node.preview.children("img");
            image[0].src = url;
            image.width(currentWidth);
            image.height(currentHeight);
            image.css("top", border + "px");
            image.css("left", border + "px");
        }
    }
    
    // Updates the time line based on the width of the browser.
    self.onResize = function (width)
    {
        self.updateTimeLine();
    }
    
    // Changes the media displayed in the time line.
    self.newMedia = function (media)
    {
        var file = gl_files.list[player.fileIndex];
        self.clear();
        self.media = media;
        self.duration = file.duration;
        node.current_time.html("0:00");
        node.duration.html(player.timeToString(self.duration));
        self.setTimeLineType(file.type);
        if (file.type == "video")
            self.previewHeight = Math.round(C.Player.Seek.previewWidth * file.height / file.width);
        else
            self.previewHeight = 0;
        self.currentPreviewTime = undefined;
        $(media).bind("timeupdate", function (e) { self.onTimeUpdate(e); });
        $(media).bind("progress", function (e) { self.onTimeUpdate(e); });
    }
    
    // Clears the time line.
    self.clear = function ()
    {
        if (self.media)
        {
            $(self.media).unbind("timeupdate");
            $(self.media).unbind("progress");
        }
        self.media = undefined;
        self.duration = 0;
        self.before = 0;
        self.played = 0;
        self.buffered = 0;
        self.currentPreviewTime = undefined;
        self.updateTimeLine();
        node.current_time.html("0:00");
        node.duration.html("0:00");
    }
    
    // The time of the media played has changed.
    // Updates the time line based on the new time.
    self.onTimeUpdate = function (e)
    {
        var currentTime = self.media.currentTime;
        var buffered = 0;
        if (self.media.buffered.length)
            buffered = self.media.buffered.end(0) - self.media.buffered.start(0) - currentTime;
        self.played = currentTime / self.duration;
        self.buffered = buffered / self.duration;
        self.updateTimeLine();
        node.current_time.html(player.timeToString(currentTime));
    }
    
    // Mouse enter the time line.
    self.mouseEnter = function (e)
    {
        if (!self.media || gl_desktop.drag.isDragging())
            return ;
        node.timeline.addClass("over");
        node.activeArea.addClass("expand");
        if (self.mouseLeaveTimeout)
        {
            clearTimeout(self.mouseLeaveTimeout);
            self.mouseLeaveTimeout = 0;
        }
    }
    
    // Mouse leave the time line.
    self.mouseLeave = function (e)
    {
        if (!self.media)
            return ;
        node.timeline.removeClass("over");
        self.mouseLeaveTimeout = setTimeout(function ()
        {
            node.activeArea.removeClass("expand");
            self.mouseLeaveTimeout = 0;
        }, C.Player.mouseLeaveTimeout);
    }
    
    // Mouse move over the time line.
    self.mouseMove = function (e)
    {
        if (!self.media)
            return ;
        var seek = node.seek[0];
        var time = node.seek_time[0];
        var width = self.previewHeight ? C.Player.Seek.previewWidth : time.offsetWidth;
        
        // Updates the time
        time.innerHTML = player.timeToString(e.pageX / gl_browserSize.width * self.duration);
        // Manages the left and right borders of the time line
        var left = e.pageX - width / 2 - C.Player.Seek.border - 2;
        var position;
        if (left <= -C.Player.Seek.positionLimit - C.Player.Seek.border)
        {
            left = -C.Player.Seek.positionLimit - C.Player.Seek.border;
            position = "left";
        }
        else if (left + width + C.Player.Seek.border > gl_browserSize.width + C.Player.Seek.positionLimit)
        {
            left = gl_browserSize.width + C.Player.Seek.positionLimit - C.Player.Seek.border - width;
            position = "right";
        }
        // Updates the seek box
        self.updateSeek(width, position, self.previewHeight);
        seek.style.left = left + "px";
        // Updates the preview
        if (self.previewHeight)
        {
            // Computes the time of the preview under the cursor
            var previewTime = Math.round(Math.floor(e.pageX / (gl_browserSize.width / C.Player.Seek.numberPreviews) + 1) * self.duration / C.Player.Seek.numberPreviews);
            // Gets the new preview if the current one doesn't match
            if (self.currentPreviewTime != previewTime)
                self.setPreview("command/preview?id=" + gl_files.list[player.fileIndex].id + "&width=" + width + "&height=" + self.previewHeight + "&position=" + previewTime + getSession());
            self.currentPreviewTime = previewTime;
        }
    }
    
    // Seeks to the position on the cursor.
    self.mouseDown = function (e)
    {
        if (!self.media || e.which != 1)
            return ;
        var time = e.pageX / gl_browserSize.width * self.duration;
        if (self.media.buffered.length > 0 && self.media.buffered.end(0) > time)
            self.media.currentTime = time;
    }
    
    // Expands the time line.
    self.expand = function ()
    {
        self.expandTimeLine();
        node.timeline.addClass("expand");
    }
    
    // Retracts the time line.
    self.retract = function ()
    {
        self.retractTimeLine();
        node.timeline.removeClass("expand");
    }
    
    // Returns trus if the time line is expanded.
    self.isExpanded = function ()
    {
        return (node.timeline.hasClass("expand"));
    }
    
    self.init();
    return (self);
}

// Manages the playlist.
self.Playlist = function (player)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.height; // The height of the playlist
        self.pinned; // True if the playlist is pinned
        self.rowTemplate = node.playlist.children(".row_template").html(); // The template used to create the rows.
        
        // Default values
        self.setHeight(C.Player.playlistHeight);
        self.hide();
        
        // Events
    }
    
    // Sets the height of the playlist.
    self.setHeight = function (height)
    {
        var listHeight;
        if (!self.pinned)
            listHeight = self.setHeightUnpinned(height);
        else
            listHeight = self.setHeightPinned(height);
        // Fills the empty space of the playlist with empty rows
        var wantedFilesNumber = listHeight / C.Player.listFileHeight;
        var currentFilesNumber = node.list.children().length;
        for (var i = currentFilesNumber; i < wantedFilesNumber; i++)
            $("<div></div>").addClass(i % 2 ? "even" : "odd").appendTo(node.list);
        currentFilesNumber = i;
        for (var i = currentFilesNumber - 2; i > wantedFilesNumber; i--)
            node.list.children().last().remove();
    }
    // Sets the height in the unpinned mode.
    self.setHeightUnpinned = function (height)
    {
        // Clamp the height
        if (height <= C.Player.headerHeight)
            height = C.Player.headerHeight + 1;
        else if (height > gl_desktop.middleHeight - C.Player.timelineExpandHeight)
            height = gl_desktop.middleHeight - C.Player.timelineExpandHeight;
        // Resizes the playlist
        var listHeight = height - C.Player.headerHeight;
        self.height = height;
        node.playlist.height(height);
        node.list.height(listHeight);
        node.playlist[0].style.top = -(height + C.Player.timelineExpandHeight) + "px";
        return (listHeight);
    }
    // Sets the height in the pinned mode.
    self.setHeightPinned = function (height)
    {
        // Clamp the height
        if (height <= C.Player.headerHeight)
            height = C.Player.headerHeight + 1;
        else if (gl_browserSize.height - (height + C.Player.defaultHeight + C.Player.timelineExpandHeight) < C.Desktop.topHeight)
            height = gl_browserSize.height - (C.Player.defaultHeight + C.Player.timelineExpandHeight + C.Desktop.topHeight);
        // Resizes the playlist
        var listHeight = height - C.Player.headerHeight;
        self.height = height;
        node.playlist.height(height);
        node.list.height(listHeight);
        // Updates the desktop
        C.Desktop.bottomHeight = self.height + C.Player.defaultHeight + C.Player.timelineExpandHeight;
        node.bottom.css("padding-top", self.height + C.Player.timelineExpandHeight + "px");
        gl_desktop.onResize();
        return (listHeight);
    }
    
    // Resizes the playlist when the browser size changes.
    self.onResize = function (width, height)
    {
        node.playlist.width(width);
        // Checks if the playlist is highter than the top part of the desktop
        if (!self.pinned)
        {
            if (self.height > height - C.Desktop.bottomHeight - C.Desktop.topHeight)
                self.setHeight(self.height);
        }
        else if (height - (self.height + C.Player.defaultHeight + C.Player.timelineExpandHeight) < C.Desktop.topHeight)
            self.setHeight(self.height);
    }

    // Displays the playlist.
    self.display = function ()
    {
        node.playlist.addClass("display");
        node.playlist.height(self.height);
    }

    // Hides the playlist.
    self.hide = function ()
    {
        node.playlist.removeClass("display");
        node.playlist.height(0);
    }
    
    // Returns true if the playlist is displayed.
    self.isDisplayed = function ()
    {
        return (node.playlist.hasClass("display"));
    }
    
    // Pin the playlist to the player.
    self.pin = function ()
    {
        self.pinned = true;
        node.playlist.css("top", "0px");
        self.setHeight(self.height);
    }
    
    // Unpin the playlist from the player.
    self.unpin = function ()
    {
        self.pinned = false;
        node.bottom.css("padding-top", "0px");
        node.playlist.css("top", -(self.height + C.Player.timelineExpandHeight) + "px");
        self.hide();
        player.timeLine.retract();
        C.Desktop.bottomHeight = C.Player.defaultHeight;
        gl_desktop.onResize();
    }
    
    // Returns true if the playlist is pinned to the player.
    self.isPinned = function ()
    {
        return (self.pinned);
    }
    
    // Adds a file to the playlist.
    self.addFile = function (fileIndex)
    {
        // Creates a new playlist if there is none
        if (!player.header.tabFocused.length)
            new player.Tab(player, T.Player.recentFiles);
        // Adds the file to the current playlist
        player.header.tabFocused[0].object.addFile(fileIndex);
    }
    
    self.init();
    return (self);
}
    
// Manages the headers of the playlist, which includes the tabs, the pin and the resize.
self.Header = function (player)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.tabTemplate = node.header.children(".tab_template").html(); // The template used to create the tabs
        // Playlist resize
        self.element; // The initial y position of the header
        self.mouse; // The y position of the mouse in the header
        self.initialHeight; // The initial height of the playlist
        self.tabFocused = $(); // The tab that has the focus
        
        // Default values
        node.header.height(C.Player.headerHeight);
        self.drawAdd();
        self.drawPin();
        
        // Events
        node.header.mousedown(function (e) { self.mouseDownOnHeader(e); });
    }
    
    // Emulates the mouse enter, leave and click events for the close icon that is below the focused tab.
    self.mouseMoveOnFocusTab = function (e)
    {
        // We are not over the the close icon
        if (e.pageX - e.delegateTarget.offsetLeft > C.Player.tabExternLeft || !e.delegateTarget.previousSibling)
            return ;
        // Gets the exact position of the icon
        var left =  e.delegateTarget.offsetLeft - 7;
        var right = left + 15;
        var top = $(e.delegateTarget).offset().top + 3;
        var bottom = top + 15;
        // We are over the icon
        if (e.pageX <= right && e.pageY >= top && e.pageY <= bottom)
        {
            var close = $(e.delegateTarget.previousSibling).children(".close")[0].firstChild;
            if (close.mouseOverCloseIcon)
                return ;
            // Emulates the click event
            var tabLeft = $(e.delegateTarget).children(".left");
            tabLeft.mousedown(function (e) { close.mouseDownOnCloseIcon = (e.which == 1); });
            $(close).mouseup(function (ev) { if (close.mouseDownOnCloseIcon && ev.which == 1) e.delegateTarget.previousSibling.object.remove(); });
            // Emulates the mouse leave event
            $("body").mousemove(function (e)
            {
                if (e.pageX < left || e.pageX > right || e.pageY < top || e.pageY > bottom)
                {
                    $("body").unbind(e);
                    tabLeft.unbind("mousedown");
                    $(close).unbind("mouseup");
                    close.mouseOverCloseIcon = false;
                    close.mouseDownOnCloseIcon = false;
                    close.mouseLeaveCloseIcon();
                }
            });
            close.mouseEnterCloseIcon();
            close.mouseOverCloseIcon = true;
        }
    }
    
    // Puts the focus on a tab.
    self.focus = function (tab)
    {
        var tabs = node.tabs.children();
        var z = tabs.length;
        
        // Updates the z-index
        tabs.each(function ()
        {
            this.style.zIndex = z--;
        });
        self.tabFocused.removeClass("focus");
        self.tabFocused.unbind("mousemove");
        self.updateOddEvenTab();
        // Applies the new focus
        tab.addClass("focus");
        for (var j = 0; j < tab[0].tabIconEvent.length; ++j)
            tab[0].tabIconEvent[j]("focus");
        tab.css("z-index", tabs.length + 1);
        tab.mousemove(function (e) { self.mouseMoveOnFocusTab(e); });
        self.tabFocused = tab;
    }
    
    // Updates the odd / even alternation of the tabs.
    self.updateOddEvenTab = function ()
    {
        var tabs = node.tabs.children();
        var i = 0;
        
        tabs.each(function ()
        {
            var node = $(this);
            node.removeClass("odd");
            node.removeClass("even");
            if (i++ % 2)
            {
                node.addClass("odd");
                if (!node.hasClass("focus"))
                    for (var j = 0; j < this.tabIconEvent.length; ++j)
                        this.tabIconEvent[j]("odd");
            }
            else
            {
                node.addClass("even");
                if (!node.hasClass("focus"))
                    for (var j = 0; j < this.tabIconEvent.length; ++j)
                        this.tabIconEvent[j]("even");
            }
        });
    }
    
    // Draws the add element of the header.
    self.drawAdd = function ()
    {
        var paper = Raphael(node.add[0], 53, 23);
        // Background
        var tab = paper.path(gl_svg.Player.addTab);
        tab.translate(4, 0);
        tab.attr("fill", "#404040");
        tab.attr("stroke", "none");
        tab.glow({ width : 6, color : "black", opacity : 0.1 });
        tab.glow({ width : 1, color : "#4d4d4d", opacity : 1 });
        // Fills the left side of the background when there is no tab
        var rect = paper.rect(0, 0, 24, 24);
        rect.attr("stroke", "none");
        rect.attr("fill", "#242424");
        rect.hide();
        // The add icon
        var add = paper.path(gl_svg.Player.add);
        add.translate(28, 6.95);
        add.attr("fill", "white");
        add.attr("opacity", "0.5");
        add.attr("stroke", "none");
        add.glow({ width : 4, color : "black", opacity : 0.05 });
        node.add.mousedown(function (e) { self.mouseDownOnTab(e); });
        // The style displayed when there is no tab
        var glow = add.glow({ width : 4, color : "black", opacity : 0.05 });
        glow.translate(-3, 0);
        glow.hide();
        node.add.addIconNoTabStyle = function ()
        {
            if (!node.add.addIconNoTab)
            {
                node.add.addIconNoTab = true;
                tab.attr("fill", "#242424");
                add.translate(-3, 0);
                rect.show();
                glow.show();
            }
        };
        // Displays the default style back
        node.add.addIconDefaultStyle = function ()
        {
            if (node.add.addIconNoTab)
            {
                node.add.addIconNoTab = false;
                tab.attr("fill", "#404040");
                add.translate(3, 0);
                rect.hide();
                glow.hide();
            }
        };
        self.updateAdd();
    }
    
    // Puts the add icon after the last tab.
    self.updateAdd = function ()
    {
        var lastTab = node.tabs.children().last()[0];
        
        // The icon has not been drawn yet
        if (!node.add.addIconDefaultStyle)
            return ;
        // Clears the previous update if it is still running
        clearInterval(node.add.addTabInterval);
        // Waits for the offsetWidth of the last tab to be defined before updating the position
        if (lastTab)
        {
            node.add.addTabInterval = setInterval(function ()
            {
                if (lastTab.offsetWidth && lastTab.offsetLeft)
                {
                    node.add.css("left", (lastTab.offsetLeft + lastTab.offsetWidth - C.Player.addIconMargin) + "px");
                    clearInterval(node.add.addTabInterval);
                }
            }, C.Player.tabOffsetInterval);
            node.add.addIconDefaultStyle();
        }
        // There is no tab, so we put the add icon at the origin
        else
        {
            node.add.addIconNoTabStyle();
            node.add.css("left", C.Player.addIconOrigin + "px");
        }
    }
    
    // Focus / add a tab.
    self.mouseDownOnTab = function (e)
    {
        var tab = self.getTabUnderMouse(e);
        
        if (e.which == 1 && tab)
        {
            if (tab.hasClass("tab"))
            {
                self.focus(tab);
                tab[0].object.mouseDown(e);
            }
            else if (tab.hasClass("add"))
                new player.Tab(player, "New playlist " + (node.tabs.children().length + 1));
        }
    }
    
    // Draws the pin element of the header.
    self.drawPin = function ()
    {
        var paper = Raphael(node.pin[0], 53, 24);
        var tabLeft = paper.path(gl_svg.Player.tabLeft);
        tabLeft.translate(4, 1);
        tabLeft.attr("fill", "#242424");
        tabLeft.attr("stroke", "none");
        tabLeft.glow({ width : 6, color : "black", opacity : 0.15 });
        tabBorder = tabLeft.glow({ width : 1, color : "#4d4d4d", opacity : 1 });
        tabBorderOver = tabLeft.glow({ width : 1, color : "#e5e5e5", opacity : 1 }).hide();
        var rect = paper.rect(35, 1, 50, 24);
        rect.attr("fill", "#242424");
        rect.attr("stroke", "none");
        var pin = paper.path(gl_svg.Player.pin);
        pin.translate(28, 5);
        pin.attr("fill", "white");
        pin.attr("opacity", "0.5");
        pin.attr("stroke", "none");
        var pinShadow = pin.glow({ width : 6, color : "black", opacity : 0.15 });
        var borderTop = paper.rect(30, 0, 50, 1);
        borderTop.attr("fill", "#4d4d4d");
        borderTop.attr("stroke", "none");
        // Events
        node.pin.mouseenter(function (e)
        {
            tabLeft.attr("fill", "#e5e5e5");
            tabBorder.hide();
            tabBorderOver.show();
            rect.attr("fill", "#e5e5e5");
            pin.attr("fill", "#444444");
            pin.attr("opacity", "1");
            pinShadow.hide();
            borderTop.attr("fill", "#e5e5e5");
            node.pin.addClass("over");
        });
        node.pin.mouseleave(function (e)
        {
            tabLeft.attr("fill", "#242424");
            tabBorder.show();
            tabBorderOver.hide();
            rect.attr("fill", "#242424");
            pin.attr("fill", "white");
            pin.attr("opacity", "0.5");
            pinShadow.show();
            borderTop.attr("fill", "#4d4d4d");
            node.pin.removeClass("over");
        });
        node.pin.click(function (e)
        {
            if (!player.playlist.isPinned())
                player.playlist.pin();
            else
                player.playlist.unpin();
        });
    }
    
    // Starts to resize the playlist.
    self.mouseDownOnHeader = function (e)
    {
        // Something is already being dragged
        if (e.which != 1 || gl_desktop.drag.isDragging() || !$(e.target).hasClass("header"))
            return ;
        gl_desktop.drag.start(e, node.header[0], self, "mouseMoveResize");
        self.element = gl_desktop.drag.getElement().y;
        self.mouse = gl_desktop.drag.getMouse().y;
        self.initialHeight = player.playlist.height;
    }
    
    // Resizes the playlist while the header is being dragged.
    self.mouseMoveResize = function (e)
    {
        player.playlist.setHeight(self.element + self.mouse - e.pageY + self.initialHeight);
    }
    
    // Returns the real tab under the mouse, which can be different from the
    // target because of the curves of the tabs.
    self.getTabUnderMouse = function (e)
    {
        var root;
        var target = e.target;
        var tab;
        var add;
        
        // We are on a tab
        if ((tab = $(e.delegateTarget)).hasClass("tab"))
        {
            // If we don't have a target inside the tab, we just make a rough approximation
            if ($(e.target).hasClass("tab"))
            {
                var position = e.pageX - tab[0].offsetLeft;
                if (position < C.Player.tabExternLeft)
                    return (tab.prev());
                else if (position > tab[0].offsetWidth - C.Player.tabExternRight)
                    return (tab.next().length ? tab.next() : node.add);
                return (tab);
            }
            // Otherwise we can use the path of the svg to ensure that the mouse is over the tab
            if (target.tagName == "svg")
                root = target.parentNode;
            else if (target.tagName == "path")
                root = target.parentNode.parentNode;
            // We might be out of the tab
            if (root && ($(root).hasClass("left") || $(root).hasClass("right")) && !target.isMainPath)
            {
                // Resolves a bug on the right side
                if (target.tagName != "svg" || (e.pageX - root.parentNode.offsetLeft - target.parentNode.offsetLeft) >= 0)
                {
                    // The correct tab is on the left
                    if ($(root).hasClass("left"))
                        tab = tab.prev();
                    // On the right
                    else if (!(tab = tab.next()).length)
                        // There is nothing on the right of the tab, so the mouse is over the add button
                        return (node.add);
                }
            }
            if (tab.length)
                return (tab);
        }
        // The mouse is on the add tab icon
        if ($(e.delegateTarget).hasClass("add"))
            return (node.add);
    }
    
    self.init();
    return (self);
}

// Creates and manages a tab.
// name : The name of the tab.
self.Tab = function (player, name)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.tab = self.create(name);
        self.tab[0].object = self;
        self.files = new Array();
        // Drag tab
        self.element; // The initial position of the tab
        self.mouse; // The position of the mouse in the tab
        self.resistance; // The dragging will start only when the resistance is broken
        self.resizeResistance; // The resize of the playlist will start only when the resistance is broken
    }
    
    // Creates the tab
    self.create = function (name)
    {
        var tab = $(player.header.tabTemplate);
        var lastTab = node.tabs.children().last()[0];
        
        // Builds the tab
        tab[0].tabIconEvent = new Array(); // Allows to send events to the icon (focus, odd, even)
        self.drawTabIcon(tab.children(".left")[0], 4, gl_svg.Player.tabLeft);
        self.drawTabIcon(tab.children(".right")[0], -5, gl_svg.Player.tabRight);
        self.drawCloseIcon(tab.children(".close").children()[0], gl_svg.Player.tabClose);
        tab.children(".middle").children()[0].innerHTML = name;
        player.header.focus(tab);
        tab.mousedown(function (e) { player.header.mouseDownOnTab(e); });
        tab.mouseup(function (e) { self.mouseUp(e); });
        tab.appendTo(node.tabs);
        // Waits for the offsetWidth of the tab to be defined before displaying it,
        // since we can't know the width of the tab in advance because of its text.
        tab.addTabInterval = setInterval(function ()
        {
            if (tab[0].offsetWidth)
            {
                // Ensures that the tab doesn't "break"
                tab.width(tab[0].offsetWidth + 5);
                clearInterval(tab.addTabInterval);
                // We need the offsetWidth and offsetLeft of the previous tab, which might not be defined yet
                if (lastTab)
                    tab.addTabInterval = setInterval(function ()
                    {
                        if (lastTab.offsetWidth && lastTab.offsetLeft)
                        {
                            // Sets the position of the tab relative to the previous one
                            tab.css("left", (C.Player.tabOrigin + lastTab.offsetLeft + lastTab.offsetWidth - C.Player.tabMargin) + "px");
                            clearInterval(tab.addTabInterval);
                            tab.addClass("display");
                            // Puts the add icon after the new tab
                            player.header.updateAdd();
                        }
                    }, C.Player.tabOffsetInterval);
            }
        }, C.Player.tabOffsetInterval);
        // The first tab can be directly displayed
        if (!lastTab)
        {
            tab.css("left", C.Player.tabOrigin + "px");
            tab.addClass("display");
            player.header.updateAdd();
        }
        return (tab);
    }
    
    // Draws the left or right part of the tab in SVG.
    self.drawTabIcon = function (destination, left, path)
    {
        var paper = Raphael(destination, 30, 24);
        var icon = paper.path(path);
        icon.node.isMainPath = true;  // Distinguishes the main path from its shadow
        icon.translate(left, 0);
        icon.attr("fill", "90-#252525-#2c2c2c");
        icon.attr("stroke", "none");
        icon.glow({ width : 6, color : "black", opacity : 0.15 });
        icon.glow({ width : 1, color : "#4d4d4d", opacity : 1 })[0].node.isMainPath = true; // Part of the main path
        // Covers the shadow on the bottom border
        var rect;
        if ($(destination).hasClass("left"))
            rect = paper.rect(0, 23, 5, 1);
        else
            rect = paper.rect(27, 23, 5, 1);
        rect.attr("fill", "#4d4d4d");
        rect.attr("stroke", "none");
        // Changes the background of the SVG depending on the event
        destination.parentNode.tabIconEvent.push(function (name)
        {
            if (name == "odd")
                icon.attr("fill", "#292929");
            else if (name == "even")
                icon.attr("fill", "#303030");
            else if (name == "focus")
                icon.attr("fill", "90-#252525-#2c2c2c");
        });
    }
    
    // Draws the SVG close icon of the tab.
    self.drawCloseIcon = function (destination, path)
    {
        var paper = Raphael(destination, 14, 14);
        var icon = paper.path(path);
        icon.translate(3.5, 3.5);
        icon.attr("fill", "white");
        icon.attr("fill-opacity", "0.25");
        icon.attr("stroke", "none");
        icon.glow({ width : 3, color : "#000000", opacity : 0.13 });
        // Events
        destination.mouseEnterCloseIcon = function ()
        {
            icon.attr("fill-opacity", "0.8");
        };
        $(destination).mouseenter(function () { destination.mouseEnterCloseIcon(); });
        destination.mouseLeaveCloseIcon = function ()
        {
            if (!this.mouseOverCloseIcon)
                icon.attr("fill-opacity", "0.25");
        }
        $(destination).mouseleave(function () { destination.mouseLeaveCloseIcon(); });
        $(destination).click(function (e)
        {
            if (e.which == 1)
                self.remove();
        });
    }
    
    // Removes the tab.
    self.remove = function ()
    {
        var offset = self.tab[0].offsetWidth - C.Player.tabMargin + C.Player.tabOrigin;
        
        // Moves the next tabs to replace the removed tab
        for (var nextTab = self.tab.next(); nextTab.length; nextTab = nextTab.next())
            nextTab.css("left", (nextTab[0].offsetLeft - offset) + "px");
        // Ensures that the focus is always on a tab
        if (self.tab.hasClass("focus"))
        {
            if (self.tab.next().length)
                player.header.focus(self.tab.next());
            else if (self.tab.prev().length)
                player.header.focus(self.tab.prev());
            else
                player.header.tabFocused = $();
        }
        // Removes the tab and updates the others
        self.tab.remove();
        player.header.updateOddEvenTab();
        player.header.updateAdd();
    }

    // Starts the dragging of the tab.
    self.mouseDown = function (e)
    {
        if (gl_desktop.drag.isDragging())
            return ;
        gl_desktop.drag.start(e, self.tab[0], self, "mouseMove", "", "mouseUpMove");
        self.element = gl_desktop.drag.getElement();
        self.mouse = gl_desktop.drag.getMouse();
        self.resistance = true;
        self.resizeResistance = true;
    }
    
    // Drags the tab.
    self.mouseMove = function (e)
    {
        var tab = self.tab[0];
        
        self.resizePlaylist(e);
        // Starts to drag the tab horizontally once the resistance is broken
        if (self.resistance)
        {
            var delta = e.pageX - (self.element.x + self.mouse.x);
            if (Math.abs(delta) <= C.Player.tabDragResistance)
                return ;
            self.mouse.x += delta;
            node.add.addClass("hide");
            self.resistance = false;
        }
        // Moves the tab horizontally
        var x = e.pageX - self.mouse.x;
        var left = tab.offsetLeft;
        if (x < C.Player.tabOrigin)
            x = C.Player.tabOrigin;
        // The tab has been moved to the left
        if (x < left)
            for (var previous = tab.previousSibling; previous && previous.offsetLeft + previous.offsetWidth / 3 > x; previous = previous.previousSibling.previousSibling)
            {
                self.tab.after(previous);
                previous.style.left = (previous.offsetLeft + tab.offsetWidth - C.Player.tabMargin + C.Player.tabOrigin) + "px";
                player.header.updateOddEvenTab();
            }
        // To the right
        else if (x > left)
        {
            for (var next = tab.nextSibling; next && next.offsetLeft + next.offsetWidth - next.offsetWidth / 3 < x + tab.offsetWidth; next = next.nextSibling.nextSibling)
            {
                self.tab.before(next);
                next.style.left = (next.offsetLeft - tab.offsetWidth + C.Player.tabMargin - C.Player.tabOrigin) + "px";
                player.header.updateOddEvenTab();
            }
        }
        tab.style.left = x + "px";
    }
    
    // Resizes the playlist if the mouse is on the vertical edges of the header.
    self.resizePlaylist = function (e)
    {
        var y = e.pageY - node.header.offset().top;
        
        // Applies the resistance, so that the resize starts only when the mouse is far enough
        if (self.resizeResistance)
        {
            var delta = 0;
            if (y < -C.Player.playlistResizeEdges)
                delta = y + C.Player.playlistResizeEdges;
            else if (y > C.Player.headerHeight + C.Player.playlistResizeEdges)
                delta = y - C.Player.headerHeight - C.Player.playlistResizeEdges;
            // The resistance broke
            if (Math.abs(delta) > C.Player.playlistResizeResistance)
                self.resizeResistance = false;
        }
        // Resizes the playlist
        else
        {
            if (y < -C.Player.playlistResizeEdges)
                player.playlist.setHeight(player.playlist.height - y - C.Player.playlistResizeEdges);
            else if (y > C.Player.headerHeight + C.Player.playlistResizeEdges)
                player.playlist.setHeight(player.playlist.height - y + C.Player.headerHeight + C.Player.playlistResizeEdges);
        }
    }
    
    // Moves the dragged tab to its final position.
    self.mouseUpMove = function (e)
    {
        node.add.removeClass("hide");
        if (self.tab.prev().length)
            self.tab.css("left", (C.Player.tabOrigin + self.tab.prev()[0].offsetLeft + self.tab.prev().width() - C.Player.tabMargin) + "px");
        else
            self.tab.css("left", C.Player.tabOrigin + "px");
    }
    
    // Removes a tab.
    self.mouseUp = function (e)
    {
        var tab = player.header.getTabUnderMouse(e);
        
        if (e.which == 2 && tab && tab.hasClass("tab"))
            tab[0].object.remove();
    }
    
// File management

    // Adds a file to the list.
    self.addFile = function (fileIndex)
    {
        var file = gl_files.list[fileIndex];
        
        // Adds the file to the list
        self.files.push(fileIndex);
        var row = $(node.list.children(":eq(" + (self.files.length - 1) + ")"));
        row.html(player.playlist.rowTemplate);
        row.children(".number").html(self.files.length);
        row.children(".name").html(file.name);
        row[0].fileIndex = fileIndex;
        // Updates the player
        node.numerator.html(self.files.length);
        node.denominator.html(self.files.length);
    }
    
    self.init();
    return (self);
}

// Manages the audio player.
self.Audio = function (player)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.audio; // The audio player.
        
        // Default values
        
        // Events
    }
    
    // Sets an audio file to play.
    self.setFile = function (fileIndex)
    {
        // Creates the audio element
        var file = gl_files.list[fileIndex];
        node.audio.html("<audio preload=\"auto\" />");
        self.audio = $(node.audio).children("audio")[0];
        var ogg = "command/audio.ogg?id=" + file.id + getSession();
        var mp3 = "command/audio.mp3?id=" + file.id + getSession();
        var html = "<source src=\"" + ogg + "\" type=\"audio/ogg\" />";
        html += "<source src=\"" + mp3 + "\" type=\"audio/mpeg\" />";
        self.audio.innerHTML = html;
        self.play();
        player.play();
        // Replaces the file name by the title of the music and the artist if possible
        if (file.title)
        {
            var filename = file.title;
            if (file.artist)
                filename += "<span class=\"secondary\"> - " + file.artist + "</span>";
            node.filename.html(filename);
        }
    }
    
    // Clears the audio.
    self.clear = function ()
    {
        $(self.audio).remove();
    }
    
    // Player interface
    {
        self.play = function ()
        {
            self.audio.play();
        }
        
        self.pause = function ()
        {
            self.audio.pause();
        }
        
        self.getMedia = function ()
        {
            return (self.audio);
        }
    }
    
    self.init();
    return (self);
}

    self.init();
    return (self);
}
