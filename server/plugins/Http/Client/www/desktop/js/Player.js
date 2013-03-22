// Manages the file player and the playlist.
var gl_player;

function Player(task)
{
    var self = this;
    var player = self;
    gl_player = self;

// Player methods
{
    self.init = function ()
    {
        // Nodes
        self.node = new Object();
        self.node.bottom = $("#desktop>.bottom");
        self.node.time_line = self.node.bottom.children(".time_line");
        self.node.seek = self.node.time_line.children(".seek");
        self.node.preview = self.node.seek.children(".preview");
        self.node.seek_time = self.node.seek.children(".time");
        self.node.activeArea = self.node.time_line.children(".active_area");
        self.node.player = self.node.bottom.children(".player");
        self.node.playback = self.node.player.children(".playback");
        self.node.controls = self.node.player.children(".controls");
        self.node.number = self.node.playback.children(".number");
        self.node.numerator = self.node.number.children(".numerator"); // The number of the current file played
        self.node.denominator = self.node.number.find(".denominator"); // The total number of files
        self.node.time = self.node.playback.find(".time>span");
        self.node.current_time = self.node.playback.find(".current_time"); // The current time of the file played
        self.node.duration = self.node.playback.find(".duration"); // The duration of the file
        self.node.file_name = self.node.player.children(".file_name");
        self.node.audio = self.node.bottom.children(".audio");
        // Playlist
        self.node.playlist = self.node.bottom.children(".playlist");
        self.node.header = self.node.playlist.children(".header");
        self.node.tabs = self.node.header.children(".tabs");
        self.node.add = self.node.header.children(".add");
        self.node.pin = self.node.header.children(".pin");
        self.node.list = self.node.playlist.children(".list");

        // Members
        self.playback; // Manages the playback buttons
        self.controls; // Manages the control buttons
        self.fileName; // Manages the display of the file name
        self.timeLine; // Manages the time line
        self.playlist; // Manages the playlist
        self.header; // Manages the header of the playlist
        self.audio; // Manages the audio player
        self.height; // The height of the player.
        self.buttonsHeight; // The height of the buttons of the player
        self.mouseLeaveTimeout = 0; // Delays the effect of the mouse leave
        self.mouseOverPlayer; // True while the mouse is over the player
        self.playlistInterface; // The current playlist, from which the current file is played
        self.fileInterface; // The current file in the playlist.
        self.playerInterface; // The file currently played. May be different than the fileInterface.
        self.fullScreenHideTimeout; // Used to hide the player in full screen mode
        
        // Default values
        self.height = C.Player.defaultHeight;
        self.node.bottom.height(self.height);
        self.buttonsHeight = C.Player.defaultHeight - C.Player.TimeLine.height;
        self.fileName = new self.FileName();
        self.playback = new self.Playback();
        self.controls = new self.Controls();
        self.timeLine = new self.TimeLine();
        self.playlist = new self.Playlist();
        self.header = new self.Header();
        self.audio = new self.Audio();
        
        // Events
        self.node.bottom.mouseenter(function (e) { self.mouseEnter(e); });
        self.node.bottom.mouseleave(function (e) { self.mouseLeave(e); });
    }
    
    // Resizes the player when the browser size changes.
    self.onResize = function (width, height)
    {
        self.fileName.onResize(width);
        self.timeLine.onResize();
        self.playlist.onResize(width, height);
    }
    
    // We entered/leaved the full screen mode.
    self.onFullScreen = function (fullScreen)
    {
        self.clearFullScreenTimeout();
        self.timeLine.opaqueTimeLine();
        self.controls.onFullScreen(fullScreen);
        self.displayFullScreen();
        if (!fullScreen)
            gl_desktop.events.unbind("mousedown", self);
        else if (!self.mouseOverPlayer)
            self.hideFullScreen();
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
        // Expands the time line if the playlist is not displayed and there is a media playing
        else if (!self.playlist.isDisplayed() && self.playerInterface)
            self.timeLine.expand();
        // Displays the player if we are in full screen mode
        if (gl_desktop.isFullScreen())
            self.displayFullScreen();
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
            gl_desktop.events.bind("mousedown", self.playlist, function (e)
            {
                gl_desktop.events.unbind("mousedown", self.playlist);
                if (!self.mouseOverPlayer && !self.playlist.isPinned())
                {
                    self.playlist.hide();
                    self.timeLine.retract();
                }
            });
        }
        // Hides the player after the timeout, in full screen mode
        if (gl_desktop.isFullScreen())
            self.fullScreenHideTimeout = setTimeout(function ()
            {
                self.hideFullScreen();
                delete self.fullScreenHideTimeout;
            }, C.Player.FullScreen.displayDuration);
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
    
    // This event is received when a page is displayed or hidden by the desktop.
    self.onDesktopPage = function (display)
    {
        self.timeLine.opaqueTimeLine(display, true);
    }
    
    // Called by the media play event.
    self.play = function ()
    {
        self.playback.setPlay();
    }
    
    // Called by the media pause event.
    self.pause = function ()
    {
        self.playback.setPause();
    }

    // Puts the player in its initial state
    self.clear = function ()
    {
        self.timeLine.clear();
        self.pause();
        if (self.playerInterface)
        {
            $(self.playerInterface.getMedia()).unbind("play");
            $(self.playerInterface.getMedia()).unbind("pause");
        }
        self.playlistInterface = undefined;
        self.fileInterface = undefined;
        self.playerInterface = undefined;
        self.playback.hideTime();
        self.playback.setNumber(0, 0);
        self.fileName.setText();
    }

    // Displays the player, in full screen mode.
    // @param fullScreenTimeout: If the full screen timeout have to be enabled if the mouse is outside of the player.
    self.displayFullScreen = function (fullScreenTimeout)
    {
        self.node.bottom.removeClass("hide");
        self.node.bottom.css("bottom", 0);
        self.clearFullScreenTimeout();
        // Hides the player when the user clicks outside it
        gl_desktop.events.bind("mousedown", self, function(e) {
            if (!$(e.target).parents("#desktop>.bottom").length)
                self.hideFullScreen();
        });
        // Sets the full screen timeout if the mouse is outside of the player
        if (fullScreenTimeout && !self.mouseOverPlayer)
            self.fullScreenHideTimeout = setTimeout(function () {
                self.hideFullScreen();
                delete self.fullScreenHideTimeout;
            }, C.TasksList.FullScreen.displayDuration);
    }
    
    // Hides the player, in full screen mode.
    self.hideFullScreen = function ()
    {
        self.node.bottom.addClass("hide");
        self.node.bottom.css("bottom", -self.height + C.Player.FullScreen.hideHeight);
        if (!self.playlist.isPinned() && self.playlist.isDisplayed())
            self.playlist.hide();
        gl_desktop.events.unbind("mousedown", self);
    }
    
    // Clears the full screen timeout.
    self.clearFullScreenTimeout = function ()
    {
        if (self.fullScreenHideTimeout)
        {
            clearTimeout(self.fullScreenHideTimeout);
            delete self.fullScreenHideTimeout;
        }
    }
}
    
// File interface
// Implemented by all the files.
    {
        // Returns the index of the file, used to get its informations.
        function getFileIndex() {}
        // Changes the playlist of the file.
        function setPlaylist(playlistInterface) {}
    }
    
// Player interface
// Implemented by the audio and video files in addition to the file interface, and used to control their playback.
    {
        // Asks the player to start the playback.
        function play() {}
        // Asks the player to pause the playback.
        function pause() {}
        // Seeks the media to the given time in second (float).
        function seek(time) {}
        // Returns the media of the player (the audio or the video element).
        // media.mediaId : The id of the media, used to communicate with the server.
        // media.timeOffset : The number of second to seek into the media (server side seek).
        function getMedia() {}
    }
    
// Playlist interface
    {
        // Returns the number of files in the playlist, the number of file currently played and its fileIndex.
        function getNumberFiles() { return ({ fileNumber : 0, numberOfFiles : 0, fileIndex : 0 }); }
        // Called by the file interface to tell the playlist that it is ready to play the file.
        function readyToPlay(fileInterface) {}
    }
    
// Player API
    {
        // Plays a file within a playlist. The current playlist is replaced by the one given.
        // @param playlistInterface : The playlist from which the file is played. Muts implement the playlist interface.
        // @param fileInterface : The object that manages the file. Implements the file interface and the player interface if it is an audio or a video file.
        self.playFile = function (playlistInterface, fileInterface)
        {
            self.playlistInterface = playlistInterface;
            self.fileInterface = fileInterface;
            self.setFileName(fileInterface.getFileIndex());
            self.playback.setNumber(playlistInterface.getNumberFiles().fileNumber, playlistInterface.getNumberFiles().numberOfFiles);
            // The file implements the player interface
            if (fileInterface.getMedia)
            {
                // Displays the new player interface
                self.resumeFile(fileInterface);
                // Starts the playback
                self.playerInterface.getMedia().play();
            }
            // Displays the time if there is a player interface
            if (self.playerInterface)
                self.playback.displayTime();
            else
                self.playback.hideTime();
            // Adds the file to the recent files playlist
            self.playlist.addRecentFile(fileInterface.getFileIndex());
        }
        
        // Resumes the playback of a player interface which have been paused earlier.
        self.resumeFile = function (playerInterface)
        {
            if (self.playerInterface != playerInterface || playerInterface == self.audio)
            {
                // There is no playlist
                if (!self.playlistInterface)
                    self.playback.setNumber(1, 1);
                // Pauses the current player interface
                if (self.playerInterface)
                {
                    $(self.playerInterface.getMedia()).unbind("play");
                    $(self.playerInterface.getMedia()).unbind("pause");
                    self.playerInterface.pause();
                    if (gl_files.list[playerInterface.getFileIndex()].type != "audio")
                        self.audio.clear();
                }
                // And replaces it by the new one
                self.playerInterface = playerInterface;
                var media = self.playerInterface.getMedia();
                self.timeLine.newMedia(playerInterface.getFileIndex(), media);
                self.playback.displayTime();
                $(media).bind("play", function (e) { self.play(e); });
                $(media).bind("pause", function (e) { self.pause(e); });
            }
        }
        
        // Removes the current file or player interface.
        self.closeFile = function (fileInterface)
        {
            // The file given is the current one
            if (fileInterface == self.fileInterface)
            {
                // Stops its playback if it is a player interface
                if (fileInterface == self.playerInterface)
                {
                    self.timeLine.clear(true);
                    $(self.playerInterface.getMedia()).unbind("play");
                    $(self.playerInterface.getMedia()).unbind("pause");
                    self.pause();
                    self.playerInterface = undefined;
                }
                // Replaces the closed file by the player interface if there is one
                if (self.playerInterface)
                    self.fileInterface = self.playerInterface;
                // Or just removes the file interface
                else
                    self.fileInterface = undefined;
            }
            // The file is a player interface
            else if (fileInterface == self.playerInterface)
            {
                // So if the current file interface is also a player interface we can replace it
                if (self.fileInterface.getMedia)
                {
                    self.resumeFile(self.fileInterface);
                    self.pause();
                }
                // Otherwise we clear the player interface
                else
                {
                    self.timeLine.clear();
                    $(self.playerInterface.getMedia()).unbind("play");
                    $(self.playerInterface.getMedia()).unbind("pause");
                    self.pause();
                    self.playerInterface = undefined;
                }
                self.setFileName();
            }
            // Displays / hides the time
            if (self.playerInterface)
                self.playback.displayTime();
            else if (self.fileInterface)
                self.playback.hideTime();
            // Nothing to display anymore
            else if (!self.playlistInterface)
                self.clear();
        }
        
        // Closes the current playlist.
        self.closePlaylist = function (playlistInterface)
        {
            if (self.playlistInterface == playlistInterface)
            {
                // Displays the current file alone
                if (self.fileInterface)
                {
                    self.playlistInterface = undefined;
                    self.fileInterface.setPlaylist(undefined);
                    self.playback.setNumber(1, 1);
                    // The player replaces the current file
                    if (self.playerInterface)
                        self.fileInterface = self.playerInterface;
                }
                else
                    self.clear()
            }
        }
        
        // Initializes the global audio player.
        self.openAudio = function (playlistInterface, fileIndex)
        {
            self.audio.setFile(playlistInterface, fileIndex);
        }
        
        // Sets the name of the file displayed by the player.
        // @param fileIndex : If undefined, the name of the current file is used.
        self.setFileName = function (fileIndex)
        {
            if (fileIndex || fileIndex === 0)
                self.fileName.setText(fileIndex);
            else if (self.playerInterface)
                self.fileName.setText(self.playerInterface.getFileIndex());
            else if (self.fileInterface)
                self.fileName.setText(self.fileInterface.getFileIndex());
            else if (self.playlistInterface)
                self.fileName.setText(self.playlistInterface.getNumberFiles().fileIndex);
            else
                self.fileName.setText();
        }
    }

// Manages the playback buttons.
self.Playback = function ()
{
    var self = this;
    var node = player.node;
    
    self.init = function ()
    {
        // Members
        self.C = C.Player.Playback; // The configuration of the playback
        self.paper; // The SVG paper on which the buttons are drawn
        self.slope; // The slope of the buttons
        self.defaultLink; // This link is used when the mouse is outside the buttons areas, but still on the paper
        self.width = self.C.initialPaperWidth; // The total width of the buttons
        // The buttons properties
        self.defaultBackground = {path: 0, left: 0};
        self.play = {icon: 0, background: 0, link: 0, isPlaying: false, mouseOver: false};
        self.pause = {icon: 0, background: 0, link: 0};
        self.previous = {icon: 0, background: 0, link: 0};
        self.number = {left: 0, background: 0, link: 0};
        self.next = {icon: 0, background: 0, link: 0};
        self.time = {background: 0, link: 0, left: 0, display: false};
        
        // Default values
        self.slope = self.C.slopeRatio * player.buttonsHeight;
        node.playback.css("width", self.width);
        self.paper = Raphael(node.playback[0], "100%", player.buttonsHeight);
        node.numerator.html("0");
        node.denominator.html("0");
        node.current_time.html("0:00");
        node.duration.html("0:00");
        self.createButtons();
        self.addEvents();
        self.updatePositions();
        
        // Events
    }
    
    // Creates the buttons.
    self.createButtons = function ()
    {
        var slope = self.slope / 2;
        var left = self.C.margin / 2;
        var top = self.C.top;
        
        // Default link
        self.defaultLink = self.paper.rect(0, 0, 1000, player.buttonsHeight);
        self.defaultLink.attr(self.C.linkAttr);
        
        // Default background
        self.defaultBackground.left = self.C.margin / 2 + self.C.playWidth + self.C.margin - slope - self.C.margin / 2;
        self.defaultBackground.path = self.createBackground(self.defaultBackground.left, self.C.initialDefaultBackgroundWidth, self.C.defaultBackgroundAttr, false);
        
        // Play
        var backgroundLeft = 0;
        var backgroundWidth = left + self.C.playWidth + slope + self.C.margin / 2;
        self.play.background = self.createBackground(backgroundLeft - 100, backgroundWidth + 100, self.C.playBackgroundAttr).hide();
        
        var play = self.paper.path(gl_svg.Player.play);
        play.transform("T" + left + "," + top);
        play.attr(self.C.iconAttr);
        play.glow = play.glow(self.C.iconGlow);
        self.play.icon = play;
        self.play.link = self.play.background.clone().attr(self.C.linkAttr);
        
        // Pause
        var backgroundLeft = 0;
        var backgroundWidth = left + self.C.playWidth + slope + self.C.margin / 2;
        self.pause.background = self.createBackground(backgroundLeft - 100, backgroundWidth + 100, self.C.pauseBackgroundAttr).hide();
        
        var pause = self.paper.path(gl_svg.Player.pause);
        pause.transform("T" + left + "," + top);
        pause.attr(self.C.iconAttr);
        pause.glow = pause.glow(self.C.iconGlow);
        self.pause.icon = pause;
        left += self.C.playWidth;
        pause.hide();
        pause.glow.hide();
        self.pause.link = self.play.link.toFront();
        
        // Previous
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin / 2 + self.C.previousWidth + self.C.numberMargin / 2 + slope * 2;
        self.previous.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.previousBackgroundAttr).hide();
        
        var previous = self.paper.path(gl_svg.Player.previous);
        previous.transform("T" + left + "," + top);
        previous.attr(self.C.iconAttr);
        previous.glow(self.C.iconGlow);
        self.previous.icon = previous;
        left += self.C.previousWidth;
        self.previous.link = self.previous.background.clone().attr(self.C.linkAttr);
        
        // Number
        left += self.C.numberMargin;
        var backgroundLeft = left - self.C.numberMargin / 2 - slope;
        var backgroundWidth = self.C.numberMargin / 2 + self.C.initialNumberTextWidth + self.C.numberMargin / 2 + slope * 2;
        self.number.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.numberBackgroundAttr).hide();
        
        node.number.css("left", left);
        self.number.left = left;
        left += self.C.initialNumberTextWidth;
        self.number.link = self.number.background.clone().attr(self.C.linkAttr);
        
        // Next
        left += self.C.numberMargin;
        var backgroundLeft = left - self.C.numberMargin / 2 - slope;
        var backgroundWidth = self.C.numberMargin / 2 + self.C.nextWidth + self.C.margin / 2 + slope * 2;
        self.next.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.nextBackgroundAttr).hide();
        
        var next = self.paper.path(gl_svg.Player.next);
        next.attr(self.C.iconAttr);
        next.glow = next.glow(self.C.iconGlow);
        next.transform("T" + left + "," + top);
        self.next.icon = next;
        left += self.C.nextWidth;
        self.next.link = self.next.background.clone().attr(self.C.linkAttr);
        
        // Time
        var backgroundLeft = left - self.C.margin / 2 - slope;
        var backgroundWidth = self.C.numberMargin / 2 + node.time.width() + self.C.adjustTextWidth + self.C.margin / 2 + slope * 2;
        self.time.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.timeBackgroundAttr).hide();
        
        left += self.C.margin;
        node.time.css("left", left);
        self.time.link = self.time.background.clone().attr(self.C.linkAttr).hide();
    }
    
    // Adds the events to the buttons.
    self.addEvents = function ()
    {
        // Default link
        $(self.defaultLink.node).mousedown(function (e) { player.playlist.mouseDownName(e); });
    
        // Play / Pause
        $(self.play.link.node).mouseover(function (e)
        {
            self.mouseEnter(self.play.isPlaying ? self.pause : self.play);
            self.play.mouseOver = true;
        });
        $(self.play.link.node).mouseout(function (e)
        {
            self.mouseLeave(self.play.isPlaying ? self.pause : self.play);
            self.play.mouseOver = false;
        });
        $(self.play.link.node).mousedown(function (e) { self.mouseDownPlayPause(); });
        
        // Previous
        $(self.previous.link.node).mouseover(function (e) { self.mouseEnter(self.previous); });
        $(self.previous.link.node).mouseout(function (e) { self.mouseLeave(self.previous); });
        
        // Number
        $(self.number.link.node).mouseover(function (e)
        {
            if (!node.number.hasClass("hover"))
                self.mouseEnter(self.number);
        });
        $(self.number.link.node).mouseout(function (e)
        {
            if (e.relatedTarget != node.number[0] && !$(e.relatedTarget).parentsUntil(node.playback, ".number").length)
                self.mouseLeave(self.number);
        });
        $(node.number).mouseover(function (e)
        {
            if (!node.number.hasClass("hover"))
                self.mouseEnter(self.number);
        });
        $(node.number).mouseout(function (e)
        {
            if (e.relatedTarget != self.number.link.node && e.relatedTarget != node.number[0] && !$(e.relatedTarget).parentsUntil(node.playback, ".number").length)
                self.mouseLeave(self.number);
        });
        
        // Next
        $(self.next.link.node).mouseover(function (e) { self.mouseEnter(self.next); });
        $(self.next.link.node).mouseout(function (e) { self.mouseLeave(self.next); });
        
        // Time
        $(self.time.link.node).mouseover(function (e)
        {
            if (!node.time.hasClass("hover"))
                self.mouseEnter(self.time);
        });
        $(self.time.link.node).mouseout(function (e)
        {
            if (e.relatedTarget != node.time[0] && !$(e.relatedTarget).parentsUntil(node.playback, ".time").length)
                self.mouseLeave(self.time);
        });
        $(node.time).mouseover(function (e)
        {
            if (!node.time.hasClass("hover"))
                self.mouseEnter(self.time);
        });
        $(node.time).mouseout(function (e)
        {
            if (e.relatedTarget != self.time.link.node && e.relatedTarget != node.time[0] && !$(e.relatedTarget).parentsUntil(node.playback, ".time").length)
                self.mouseLeave(self.time);
        });
    }
    
    // Updates the position of the buttons based on the texts width.
    self.updatePositions = function ()
    {
        var slope = self.slope / 2;
        var top = self.C.top;
        var left = self.number.left;
        var numberTextWidth = node.number.width() + self.C.adjustTextWidth;
        var timeTextWidth = node.time.width() + self.C.adjustTextWidth;
        
        // Default background
        var width = self.C.previousWidth + numberTextWidth + self.C.nextWidth + self.C.numberMargin * 2 + slope * 2 + self.C.margin;
        self.updateBackground(self.defaultBackground.path, self.defaultBackground.left, width, false);
        
        // Number
        var backgroundLeft = left - self.C.numberMargin / 2 - slope;
        var backgroundWidth = self.C.numberMargin / 2 + numberTextWidth + self.C.numberMargin / 2 + slope * 2;
        self.updateBackground(self.number.background, backgroundLeft, backgroundWidth);
        self.updateBackground(self.number.link, backgroundLeft, backgroundWidth);
        
        // Next
        left += numberTextWidth + self.C.numberMargin;
        self.next.icon.transform("T" + left + "," + top);
        self.next.icon.glow.transform("T" + left + "," + top);
        var backgroundLeft = left - self.C.numberMargin / 2 - slope;
        var backgroundWidth = self.C.numberMargin / 2 + self.C.nextWidth + self.C.margin / 2 + slope * 2;
        self.updateBackground(self.next.background, backgroundLeft, backgroundWidth);
        self.updateBackground(self.next.link, backgroundLeft, backgroundWidth);
        
        // Time
        left += self.C.nextWidth + self.C.margin;
        var backgroundLeft = left - self.C.margin / 2 - slope;
        var backgroundWidth = self.C.margin / 2 + timeTextWidth + self.C.margin / 2 + slope * 2;
        self.updateBackground(self.time.background, backgroundLeft, backgroundWidth);
        self.updateBackground(self.time.link, backgroundLeft, backgroundWidth);
        node.time.css("left", left);
        self.time.left = left;
        
        // Updates the width of the paper
        if (self.time.display)
            self.setWidth(left + timeTextWidth + self.C.margin / 2 + Math.abs(slope));
        else
            self.setWidth(left - self.C.margin / 2 + Math.abs(slope));
    }
    
    // Creates and returns a button background.
    // @correctGap: If not false, the width is incremented by 1, in order to close gap between two adjacent backgrounds.
    self.createBackground = function (left, width, attr, correctGap)
    {
        if (self.C.correctGap && correctGap !== false)
            width++;
        var path = self.paper.path("M0," + player.buttonsHeight + "L" + self.slope + " 0,H" + width + ",l" + -self.slope + " " + player.buttonsHeight + "z");
        path.transform("T" + left + ",0");
        path.attr(attr);
        return (path);
    }
    // Updates the background position and width.
    // @correctGap: If not false, the width is incremented by 1, in order to close gap between two adjacent backgrounds.
    self.updateBackground = function (background, left, width, correctGap)
    {
        if (self.C.correctGap && correctGap !== false)
            width++;
        background.attr({path: "M0," + player.buttonsHeight + "L" + self.slope + " 0,H" + width + ",l" + -self.slope + " " + player.buttonsHeight + "z"});
        background.transform("T" + left + ",0");
    }
    
    // The mouse entered a button.
    self.mouseEnter = function (button)
    {
        button.background.show();
        if (button == self.number)
            node.number.addClass("hover");
        else if (button == self.time)
            node.time.addClass("hover");
    }
    
    // The mouse leaved a button.
    self.mouseLeave = function (button)
    {
        button.background.hide();
        if (button == self.number)
            node.number.removeClass("hover");
        else if (button == self.time)
            node.time.removeClass("hover");
    }
    
    // The user clicked on the play / pause button.
    self.mouseDownPlayPause = function ()
    {
        var media;
        if (player.playerInterface && (media = player.playerInterface.getMedia()))
        {
            if (media.paused)
                player.playerInterface.play();
            else
                player.playerInterface.pause();
        }
    }
    
    // Displays the pause icon.
    self.setPlay = function ()
    {
        self.play.icon.hide();
        self.play.icon.glow.hide();
        self.pause.icon.show();
        self.pause.icon.glow.show();
        self.play.background.hide();
        self.pause.background.hide();
        if (self.play.mouseOver)
            self.pause.background.show();
        self.play.isPlaying = true;
    }
    
    // Displays the play icon.
    self.setPause = function ()
    {
        self.pause.icon.hide();
        self.pause.icon.glow.hide();
        self.play.icon.show();
        self.play.icon.glow.show();
        self.play.background.hide();
        self.pause.background.hide();
        if (self.play.mouseOver)
            self.play.background.show();
        self.play.isPlaying = false;
    }
    
    // Sets the number numerator and denominator.
    self.setNumber = function (numerator, denominator)
    {
        node.numerator.html(numerator);
        node.denominator.html(denominator);
        self.updatePositions();
    }
    
    // Sets the current time and the duration.
    self.setTime = function (current_time, duration)
    {
        var oldCurrentTime = node.current_time.html();
        var oldDuration = node.duration.html();
        
        current_time = player.timeToString(current_time);
        node.current_time.html(current_time);
        if (duration != undefined)
        {
            duration = player.timeToString(duration);
            node.duration.html(duration);
        }
        if (current_time.length != oldCurrentTime.length || (duration != undefined && duration.length != oldDuration.length))
            self.updatePositions();
    }
    // Displays the time button.
    self.displayTime = function ()
    {
        if (self.time.display)
            return ;
        self.time.display = true;
        node.time.addClass("display");
        self.time.link.show();
        self.setWidth(self.time.left + node.time.width() + self.C.margin / 2 + Math.abs(self.slope / 2));
        self.updatePositions();
    }
    // Hides the time button.
    self.hideTime = function ()
    {
        if (!self.time.display)
            return ;
        self.time.display = false;
        node.time.removeClass("display");
        self.time.background.hide();
        self.time.link.hide();
        self.setWidth(self.time.left - self.C.margin / 2 + Math.abs(self.slope / 2));
    }
    
    // Sets the width of the playback.
    self.setWidth = function (width)
    {
        if (self.width == width)
            return ;
        self.width = width;
        node.playback.css("width", width);
        player.fileName.onResize();
    }
    
    // Returns the total width of the playback.
    self.getWidth = function ()
    {
        return (self.width);
    }
    
    self.init();
    return (self);
}

