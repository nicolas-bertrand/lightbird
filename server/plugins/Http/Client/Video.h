#ifndef VIDEO_H
# define VIDEO_H

# include "IVideo.h"

# include "Media.h"

class Video : public Media
{
public:
    Video(LightBird::IClient &client);
    ~Video();
    /// @brief Read the data on the transcoding stream and put it in the client's response.
    void                read();

private:
    LightBird::IVideo   *video; ///< Transcode the video in another format while it is streamed.
};

#endif // VIDEO_H
