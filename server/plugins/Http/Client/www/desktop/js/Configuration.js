// The configuration of the Client.
var Configuration =
{
    Desktop:
    {
        minWidth: 650, // The minimum width of the desktop.
        minHeight: 300, // The minimum height of the desktop.
        browserFullScreen: false, // If we have to use the browser full screen API.
        stopDragLeaveDesktop: false, // If the current drag have to be stopped when the mouse leaves the browser.
        mouseWheelMultiplier: 50, // Used to normalize the mouse wheel delta. Increases the delta of each click.
        insertTaskAreaSize: 50, // The size of the page border areas that allows to insert a task.
        taskMargin: 5, // The margin between the tasks content in a page. It is also the size of the resize task bars.
        taskBorder: 1, // The size of the border of the tasks content.
        resizeTaskLimitMin: 0, // The minimum ratio by which a task can be resized. Between 0 and 1.
        resizeTaskLimitMax: 1, // The maximum ratio by which a task can be resized. Between 0 and 1.
        resizeResistance: 50, // The number of pixels the mouse need to move before the resize of the tasks starts.
        resizeDeltaDivisor: 20, // Reduce the delta of the mouse wheel that resize the task margin.
        doubleClickInterval: 250 // The time interval between two clicks required to create a double click.
    },
    TasksList:
    {
        defaultWidth: 120, // The default width of the tasks list.
        top: 15, // The height of the top margin of the tasks list.
        pageMargin: 15, // The height of the bottom margin between the pages icons.
        taskTitleHeight: 30, // The height of the tasks icon title.
        taskResistance: 3, // Number of pixels a task icon have to be dragged before actually moving.
        taskDragShift: { x: 10, y: 10 }, // The shift of the dragged task when when it is out of the tasks list.
        defaultPosition: "e", // The default position of a task moved (n s e w).
        Scroll:
        {
            height: 50, // Height of the scrolling areas.
            speed: 20, // The speed of the scroll areas.
            fps: 1000 / 30, // The number of time per second the scroll is performed.
        },
        FullScreen:
        {
            hideWidth: 1, // The width of the tasksList while it is hidden, which allows to display it back when the mouse enters it again.
            displayDuration: 2000, // The duration during which the player is kept displayed when the mouse leaves it.
            zIndex: 999999 // In full screen, the tasks list is above the windows.
        },
        Buttons:
        {
            slopeRatio: 75 / 130, // The ratio of the slope of the background.
            margin: 5, // The margin of the icons.
            linkAttr: {fill: "white", opacity: 0}, // The background of the buttons active areas.
            close:
            {
                width: 10, // The width of the close path.
                height: 9, // The height of the close path.
                attr: {fill: "white", opacity: 0.5, stroke: "none"}, // The attributes of the close path.
                attrOver: {opacity: 1}, // The attributes of the close path when the mouse is over it.
                // The attributes of the background.
                backgroundDesktop: {fill: "180-#4895d0:50-#4895d0", "fill-opacity": 0.5, stroke: "none"}, // When the icon is on the desktop
                backgroundWindow: {fill: "#53b85c", stroke: "none"}, // When the icon is a window
                backgroundOver: {fill: "#e34738", stroke: "none"}, // When the mouse is over the button
            },
            window:
            {
                width: 10, // The width of the window path.
                height: 9, // The height of the window path.
                attr: {fill: "white", opacity: 0.5, stroke: "none"}, // The attributes of the window path.
                attrOver: {opacity: 1}, // The attributes of the window path when the mouse is over it.
                background: {fill: "#575757", opacity: 0.5, stroke: "none"}, // The attributes of the background.
                backgroundDesktop: {fill: "#53b85c", opacity: 1}, // The color of the background when the page is on the desktop.
                backgroundWindow: {fill: "#4895d0", opacity: 1}, // The color of the background when the page is in a window.
            },
        }
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
        bottomHeight: 30, // The height of the bottom of the window.
        maximizeMargin: 40 // The margin between the desktop sides and the maximized window.
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
        FullScreen:
        {
            hideHeight: 1, // The height of the player while it is hidden, which allows to display it back when the mouse enters it again.
            displayDuration: 2000 // The duration during which the player is kept displayed when the mouse leaves it.
        },
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
        FullScreen:
        {
            hideHeight: 1, // The height of the header while it is hidden, which allows to display it back when the mouse enters it again.
            displayDuration: 2000 // The duration during which the header is kept displayed when the mouse leaves it.
        },
        Menu:
        {
            slopeRatio: -75 / 130, // The ratio of the slope of the backgrounds.
            paperWidth: 300, // The default width of the SVG paper.
            defaultButtonWidth: 70, // The default width of the buttons.
            margin: 30, // The margin between the buttons.
            defaultBackgroundAttr: {fill: "90-#2b2b2b-#404040", stroke: "none"},
            filesBackgroundAttr: {fill: "90-#0075c0-#009bfe", stroke: "none"},
            uploadBackgroundAttr: {fill: "90-#b62e14-#f13d1a", stroke: "none"},
            linkAttr: {fill: "white", opacity: 0}, // This transparent background is used to detect when the mouse enters or clicks on a button.
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
    Context:
    {
        cyclicScroll: true, // Allows to scroll the actions with an infinite loop.
    },
    Uploads:
    {
        onUploadInterval: 1000 / 10, // The number of milliseconds between the onUpload event calls.
        concurrent: 4, // The number of files that can be sent at the same time. 
    },
    Files:
    {
        updateInterval: 2 * 1000, // The number of milliseconds between the file update request to the server.
        cleanFilesDeleted: 30 * 1000, // The number of milliseconds after which the list of the files deleted is cleaned.
    },
    User:
    {
        passwordCharacter: "\u25CF", // The character used to replace the password.
        activeElementLoop: true, // If true the tab key will loop the focus between the inputs of the authentication form.
    },
    R: // Resources
    {
        Files:
        {
            controlsHeight: 40,
            listRowHeight: 30, // The height of the rows of the files list.
            Columns:
            {
                height: 30, // The default height of the columns.
                slopeRatio: 75 / 130, // The ratio of the slope of the columns.
                backgroundAttr: {fill: "90-#9bd0f9-#afd9fa", stroke: "none"}, // The attributes of the background of the columns.
                oddAttr: {fill: "90-#48aaf0-#6cbbf3", stroke: "none"}, // The attributes of the odd columns.
                evenAttr: {fill: "90-#2f9eed-#6cbbf3", stroke: "none"}, // The attributes of the even columns.
                separatorWidth: 15, // The width of the separators.
                separatorAttr: {fill: "90-#5ab7fb-#83c9fc", stroke: "none"}, // The attributes of the separators.
                columnMargin: 10, // The margin between the column text and the separator.
                shortenString: ".", // A string that tells that a text has been shortened.
                defaultWidth: 200, // The default width of the coulmns of the columns.
                defaultMinWidth: 40, // The default minimum width of the columns of the columns.
                typeTextMinWidth: 30, // The minimum width required by the type column to display the type in text instead of the colors.
                moveResistance: 3, // The number of pixels the mouse have to move before to start to move the column.
                moveAttr: {fill: "90-#f0a44a-#f4bb77", stroke: "none"}, // The attribute of the column moved.
                moveBackgroundAttr: {fill: "#81aac7"}, // The color of the space that the moved column will take.
                sortAttr: {fill: "90-#4a94f0-#77aff4"}, // The color of the sorted column.
                sortSeparatorAttr: {fill: "90-#65a5f5-#8bbcf8"}, // The color of the separator adjacent to the sorted column.
                adjustContext: true, // True if the context menu to adjust the columns has to be displayed.
                actionHideContext: false, // True if an action on the context menu should hide it.
            },
        },
        View:
        {
            minHeight: 15, // The minimal height of the images and videos.
            focusBeforeAction: true // If the page have to be focused before doing any action when the mouse clicks on a task.
        },
    },
};
var C = Configuration;
