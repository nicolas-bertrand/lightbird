// The configuration of the Client.
var Configuration =
{
	animationFPS: 20, // Default animation FPS.
    focusBeforeAction: true, // If the page have to be focused before doing any action when the mouse clicks on a task.
    // The configuration of the desktop.
    Desktop:
    {
        minWidth: 650, // The minimum width of the desktop.
        minHeight: 300, // The minimum height of the desktop.
        browserFullScreen: true, // If we have to use the browser full screen API.
        stopDragLeaveDesktop: false, // If the current drag have to be stopped when the mouse leaves the browser.
        pageMargin: 10, // The height of the bottom margin between the pages.
        tasksListWidth: 120, // The width of the tasks list.
        tasksListMargin: 10, // The width of the left margin of the tasks list.
        taskIconHeight: 75, // The height of a task icon.
        taskResistance: 3, // Number of pixels a task icon have to be dragged before actually moving.
        taskDragShift: { x: 10, y: 10 }, // The shift of the dragged task when when it is out of the tasks list.
        tasksListScrollHeight: 50, // Height of the scrolling areas of the tasks list.
        tasksListScrollSpeed: 20, // The speed of the scroll areas of the tasks list.
        tasksListScrollFps: 1000 / 30, // The number of time per second the scroll is performed.
        mouseWheelMultiplier: 50, // Used to normalize the mouse wheel delta. Increases the delta of each click.
        defaultPosition: "e", // The default position of a task moved (n s e w).
        insertTaskAreaSize: 50, // The size of the page border areas that allows to insert a task.
        taskMargin: 5, // The margin between the tasks content in a page. It is also the size of the resize task bars.
        taskBorder: 1, // The size of the border of the tasks content.
        resizeTaskLimitMin: 0, // The minimum ratio by which a task can be resized. Between 0 and 1.
        resizeTaskLimitMax: 1, // The maximum ratio by which a task can be resized. Between 0 and 1.
        resizeResistance: 50, // The number of pixels the mouse need to move before the resize of the tasks starts.
        resizeDeltaDivisor: 20 // Reduce the delta of the mouse wheel that resize the task margin.
    },
	Window:
	{
        default: // The default position of the window
        {
            left: 200,
            top: 100,
            width: 600,
            height: 400
        },
        limit: // The limits of the window when it is moved or resized
        {
            top: 8,
            bottom: 28
        },
        minWidth: 162,
        minHeight: 35,
        border: 5, // The size of the border of the window.
        topHeight: 35, // The height of the top of the window.
        bottomHeight: 30 // The height of the bottom of the window.
	},
    Player:
    {
        defaultHeight: 45, // The default height of the player.
        playlistHeight: 200, // The default height of the playlist.
        headerHeight: 24, // The height of the playlist header.
        listFileHeight: 25, // The height of each file of the playlist.
        tabOrigin: -30, // The original position of the tabs.
        tabMargin: 16, // The margin between the tabs.
        addIconMargin: 40, // The margin between the last tab and the add icon.
        addIconOrigin: -16, // The origin position of the add icon.
        tabOffsetInterval: 10, // The duration of the intervals used to get the offset* of the tabs.
        tabExternLeft: 16, // The estimation of the width of the external left part of the tab, based on its shape.
        tabExternRight: 21, // The estimation of the width of the external right part of the tab, based on its shape.
        tabDragResistance: 3, // The number of pixels the mouse needs to move in order to start the dragging.
        playlistResizeResistance: 20, // The number of pixels the mouse needs to move in order to start the resizing.
        playlistResizeEdges: -2, // The distance from the edge of the header where the resize takes effect.
        mouseLeaveTimeout: 1000, // The delay before the effect of the mouse leave.
        Seek:
        {
            border: 20, // The width of the borders (not visible).
            topPosition: 15, // The position of the seek box above the time line.
            positionLimit: 1, // The number of pixels the seek box can exceed left and right relative to the browser width.
            height: 31, // The default height of the box.
            radius: 5, // The radius of the box corners.
            previewWidth: 200, // The width of the preview displayed above the box.
            numberPreviews: 100 // The number of previews generated for each videos.
        },
        Playback:
        {
            slopeRatio: 75 / 130, // The ratio of the slope of the backgrounds.
            margin: 30, // The margin between the buttons.
            numberMargin: 20, // The margin between the number and the next / previous buttons.
            playWidth: 16, // Width of the play icon.
            previousWidth: 14,  // Width of the previous icon.
            nextWidth: 14,  // Width of the next icon.
            top: 10, // The top position of the icons.
            iconAttr: {fill: "#eeeeee", stroke: "none"}, // The attributes of the icons.
            iconGlow: {color: "black", width: 5, opacity: 0.15}, // The attributes of the glow of the icons.
            updateTextsWidthDuration: 500, // The maximum duration during which we try to get the width of the number and time texts.
            updateTextsWidthSteps: 10, // The number of attempts to get the width of the texts during the duration.
            // The attributes of the buttons backgrounds
            defaultBackgroundAttr: {fill: "90-#2b2b2b-#404040", stroke: "none"}, // This background is always displayed bellow previous / number / next.
            playBackgroundAttr: {fill: "90-#1b8828-#24b335", stroke: "none"},
            pauseBackgroundAttr: {fill: "90-#b62e14-#f13d1a", stroke: "none"},
            previousBackgroundAttr: {fill: "90-#0075c0-#009bfe", stroke: "none"},
            numberBackgroundAttr: {fill: "90-#d9d9d9-#ffffff", stroke: "none"},
            nextBackgroundAttr: {fill: "90-#0075c0-#009bfe", stroke: "none"},
            timeBackgroundAttr: {fill: "90-#d9d9d9-#ffffff", stroke: "none"},
            linkAttr: {fill: "white", opacity: 0}, // This transparent background is used to detect when the mouse enters or clicks on a button.
            initialNumberTextWidth: 34, // The initial width of the number, before the text widths computation is performed.
            initialDefaultBackgroundWidth: 137, // The initial width of the default background, before the texts width computation is performed.
            initialPaperWidth: 300,  // The initial width of the SVG paper, before the texts width computation is performed.
            adjustTextWidth: -1, // Browsers seems to add on extra pixel when calculating the width of a text.
            correctGap: false // If the width of the background have to be incremented by one to close a gap between adjacent backgrounds.
        },
        Controls:
        {
            slopeRatio: -75 / 130, // The ratio of the slope of the backgrounds. // -Math.sin(30 * (Math.PI / 180)) / Math.cos(30 * (Math.PI / 180))
            margin: 30, // The margin between the buttons.
            top: 10, // The top position of the icons.
            iconAttr: {fill: "#eeeeee", stroke: "none"}, // The attributes of the icons.
            iconGlow: {width: 5, opacity: 0.15}, // The attributes of the icons glows.
            iconGlowAttr: {stroke: "black"}, // The attributes of the icons glows.
            iconAttrInverse: {fill: "#333333", stroke: "none"}, // The inverse attributes of the icons.
            iconGlowInverse: {stroke: "white"}, // The inverse attributes of the icons glows.
            // The width of the icons
            volumeWidth: 25,
            settingWidth: 21,
            repeatWidth: 27,
            randomWidth: 27,
            fullScreenWidth: 25,
            // The attributes of the buttons backgrounds
            defaultBackgroundAttr: {fill: "90-#2b2b2b-#404040", stroke: "none"},
            volumeBackgroundAttr: {fill: "90-#1b8828-#24b335", stroke: "none"},
            settingsBackgroundAttr: {fill: "90-#0075c0-#009bfe", stroke: "none"},
            repeatBackgroundAttr: {fill: "90-#d9d9d9-#ffffff", stroke: "none"},
            randomBackgroundAttr: {fill: "90-#d9d9d9-#ffffff", stroke: "none"},
            fullScreenBackgroundAttr: {fill: "90-#b62e14-#f13d1a", stroke: "none"},
            linkAttr: {fill: "white", opacity: 0}, // This transparent background is used to detect when the mouse enters or clicks on a button.
            initialPaperWidth: 300, // The initial width of the SVG paper
            correctGap: false // If the width of the background have to be incremented by one to close a gap between adjacent backgrounds.
        },
        FileName:
        {
            slopeRatio: 75 / 130, // The ratio of the slope of the background.
            attr: {fill: "90-#2b2b2b-#404040", stroke: "none"}, // The attributes of the background.
            initialWidth: 300, // The initial width of the background.
            padding: 20, // The padding between the text and the background edges.
            minWidth: 50, // The file name is hidden bellow this width.
            shortenEndString: "...", // The string to put at the end of the shortened file name.
            separatorString: " - " // The string that separates the primary and the secondary file name, if displayed.
        },
        TimeLine:
        {
            height: 5, // The default height of the time line.
            expandHeight: 5, // The additionnal height of the time line when it is expanded.
            // The attributes of the time line elements
            before:
            {
                normal: {fill: "#4a4a4a", opacity: 0.5, stroke: "none"},
                opaque: {opacity: 1},
                audio: {fill: "#377f3f"},
                video: {fill: "#9e4332"}
            },
            played:
            {
                normal: {opacity: 1, stroke: "none"},
                opaque: {opacity: 1},
                audio: {fill: "#24b335"},
                video: {fill: "#f13d1a"}
            },
            buffered:
            {
                normal: {fill: "#cccccc", opacity: 0.8, stroke: "none"},
                opaque: {opacity: 1}
            },
            after:
            {
                normal: {fill: "black", opacity: 0.33, stroke: "none"},
                opaque: {fill: "#4a4a4a", opacity: 1}
            }
        }

    },
    Header:
    {
        defaultHeight: 40, // The default height of the header.
        Menu:
        {
            slopeRatio: -75 / 130, // The ratio of the slope of the backgrounds.
            paperWidth: 300, // The default width of the SVG paper.
            defaultButtonWidth: 70, // The default width of the buttons.
            margin: 30, // The margin between the buttons.
            defaultBackgroundAttr: {fill: "90-#2b2b2b-#404040", stroke: "none"},
            filesBackgroundAttr: {fill: "90-#0075c0-#009bfe", stroke: "none"},
            uploadsBackgroundAttr: {fill: "90-#1b8828-#24b335", stroke: "none"},
            linkAttr: {fill: "white", opacity: 0}, // This transparent background is used to detect when the mouse enters or clicks on a button.
            updateTextsWidthDuration: 500, // The maximum duration during which we try to get the width of the number and time texts.
            updateTextsWidthSteps: 10, // The number of attempts to get the width of the texts during the duration.
            correctGap: false // If the width of the background have to be incremented by one to close a gap between adjacent backgrounds.
        },
        Controls:
        {
            slopeRatio: 75 / 130, // The ratio of the slope of the backgrounds.
            paperWidth: 300, // The default width of the SVG paper.
            margin: 30, // The margin between the buttons.
            top: 10, // The top position of the icons.
            iconAttr: {fill: "#eeeeee", stroke: "none"}, // The attributes of the icons.
            iconGlow: {width: 5, opacity: 0.15}, // The attributes of the icons glows.
            iconGlowAttr: {stroke: "black"}, // The attributes of the icons glows.
            iconAttrInverse: {fill: "#333333", stroke: "none"}, // The inverse attributes of the icons.
            iconGlowInverse: {stroke: "white"}, // The inverse attributes of the icons glows.
            // The width of the icons
            searchWidth: 20,
            settingsWidth: 19,
            helpWidth: 11,
            disconnectWidth: 19,
            // The attributes of the buttons backgrounds
            defaultBackgroundAttr: {fill: "90-#2b2b2b-#404040", stroke: "none"},
            searchBackgroundAttr: {fill: "90-#d9d9d9-#ffffff", stroke: "none"},
            settingsBackgroundAttr: {fill: "90-#0075c0-#009bfe", stroke: "none"},
            helpBackgroundAttr: {fill: "90-#1b8828-#24b335", stroke: "none"},
            disconnectBackgroundAttr: {fill: "90-#b62e14-#f13d1a", stroke: "none"},
            linkAttr: {fill: "white", opacity: 0}, // This transparent background is used to detect when the mouse enters or clicks on a button.
            correctGap: false // If the width of the background have to be incremented by one to close a gap between adjacent backgrounds.
        }
    },
    Uploads:
    {
        requestProgressInterval: 1000 // The number of milliseconds between each progress request.
    },
	Files:
	{
        controlsHeight: 40,
        headerHeight: 28,
        headerSeparatorWidth: 1, // The width of the header separators.
        headerSeparatorMinColumnWidth: 15, // When the width of a column is lower than this, the active area of the separators next to it is shorten.
        headerMinWidth: 40, // The default minimum width of the columns of the header.
        headerTextPadding: 5, // The padding between the separator and the header columns.
        headerDefaultWidth: 200, // The default width of the coulmns of the header.
        listRowHeight: 30, // The height of the rows of the files list.
        columnTypeTextMinWidth: 30 // The minimum width required by the type column to display the type in text instead of the colors.
	},
	View:
	{
        minHeight: 15 // The minimal height of the images and videos.
	}
};
var C = Configuration;