// Manages the controls buttons.
self.Controls = function ()
{
    var self = this;
    var node = player.node;
    
    self.init = function ()
    {
        // Members
        self.C = C.Player.Controls; // The configuration of the controls
        self.paper; // The SVG paper on which the buttons are drawn
        self.slope; // The slope of the buttons
        self.defaultLink; // This link is used when the mouse is outside the buttons areas, but still on the paper
        self.width = self.C.initialPaperWidth;
        // The buttons properties
        self.defaultBackground;
        self.volume = {icon: 0, background: 0, link: 0};
        self.mute = {icon: 0, background: 0};
        self.settings = {icon: 0, background: 0, link: 0};
        self.repeat = {icon: 0, background: 0, link: 0, previousIcon: 0, currentIcon: 0, nextIcon: 0};
        self.repeatOne;
        self.noRepeat;
        self.random = {icon: 0, background: 0, link: 0, currentIcon: 0, nextIcon: 0};
        self.linear;
        self.fullScreen = {icon: 0, background: 0, link: 0};
        self.normalScreen;
        
        // Default values
        self.slope = self.C.slopeRatio * player.buttonsHeight;
        node.controls.css("width", self.width);
        self.paper = Raphael(node.controls[0], "100%", player.buttonsHeight);
        self.createButtons();
        self.addEvents();
        
        // Events
    }
    
    // Creates the buttons.
    self.createButtons = function ()
    {
        var slope = self.slope / 2;
        var left = 0;
        var top = self.C.top;
        
        // Default link
        self.defaultLink = self.paper.rect(0, 0, 1000, player.buttonsHeight);
        self.defaultLink.attr(self.C.linkAttr);
        
        // Volume
        left += self.C.margin / 2 + Math.abs(slope);
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.volumeWidth + slope * 2;
        self.volume.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.volumeBackgroundAttr).hide();
        
        var volume = self.paper.path(gl_svg.Player.volume);
        volume.transform("T" + left + "," + top);
        volume.attr(self.C.iconAttr);
        volume.glow = volume.glow(self.C.iconGlow);
        self.volume.icon = volume;
        
        self.mute = self.volume;
        self.mute.icon = volume.clone().attr({path: gl_svg.Player.mute});
        self.mute.icon.glow = self.mute.icon.glow = self.mute.icon.glow(self.C.iconGlow);
        self.mute.icon.hide();
        self.mute.icon.glow.hide();
        left += self.C.volumeWidth;
        self.volume.link = self.volume.background.clone().attr(self.C.linkAttr);
        
        // Settings
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.settingWidth + slope * 2;
        self.settings.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.settingsBackgroundAttr).hide();
        
        var settings = self.paper.path(gl_svg.Player.settings);
        settings.transform("T" + left + "," + top);
        settings.attr(self.C.iconAttr);
        settings.glow = settings.glow(self.C.iconGlow);
        self.settings.icon = settings;
        left += self.C.settingWidth;
        self.settings.link = self.settings.background.clone().attr(self.C.linkAttr);
        
        // Repeat
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.repeatWidth + slope * 2;
        var defaultBackgroundLeft = backgroundLeft;
        self.repeat.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.repeatBackgroundAttr).hide();

        var repeat = self.paper.path(gl_svg.Player.repeat);
        repeat.transform("T" + left + "," + top);
        repeat.attr(self.C.iconAttr);
        repeat.glow = repeat.glow(self.C.iconGlow).attr(self.C.iconGlowAttr);
        self.repeat.icon = repeat;
        self.repeat.icon.hide();
        self.repeat.icon.glow.hide();
        
        self.repeatOne = repeat.clone().attr({path: gl_svg.Player.repeatOne});
        self.repeatOne.glow = self.repeatOne.glow(self.C.iconGlow).attr(self.C.iconGlowAttr);
        self.repeatOne.hide();
        self.repeatOne.glow.hide();
        
        self.noRepeat = repeat.clone().attr({path: gl_svg.Player.noRepeat});
        self.noRepeat.glow = self.noRepeat.glow(self.C.iconGlow).attr(self.C.iconGlowAttr);
        
        self.repeat.previousIcon = self.repeatOne;
        self.repeat.currentIcon = self.noRepeat;
        self.repeat.nextIcon = self.repeat.icon;
        self.repeat.nextIcon.attr(self.C.iconAttrInverse);
        self.repeat.nextIcon.glow.attr(self.C.iconGlowInverse);
        left += self.C.repeatWidth;
        self.repeat.link = self.repeat.background.clone().attr(self.C.linkAttr);
        
        // Random
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = self.C.margin + self.C.randomWidth + slope * 2;
        self.random.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.randomBackgroundAttr).hide();

        var random = self.paper.path(gl_svg.Player.random);
        random.transform("T" + left + "," + top);
        random.attr(self.C.iconAttr);
        random.glow = random.glow(self.C.iconGlow);
        self.random.icon = random;
        self.random.icon.hide();
        self.random.icon.glow.hide();
        
        self.linear = random.clone().attr({path: gl_svg.Player.linear});
        self.linear.glow = self.linear.glow(self.C.iconGlow);
        self.random.currentIcon = self.linear;
        
        self.random.nextIcon = self.random.icon;
        self.random.nextIcon.attr(self.C.iconAttrInverse);
        self.random.nextIcon.glow.attr(self.C.iconGlowInverse);
        left += self.C.randomWidth;
        self.random.link = self.random.background.clone().attr(self.C.linkAttr);
        
        // Full screen
        left += self.C.margin;
        var backgroundLeft = left - slope - self.C.margin / 2;
        var backgroundWidth = 1000;
        self.fullScreen.background = self.createBackground(backgroundLeft, backgroundWidth, self.C.fullScreenBackgroundAttr).hide();

        var fullScreen = self.paper.path(gl_svg.Player.fullScreen);
        fullScreen.transform("T" + left + "," + top);
        fullScreen.attr(self.C.iconAttr);
        fullScreen.glow = fullScreen.glow(self.C.iconGlow);
        self.fullScreen.icon = fullScreen;
        
        var normalScreen = fullScreen.clone().attr({path: gl_svg.Player.normalScreen});
        normalScreen.glow = normalScreen.glow(self.C.iconGlow);
        normalScreen.hide();
        normalScreen.glow.hide();
        self.normalScreen = normalScreen;
        
        left += self.C.fullScreenWidth;
        self.fullScreen.link = self.fullScreen.background.clone().attr(self.C.linkAttr);
        
        // Default background
        var backgroundLeft = defaultBackgroundLeft;
        var backgroundWidth = self.C.margin * 2 + self.C.repeatWidth + self.C.randomWidth + slope * 2;
        self.defaultBackground = self.createBackground(backgroundLeft, backgroundWidth, self.C.defaultBackgroundAttr, false);
        self.defaultBackground.toBack();
        
        // The width of the paper
        self.setWidth(left + self.C.margin / 2);
    }
    
    // Adds the events to the buttons.
    self.addEvents = function ()
    {
        // Default link
        $(self.defaultLink.node).mousedown(function (e) { player.playlist.mouseDownName(e); });
        
        // Volume
        $(self.volume.link.node).mouseover(function (e) { self.mouseEnter(self.volume); });
        $(self.volume.link.node).mouseout(function (e) { self.mouseLeave(self.volume); });
        
        // Settings
        $(self.settings.link.node).mouseover(function (e) { self.mouseEnter(self.settings); });
        $(self.settings.link.node).mouseout(function (e) { self.mouseLeave(self.settings); });
        
        // Repeat
        $(self.repeat.link.node).mouseover(function (e)
        {
            self.mouseEnter(self.repeat);
            self.repeat.currentIcon.attr(self.C.iconAttrInverse);
            self.repeat.currentIcon.glow.attr(self.C.iconGlowInverse);
        });
        $(self.repeat.link.node).mouseout(function (e)
        {
            self.mouseLeave(self.repeat);
            self.repeat.currentIcon.attr(self.C.iconAttr);
            self.repeat.currentIcon.glow.attr(self.C.iconGlowAttr);
        });
        $(self.repeat.link.node).mousedown(function (e)
        {
            var previous = self.repeat.previousIcon;
            self.repeat.previousIcon = self.repeat.currentIcon;
            self.repeat.currentIcon = self.repeat.nextIcon;
            self.repeat.nextIcon = previous;
            
            self.repeat.previousIcon.hide();
            self.repeat.previousIcon.glow.hide();
            self.repeat.currentIcon.show().attr(self.C.iconAttrInverse);
            self.repeat.currentIcon.glow.show().attr(self.C.iconGlowInverse);
        });
        
        // Random
        $(self.random.link.node).mouseover(function (e)
        {
            self.mouseEnter(self.random);
            self.random.currentIcon.attr(self.C.iconAttrInverse);
            self.random.currentIcon.glow.attr(self.C.iconGlowInverse);
        });
        $(self.random.link.node).mouseout(function (e)
        {
            self.mouseLeave(self.random);
            self.random.currentIcon.attr(self.C.iconAttr);
            self.random.currentIcon.glow.attr(self.C.iconGlowAttr);
        });
        $(self.random.link.node).mousedown(function (e)
        {
            var next = self.random.currentIcon;
            self.random.currentIcon = self.random.nextIcon;
            self.random.nextIcon = next;
            
            self.random.currentIcon.show().attr(self.C.iconAttrInverse);
            self.random.currentIcon.glow.show().attr(self.C.iconGlowInverse);
            self.random.nextIcon.hide();
            self.random.nextIcon.glow.hide();
        });
        
        // Full screen
        $(self.fullScreen.link.node).mouseover(function (e) { self.mouseEnter(self.fullScreen); });
        $(self.fullScreen.link.node).mouseout(function (e) { self.mouseLeave(self.fullScreen); });
        $(self.fullScreen.link.node).mousedown(function (e)
        {
            gl_desktop.setFullScreen();
        });
    }
    
    // Creates and returns a button background.
    // @correctGap: If not false, the width is incremented by 1, in order to close gap between two adjacent backgrounds.
    self.createBackground = function (left, width, attr, correctGap)
    {
        if (self.C.correctGap && correctGap !== false)
            width++;
        var path = self.paper.path("M0," + player.buttonsHeight + "L" + self.slope + " 0,H" + width + ",l" + -self.slope + " " + player.buttonsHeight + "z");
        path.transform("T" + left + ",0");
        path.attr(attr);
        return (path);
    }
    
    // The mouse entered a button.
    self.mouseEnter = function (button)
    {
        button.background.show();
    }
    
    // The mouse leaved a button.
    self.mouseLeave = function (button)
    {
        button.background.hide();
    }
    
    // Sets the width of the controls.
    self.setWidth = function (width)
    {
        self.width = width;
        node.controls.css("width", width);
    }
    
    // Returns the total width of the controls.
    self.getWidth = function ()
    {
        return (self.width);
    }
    
    // The full screen mode has changed.
    self.onFullScreen = function (fullScreen)
    {
        if (fullScreen)
        {
            self.normalScreen.show();
            self.normalScreen.glow.show();
            self.fullScreen.icon.hide();
            self.fullScreen.icon.glow.hide();
        }
        else
        {
            self.fullScreen.icon.show();
            self.fullScreen.icon.glow.show();
            self.normalScreen.hide();
            self.normalScreen.glow.hide();
        }
    }
    
    self.init();
    return (self);
}

