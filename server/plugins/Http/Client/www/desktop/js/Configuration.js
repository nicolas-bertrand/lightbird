// The configuration of the Client.
var Configuration =
{
	animationFPS : 20, // Default animation FPS.
    // The configuration of the desktop.
    Desktop :
    {
        minWidth : 600, // The minimum width of the desktop.
        minHeight : 300, // The minimum height of the desktop.
        topHeight : 40, // The height of the top menu.
        bottomHeight : 45, // The height of the bottom area.
        collectionsWidth : 10, // The width of the collections list.
        pageMargin : 10, // The height of the bottom margin between the pages.
        tasksListWidth : 120, // The width of the tasks list.
        tasksListMargin : 10, // The width of the left margin of the tasks list.
        taskIconHeight : 75, // The height of a task icon.
        taskResistance : 3, // Number of pixels a task icon have to be dragged before actually moving.
        taskDragShift : { x : 10, y : 10 }, // The shift of the dragged task when when it is out of the tasks list.
        tasksListScrollHeight : 50, // Height of the scrolling areas of the tasks list.
        tasksListScrollSpeed : 20, // The speed of the scroll areas of the tasks list.
        mouseWheelMultiplier : 50, // Used to normalize the mouse wheel delta. Increases the delta of each click.
        defaultPosition : "e", // The default position of a task moved (n s e w).
        insertTaskAreaSize : 50, // The size of the page border areas that allows to insert a task.
        taskMargin : 5, // The margin between the tasks content in a page. It is also the size of the resize task bars.
        taskBorder : 1, // The size of the border of the tasks content.
        resizeTaskLimitMin : 0, // The minimum ratio by which a task can be resized. Between 0 and 1.
        resizeTaskLimitMax : 1, // The maximum ratio by which a task can be resized. Between 0 and 1.
        resizeResistance : 50, // The number of pixels the mouse need to move before the resize of the tasks starts.
        resizeDeltaDivisor : 20, // Reduce the delta of the mouse wheel that resize the task margin.
    },
	// The size of the window
	Window :
	{
        default : // The default position of the window
        {
            left : 200,
            top : 100,
            width : 600,
            height : 400
        },
        limit : // The limits of the window when it is moved or resized
        {
            top : 8,
            bottom : 28,
        },
        minWidth : 162,
        minHeight : 35,
        border : 5, // The size of the border of the window.
        topHeight : 35, // The height of the top of the window.
        bottomHeight : 30, // The height of the bottom of the window.
	},
    Uploads :
    {
        requestProgressInterval : 1000, // The number of milliseconds between each progress request.
    },
	Files :
	{
        controlsHeight : 40,
        headerHeight : 28,
        headerSeparatorWidth : 1, // The width of the header separators.
        headerMinWidth : 40, // The default minimum width of the columns of the header.
        headerTextPadding : 5, // The padding between the separator and the header columns.
        headerDefaultWidth : 200, // The default width of the coulmns of the header.
	},
};
var C = Configuration;
var DOMWindow = window;
