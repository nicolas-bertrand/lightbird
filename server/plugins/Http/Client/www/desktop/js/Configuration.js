// The configuration of the Client.
var Configuration =
{
	// The default size of the new windows.
	newWindowWidth : 600,
	newWindowHeight : 300,
	// Default animation FPS.
	animationFPS : 20,
    // The configuration of the desktop.
    Desktop :
    {
        minWidth : 600, // The minimum width of the desktop.
        minHeight : 400, // The minimum height of the desktop.
        playerDocumentHeight : 50, // The height of the document player.
        playerMediaHeight : 100, // The height of the media player.
        tasksListWidth : 50, // The width of the tasks list.
        resizeTasksListWidth : 1, // The width of the div that allows to resize the tasks list.
        taskHeightRatio : 5 / 6, // The height of the task icon in the tasks list is : task width * taskHeightRatio
        tasksListPadding : 7, // The padding of the page icons in the tasks list.
        taskMagneticDragX : 0, // Limit between the tasks list drag and the free drag (from the tasks list right border).
        taskMagneticDragY : 3, // Number of pixels a task have to be dragged vertically before actually moving.
        tasksListScrollHeight : 50, // Height of the scrolling areas of the tasks list.
        scrollSpeed : 12, // The speed of the scroll areas of the tasks list.
        mouseWheelScrollSpeed : 13, // The number of pixels scrolled by each mouse wheel click.
        moveTaskWheelSpeed : 10, // The speed at which the tasks are changed with the mouse wheel when a task is dragged ouside the tasks list.
        newTaskHeight : 20, // Height of the new task area.
        pagePreviewDelay : 500, // The delay to display a page when the mouse enters the tasks list.
        taskMargin : 20, // The margin between the tasks of a page. It is also the size of the resize task bars.
        freeDragTashShift : { x : 10, y : 10 }, // The shift of the dragged task when we are in free drag.
        defaultPosition : "e", // The default position of a task moved (n s e w).
        insertTaskAreaSize : 50, // The size of the page border areas that allows to insert a task.
        resizeTaskLimitMin : 0, // The minimum ratio by which a task can be resized. Between 0 and 1.
        resizeTaskLimitMax : 1, // The maximum ratio by which a task can be resized. Between 0 and 1.
        resizeResistance : 50, // The number of pixels the mouse need to move before the resize of the tasks starts.
        stabilizeTaskResize : true, // If true, the ratio of the tasks being resized is stabilized.
        resizeTasksListMin : 30, // The minimum size of the tasks list.
        resizeTasksListMax : 200, // The maximum size of the tasks list.
        taskIconMinWidth : 15, // The minimum width if a task icon.
    },
	// The size of the window
	Window :
	{
		// The minimum number of pixels of a window that can be out of the screen
		minOutX : -18, // $1
		minOutY : -13, // $2
		// The minimum size of a window
		minWidth : 300,
		minHeight : 0,
		// The size of the shadow
		shadowLeft : 18, // $1
		shadowRight : 18,
		shadowTop : 13, // $2
		shadowBottom : 23,
		// The size of the borders
		borderLeft : 29,
		borderRight : 29,
		borderTop : 49,
		borderBottom : 34,
	},
	// The explorer settings
	Explorer :
	{
		// Some elements size
		buttons : 27,
		headers : 30,
		types : 29,
	},
	Information :
	{
		// The height of the button bar
		buttonsHeight : 62,
		// The maximum height of the preview image
		previewMaxHeight : 600,
	},
	Player :
	{
		playlistWidth : 200,
		playlistText : 197,
		playlistNoText : 150,
		dragWidth : 5,
		controlsHeight : 31,
		// The width of the controls without the seek bar
		controlsWidth : 349,
		// The width of the left side of the seek bar (with the position in second)
		controlsWidthLeft : 141,
		// The width of the borders of the seek bar
		controlSeekBordersWidth : 5,
		playerMinWidth : 450,
		// A preview displayed on the seek bar
		previewWidth : 200,
		previewHeight : 40,
		// The number of previews generated for each video
		previews : 15,
		// The offset of the quality button
		qualityRightOffset : 62,
		qualityBottomOffset : 37,
	}
};
var C = Configuration;
var DOMWindow = window;
