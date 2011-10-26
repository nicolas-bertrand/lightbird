// The configuration of the Client.
var Configuration =
{
	// The time between each new session id.
	identificationTimerTimeout : 50000,
	// The default size of the new windows
	newWindowWidth : 600,
	newWindowHeight : 300,
	// Default animation FPS
	animationFPS : 20,
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