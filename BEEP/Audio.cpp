#include "../TonePlayer/TonePlayer.h"
#include "../Music/MusicCode.h"
#include "Audio.h"
#include "string.h"
#include "Buzz.h"

static TonePlayer player;

void Audio_Init()
{
    player.SetCallback([](uint32_t freq, uint16_t volume)
    {
        Buzz_Tone(freq);//duration是TONE_DURATION_INFINITE,Tone播放时间在TonePlayer::Update中设定
    });
}
//定时更新
void Audio_Update()
{
    player.Update(millis());
}

bool Audio_PlayMusic(const char* name)
{
    bool retval = false;
    for (int i = 0; i < sizeof(MusicList) / sizeof(MusicList[0]); i++)
    {
        if (strcmp(name, MusicList[i].name) == 0)
        {
            player.Play(MusicList[i].mc, MusicList[i].length);
            retval = true;
            break;
        }
    }
    return retval;
}