// Manages the display of the file name.
self.FileName = function ()
{
    var self = this;
    var node = player.node;
    
    self.init = function ()
    {
        // Members
        self.C = C.Player.FileName; // The configuration of the file name
        self.paper; // The SVG paper on which the background is drawn
        self.slope; // The slope of the buttons
        self.text = node.file_name.children(".text");
        self.primary = self.text.find(".primary"); // {originalText} contains the full primary name
        self.secondary = self.text.find(".secondary"); // {originalText} contains the full secondary name
        self.text_width = self.text.find(".text_width");
        self.background = node.file_name.children(".background");
        self.fullTextWidth; // The width of the unshortened text
        self.path; // The SVG path of the background
        self.displayed; // True while the file name is displayed
        self.textShortened; // True if the text have been shortened to fit between the buttons
        
        // Default values
        self.slope = self.C.slopeRatio * player.buttonsHeight;
        self.paper = Raphael(self.background[0], "100%", player.buttonsHeight);
        self.path = self.createBackground(self.C.initialWidth);
        self.setText();
        
        // Events
        node.file_name.mousedown(function (e) { player.playlist.mouseDownName(e); });
        node.player.mousedown(function (e) { if (e.target == node.player[0]) player.playlist.mouseDownName(e); });
    }
    
    // Creates the background.
    self.createBackground = function (width)
    {
        var path = self.paper.path("M0," + player.buttonsHeight + "L" + self.slope + " 0,H" + width + ",l" + self.slope + " " + player.buttonsHeight + "z");
        path.transform("T" + (-self.slope / 2 + Math.abs(self.slope / 2)) + ",0");
        path.attr(self.C.attr);
        return (path);
    }
    
    // Sets the width of the background and the text.
    self.setWidth = function (width)
    {
        self.path.attr({path: "M0," + player.buttonsHeight + "L" + self.slope + " 0,H" + width + ",l" + self.slope + " " + player.buttonsHeight + "z"});
        self.path.transform("T" + (-self.slope / 2 + Math.abs(self.slope / 2)) + ",0");
        self.background.css("width", width + Math.abs(self.slope));
        node.file_name.css("width", width + Math.abs(self.slope));
        self.text.css("width", width - self.C.padding * 2);
    }
    
    // The player or the playback buttons width have changed,
    // so we update the position and width of the text accordingly.
    self.onResize = function ()
    {
        if (!self.displayed)
            return ;
        var playerWidth = gl_browserSize.width;
        var playbackWidth = player.playback.getWidth();
        var controlsWidth = player.controls.getWidth();
        var fullWidth = self.fullTextWidth + self.C.padding * 2 + Math.abs(self.slope);
        var width = fullWidth - Math.abs(self.slope) * 2;
        var left = playerWidth / 2 - width / 2 - playbackWidth;
        var right = playerWidth / 2 - width / 2 - controlsWidth;
        var diff = playerWidth - width - playbackWidth - controlsWidth;
        var centerLeft = playerWidth / 2 - width / 2 - Math.abs(self.slope);
        
        // We don't have enougth space to display all the text, so we shorten it
        if (diff < 0)
        {
            node.file_name.css("left", playbackWidth - Math.abs(self.slope));
            var width = fullWidth + diff - Math.abs(self.slope);
            self.setWidth(width);
            self.shortenText(width - self.C.padding * 2);
            self.textShortened = true;
        }
        // No more space on the left
        else if (left < 0)
            node.file_name.css("left", centerLeft - left);
        // No more space on the right
        else if (right < 0)
            node.file_name.css("left", centerLeft + right);
        // The text can be enterly displayed
        else
            node.file_name.css("left", centerLeft);
        // Puts the width of the text back to its full length
        if (diff >= 0 && self.textShortened)
        {
            node.file_name.removeClass("hide");
            self.setWidth(fullWidth - Math.abs(self.slope));
            self.primary.html(self.primary.originalText);
            self.secondary.html(self.secondary.originalText);
            self.textShortened = false;
        }
    }
    
    // Shortens the text in order to fit within the given width.
    self.shortenText = function (width)
    {
        self.primary.html(self.primary.originalText);
        var primary = self.primary.width();
        
        node.file_name.removeClass("hide");
        // Shortens the secondary name
        if (width > primary)
        {
            self.secondary.html(self.secondary.originalText);
            var secondary = self.primary.width();
            var text = self.secondary.originalText;
            self.secondary.html(text.substr(0, text.length * width / (primary + secondary)) + self.C.shortenEndString);
            while (width - primary < self.secondary.width() && text.length > self.C.shortenEndString.length)
            {
                text = self.secondary.html();
                self.secondary.html(text.substr(0, text.length - self.C.shortenEndString.length - 1) + self.C.shortenEndString);
            }
            if (width - primary >= self.secondary.width())
                return ;
        }
        // Shortens the primary name
        self.secondary.html("");
        var text = self.primary.originalText;
        self.primary.html(text.substr(0, text.length * width / primary) + self.C.shortenEndString);
        while (width < self.primary.width() && text.length > self.C.shortenEndString.length)
        {
            text = self.primary.html();
            self.primary.html(text.substr(0, text.length - self.C.shortenEndString.length - 1) + self.C.shortenEndString);
        }
        // There is not enougth space to display the text so we hide it
        if (self.primary.html().length <= self.C.shortenEndString.length)
            node.file_name.addClass("hide");
    }
    
    // Sets the file name.
    self.setText = function (fileIndex)
    {
        // Hides the file name
        if (fileIndex == undefined)
        {
            self.displayed = false;
            self.path.hide();
            self.primary.html("");
            self.secondary.html("");
            return ;
        }
        // Gets the texts
        var file = gl_files.list[fileIndex]
        var primary = "";
        var secondary = "";
    
        if (file.title)
            primary = file.title;
        else
            primary = file.name;
        if (file.artist)
            secondary = self.C.separatorString + file.artist;
        // This file is already displayed
        if (primary == self.primary.originalText && secondary == self.secondary.originalText)
            return ;
        self.primary.html(primary);
        self.secondary.html(secondary);
        self.primary.originalText = primary;
        self.secondary.originalText = secondary;
        // Displays the file name
        self.displayed = true;
        self.path.show();
        self.fullTextWidth = self.text_width.width();
        self.setWidth(self.fullTextWidth + self.C.padding * 2);
        self.onResize();
    }
    
    self.init();
    return (self);
}

