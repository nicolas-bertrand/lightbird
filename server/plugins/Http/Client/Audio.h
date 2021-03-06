#ifndef AUDIO_H
# define AUDIO_H

# include "IAudio.h"

# include "Media.h"

class Audio : public Media
{
public:
    Audio(LightBird::IClient &client);
    ~Audio();

    /// @brief Reads the data on the transcoding stream and put it in the client's response.
    void    read();

private:
    LightBird::IAudio *audio; ///< Transcodes the audio in another format while it is streamed.
};

#endif // AUDIO_H
