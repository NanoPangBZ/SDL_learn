#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define BACKGROUND_COLOR    0, 0, 0

/**
 * @brief 填充正弦波音频帧缓冲
 * @param audio_frame_buffer 音频帧缓冲
 * @param sample_rate 采样率
 * @param sine_freq_hz 正弦波频率
 * @param position 正弦波位置
 * @param frame_point_count 帧点数
 * @return 最后一点的相位
*/
static float fill_sine_audio_frame_buffer( float *audio_frame_buffer , int sample_rate , int sine_freq_hz , float position , uint32_t frame_point_count )
{
    float *p = audio_frame_buffer;
    for ( uint32_t i = 0; i < frame_point_count; i++ )
    {
        *p = SDL_sinf( position * 2.0f * SDL_PI_F );
        p++;
        position += (float)sine_freq_hz / (float)sample_rate;
    }

    return position;
}

/**
 * @brief 交错左右声道音频帧缓冲
 * @param l_frame_buffer 左声道音频帧缓冲
 * @param r_frame_buffer 右声道音频帧缓冲
 * @param audio_frame_buffer 交错后的音频帧缓冲
 * @param frame_point_count 帧点数
*/
static void lr_frame_interlaced( float *l_frame_buffer , float *r_frame_buffer , float *audio_frame_buffer , uint32_t frame_point_count )
{
    float *p = audio_frame_buffer;
    for ( uint32_t i = 0; i < frame_point_count; i++ )
    {
        *p = l_frame_buffer[i];
        p++;
        *p = r_frame_buffer[i];
        p++;
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_AudioStream *stream = NULL;
    SDL_AudioSpec spec;
    bool running = true;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("SDL3 100Hz Sine", 800, 500, 0,
                                     &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* 创建音频流 , 32位浮点, 1通道, 44.1kHz采样率 */
    spec.channels = 1;
    spec.format = SDL_AUDIO_F32;
    spec.freq = 44100;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                       &spec, NULL, NULL);
    if (!stream) {
        SDL_Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_ResumeAudioStreamDevice(stream);

    /* 音频帧缓冲 */
    float l_audio_frame_buffer[ 44100 ];
    float r_audio_frame_buffer[ 44100 ];
    float audio_frame_buffer[ 44100 * 2 ];
    float sine_position_l = 0;
    float sine_position_r = 0;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN &&
                 event.key.key == SDLK_ESCAPE)) {
                running = false;
            }
        }

        /* 当前SDL音频流中剩余的音频小于一帧的一半，则提交新的音频帧缓冲 */
        if (SDL_GetAudioStreamQueued(stream) < sizeof(audio_frame_buffer) / sizeof(float) / 2)
        {
            sine_position_l = fill_sine_audio_frame_buffer( l_audio_frame_buffer , spec.freq , 400 , sine_position_l , sizeof(l_audio_frame_buffer) / sizeof(float) );
            sine_position_r = fill_sine_audio_frame_buffer( r_audio_frame_buffer , spec.freq , 1000 , sine_position_r , sizeof(r_audio_frame_buffer) / sizeof(float) );
            lr_frame_interlaced( l_audio_frame_buffer , r_audio_frame_buffer , audio_frame_buffer , sizeof(audio_frame_buffer) / sizeof(float) );
            SDL_PutAudioStreamData(stream, audio_frame_buffer, (int)sizeof(audio_frame_buffer));
        }

        SDL_DelayNS( 100 );
    }

    SDL_DestroyAudioStream(stream);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