// Manages the time line.
self.TimeLine = function ()
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.C = C.Player.TimeLine; // The configuration of the time line.
        self.media; // The media currently played
        self.duration; // The duration of the media currently played
        self.mouseLeaveTimeout = 0; // Delays the effect of the mouse leaving the active area
        self.before = 0; // The percentage of the before part of the time line
        self.played = 0; // The percentage of the media played
        self.buffered = 0; // The percentage of the media buffered
        self.previewHeight; // The height of the preview if it is displayed.
        self.currentPreviewTime; // The time of the preview currently displayed.
        self.timeOffset = 0; // The offset that have to be applied to the time line due to the server side seeking
        self.fileIndex; // The index of the file displayed by the time line
        self.fileType; // The type of the file currently played
        self.desktopPage; // True while a page is displayed on the desktop.
        
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
        var height = self.C.height + self.C.expandHeight;
        var paper = Raphael(node.time_line[0], gl_browserSize.width, self.C.height);
        var svg = $(node.time_line).children("svg");
        
        svg.css("position", "absolute");
        var before = paper.rect(0, 0, 1, height);
        before.attr(self.C.before.normal);
        var played = paper.rect(0, 0, 1, height);
        played.attr(self.C.played.normal);
        var buffered = paper.rect(0, 0, 1, height);
        buffered.attr(self.C.buffered.normal);
        var after = paper.rect(0, 0, 1, height);
        after.attr(self.C.after.normal);

        // Updates the time line SVG.
        var drawElement;
        self.updateTimeLine = function ()
        {
            var transform = { translate : 0, scale : 0 };

            drawElement(before, self.before, transform);
            drawElement(played, self.played, transform);
            drawElement(buffered, self.buffered, transform);
            drawElement(after, 1, transform);
            paper.setSize(gl_browserSize.width);
        }
        // Draws an element of the time line.
        drawElement = function (element, percent, transform)
        {
            transform.translate += transform.scale;
            transform.scale = gl_browserSize.width * percent;
            if (transform.scale > 0)
                element.transform("S" + transform.scale + ",1,0,0T" + transform.translate + ",0");
            else
                element.transform("T0,1000");
        }
        
        // Expands the time line.
        self.expandTimeLine = function ()
        {
            paper.setSize(undefined, height);
            svg.css("top", "-" + self.C.expandHeight + "px");
        }
        
        // Retracts the time line.
        self.retractTimeLine = function ()
        {
            paper.setSize(undefined, self.C.height);
            svg.css("top", "0px");
        }
        
        // Makes the time line opaque if the playlist is displayed or a page is on the desktop and we are not in full screen mode.
        self.opaqueTimeLine = function (opaque, desktop)
        {
            if (desktop)
                self.desktopPage = opaque;
            if ((self.desktopPage && !gl_desktop.isFullScreen()) || (player.playlist && player.playlist.isDisplayed()))
            {
                before.attr(self.C.before.opaque);
                played.attr(self.C.played.opaque);
                buffered.attr(self.C.buffered.opaque);
                after.attr(self.C.after.opaque);
            }
            else
            {
                before.attr(self.C.before.normal);
                played.attr(self.C.played.normal);
                buffered.attr(self.C.buffered.normal);
                after.attr(self.C.after.normal);
            }
            self.setTimeLineType();
        }
        
        // Changes the color of the played part based on the file type.
        self.setTimeLineType = function ()
        {
            if (self.fileType == "audio")
            {
                before.attr(self.C.before.audio);
                played.attr(self.C.played.audio);
            }
            else if (self.fileType == "video")
            {
                before.attr(self.C.before.video);
                played.attr(self.C.played.video);
            }
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
    self.onResize = function ()
    {
        self.updateTimeLine();
        // Ensures that there is nothing bellow the time line
        if (self.isExpanded() && !player.playlist.isDisplayed())
            gl_desktop.node.middle_area.css({height: gl_desktop.height - self.C.expandHeight});
        else
            gl_desktop.node.middle_area.css({height: "100%"});
    }
    
    // Changes the media displayed in the time line.
    self.newMedia = function (fileIndex, media)
    {
        var file = gl_files.list[fileIndex];
        self.clear();
        self.media = media;
        self.fileIndex = fileIndex;
        self.duration = file.duration;
        player.playback.setTime(0, self.duration);
        self.timeOffset = media.timeOffset;
        self.fileType = file.type;
        self.setTimeLineType();
        if (player.mouseOverPlayer)
            self.expand();
        if (file.type == "video")
            self.previewHeight = Math.round(C.Player.Seek.previewWidth * file.height / file.width);
        else
            self.previewHeight = 0;
        self.currentPreviewTime = undefined;
        $(media).bind("timeupdate", function (e) { self.onTimeUpdate(e); });
        $(media).bind("progress", function (e) { self.onTimeUpdate(e); });
        self.onTimeUpdate();
    }
    
    // Clears the time line.
    // @param keepDuration : True if the duration have to be keeped.
    self.clear = function (keepDuration)
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
        player.playback.setTime(0, keepDuration ? undefined : 0);
    }
    
    // The time of the media played has changed.
    // Updates the time line based on the new time.
    self.onTimeUpdate = function ()
    {
        var currentTime = self.media.currentTime;
        var buffered = 0;
        if (self.media.buffered.length)
            buffered = self.media.buffered.end(0) - self.media.buffered.start(0) - currentTime;
        self.before = self.timeOffset / self.duration;
        self.played = currentTime / self.duration;
        self.buffered = buffered / self.duration;
        self.updateTimeLine();
        player.playback.setTime(currentTime + self.timeOffset);
    }
    
    // Mouse enter the time line.
    self.mouseEnter = function (e)
    {
        if (!self.media)
            return ;
        node.time_line.addClass("over");
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
        node.time_line.removeClass("over");
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
        var left = e.pageX - width / 2 - C.Player.Seek.border;
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
                self.setPreview("/c/command/preview?fileId=" + gl_files.list[self.fileIndex].id + "&width=" + width + "&height=" + self.previewHeight + "&position=" + previewTime + F.getSession());
            self.currentPreviewTime = previewTime;
        }
    }
    
    // Seeks to the position on the cursor.
    self.mouseDown = function (e)
    {
        if (!self.media || e.which != 1)
            return ;
        var time = e.pageX / gl_browserSize.width * self.duration - self.timeOffset;
        // Tries to seek on the data already received
        if (self.media.seekable.length > 0 && self.media.seekable.end(0) > time)
            self.media.currentTime = time;
        // Otherwise we perform a server side seeking
        if (self.media.currentTime != time)
        {
            player.playerInterface.seek(time + self.timeOffset);
            self.timeOffset = time + self.timeOffset;
            self.onTimeUpdate();
        }
    }
    
    // Expands the time line.
    self.expand = function ()
    {
        if (self.isExpanded())
            return ;
        self.expandTimeLine();
        node.time_line.addClass("expand");
        self.onResize();
    }
    
    // Retracts the time line.
    self.retract = function ()
    {
        self.retractTimeLine();
        node.time_line.removeClass("expand");
        self.onResize();
    }
    
    // Returns trus if the time line is expanded.
    self.isExpanded = function ()
    {
        return (node.time_line.hasClass("expand"));
    }
    
    self.init();
    return (self);
}

// Manages the playlist.
self.Playlist = function ()
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.height; // The height of the playlist
        self.pinned; // True if the playlist is pinned
        self.rowTemplate = node.playlist.children(".row_template").html(); // The template used to create the rows.
        self.recentFilesPlaylist; // This playlist stores all the files recently played. It is created automatically and filled by addRecentFile.
        self.oldPlaylistHeight; // Saves the height of the playlist when it is pinned.
        
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
        else if (height > gl_desktop.height - C.Player.TimeLine.expandHeight)
            height = gl_desktop.height - C.Player.TimeLine.expandHeight;
        // Resizes the playlist
        var listHeight = height - C.Player.headerHeight;
        self.height = height;
        node.playlist.height(height);
        node.list.height(listHeight);
        node.playlist[0].style.top = -(height + C.Player.TimeLine.expandHeight) + "px";
        return (listHeight);
    }
    // Sets the height in the pinned mode.
    self.setHeightPinned = function (height)
    {
        // Clamp the height
        if (height <= C.Player.headerHeight)
            height = C.Player.headerHeight + 1;
        else if (gl_browserSize.height - (height + C.Player.defaultHeight + C.Player.TimeLine.expandHeight) < gl_header.height)
            height = gl_browserSize.height - (C.Player.defaultHeight + C.Player.TimeLine.expandHeight + gl_header.height);
        // Resizes the playlist
        var listHeight = height - C.Player.headerHeight;
        self.height = height;
        node.playlist.height(height);
        node.list.height(listHeight);
        // Updates the desktop
        player.height = self.height + C.Player.defaultHeight + C.Player.TimeLine.expandHeight;
        node.bottom.css("padding-top", self.height + C.Player.TimeLine.expandHeight);
        gl_desktop.setBottom(player.height);
        return (listHeight);
    }
    
    // Resizes the playlist when the browser size changes.
    self.onResize = function (width, height)
    {
        node.playlist.width(width);
        // Checks if the playlist is highter than the top part of the desktop
        if (!self.pinned)
        {
            if (self.height > gl_desktop.height - C.Player.TimeLine.expandHeight)
                self.setHeight(self.height);
        }
        else if (height - (self.height + C.Player.defaultHeight + C.Player.TimeLine.expandHeight) < gl_header.height)
            self.setHeight(self.height);
    }

    // Displays the playlist.
    self.display = function ()
    {
        node.playlist.addClass("display");
        node.playlist.height(self.height);
        player.timeLine.expand();
        player.timeLine.opaqueTimeLine(true);
    }

    // Hides the playlist.
    self.hide = function ()
    {
        node.playlist.removeClass("display");
        node.playlist.height(0);
        if (!player.playerInterface)
            player.timeLine.retract();
        player.timeLine.opaqueTimeLine(false);
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
        node.bottom.css("padding-top", 0);
        node.playlist.css("top", -(self.height + C.Player.TimeLine.expandHeight));
        self.hide();
        player.timeLine.retract();
        player.height = C.Player.defaultHeight;
        gl_desktop.setBottom(player.height);
    }
    
    // Returns true if the playlist is pinned to the player.
    self.isPinned = function ()
    {
        return (self.pinned);
    }
    
    // Adds a file to the recent files playlist.
    self.addRecentFile = function (fileIndex)
    {
        // Creates the recent files playlist if it doesn't exists
        if (!player.header.tabFocused.length)
            self.recentFilesPlaylist = new player.Tab(T.Player.recentFiles);
        // Adds the file to the current playlist
        player.header.tabFocused[0].object.addFile(fileIndex);
    }
    
    // Mouse down on the name area. Opens / closes the playlist.
    self.mouseDownName = function (e)
    {
        if (e.which != 1)
            return ;
        // Opens / closes the playlist
        if (!self.isPinned())
        {
            if (!self.isDisplayed())
                self.display();
            else
                self.hide();
        }
        // If the playlist is pinned we resize it to the minimal height instead of hiding it
        else
        {
            if (self.height <= C.Player.headerHeight + 1)
            {
                if (!self.oldPlaylistHeight)
                    self.oldPlaylistHeight = C.Player.playlistHeight;
                self.setHeight(self.oldPlaylistHeight);
            }
            else
            {
                self.oldPlaylistHeight = self.height;
                self.setHeight(0);
            }
        }
    }
    
    self.init();
    return (self);
}

// Manages the headers of the playlist, which includes the tabs, the pin and the resize.
self.Header = function ()
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
                new player.Tab("New playlist " + (node.tabs.children().length + 1));
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
        if (e.which != 1 || !$(e.target).hasClass("header"))
            return ;
        gl_desktop.drag.start(e, node.header[0], self, "mouseMoveResize");
        gl_desktop.drag.setCursor("n-resize");
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

// Creates and manages a tab and the playlist it represents.
// Implements the playlist interface.
// @param name : The name of the tab.
self.Tab = function (name)
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.tab = self.create(name); // The tab element
        self.tab[0].object = self; // Allows to access the object from the tab element
        self.files = new Array(); // The list of the files in the playlist
        self.fileNumber = 0; // The number of the last file played in the playlist
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
        // This playlist is the recent files one
        if (player.playlist.recentFilesPlaylist == self)
            player.playlist.recentFilesPlaylist = undefined;
    }

    // Starts the dragging of the tab.
    self.mouseDown = function (e)
    {
        gl_desktop.drag.start(e, self.tab[0], self, "mouseMove", "", "mouseUpMove");
        gl_desktop.drag.setCursor("pointer");
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
        
        self.files.push(fileIndex);
        var row = $(node.list.children(":eq(" + (self.files.length - 1) + ")"));
        row.html(player.playlist.rowTemplate);
        row.children(".number").html(self.files.length);
        row.children(".name").html(file.name);
        row[0].fileIndex = fileIndex;
        self.fileNumber = self.files.length;
    }
    
// Playlist interface
    {
        self.getNumberFiles = function ()
        {
            return ({ numerator : self.fileNumber, denominator : self.files.length });
        }
        
        self.readyToPlay = function (playerInterface)
        {
            gl_player.playFile(self, playerInterface);
        }
    }
    
    self.init();
    return (self);
}

// Manages the audio player.
self.Audio = function ()
{
    var self = this;
    var node = player.node; 
    
    self.init = function ()
    {
        // Members
        self.audio = $(node.audio).children("audio")[0]; // The audio player
        self.audio.mediaId; // The id of the media, used to communicate with the server
        self.audio.timeOffset = 0; // The server side seek
        self.format = "ogg"; // The format of the audio
        self.playlistInterface; // The playlist from which the audio is played
        self.fileIndex; // The index of the current file
        
        // Checks the supported audio formats
        if (self.audio.canPlayType("audio/ogg"))
            self.format = "ogg";
        else if (self.audio.canPlayType("audio/mpeg"))
            self.format = "mp3";
        // The browser can't play ogg and mp3
        else
            ;
    }
    
    // Sets an audio file to play.
    self.setFile = function (playlistInterface, fileIndex)
    {
        var file = gl_files.list[fileIndex];
        
        self.clear();
        self.fileIndex = fileIndex;
        // Sets the source of the audio element
        self.audio.mediaId = F.getUuid();
        self.audio.src = "/c/command/audio." + self.format + "?fileId=" + file.id + "&mediaId=" + self.audio.mediaId + F.getSession();
        self.playlistInterface = playlistInterface;
        playlistInterface.readyToPlay(self);
    }
    
    // Clears the audio.
    self.clear = function ()
    {
        if (self.audio.src)
        {
            self.audio.pause();
            self.audio.src = "";
            F.request("GET", "command/audio/stop?mediaId=" + self.audio.mediaId);
            self.audio.mediaId = undefined;
            self.audio.timeOffset = 0;
        }
    }
    
// Player and file interfaces
    {
        self.getFileIndex = function ()
        {
            return (self.fileIndex);
        }
        
        self.setPlaylist = function (playlistInterface)
        {
            self.playlistInterface = playlistInterface;
        }
        
        self.play = function ()
        {
            self.audio.play();
        }
        
        self.pause = function ()
        {
            self.audio.pause();
        }
        
        self.seek = function (time)
        {
            var paused = self.audio.paused;
            
            self.clear();
            self.audio.timeOffset = time;
            self.audio.mediaId = F.getUuid();
            self.audio.src = "/c/command/audio." + self.format + "?fileId=" + gl_files.list[self.fileIndex].id + "&mediaId=" + self.audio.mediaId + "&start=" + time + F.getSession();
            if (!paused)
                self.audio.play();
        }
        
        self.getMedia = function ()
        {
            return (self.audio);
        }
        
        self.setPlaylist = function (playlistInterface)
        {
            self.playlistInterface = playlistInterface;
        }
    }
    
    self.init();
    return (self);
}

    self.init();
    return (self);
}
