#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define BACKGROUND_COLOR    0, 0, 0
#define WAVE_COLOR          255, 255, 255

/**
 * @brief 填充正弦波音频帧缓冲
 * @param audio_frame_buffer 音频帧缓冲
 * @param sample_rate 采样率
 * @param sine_freq_hz 正弦波频率
 * @param position 当前相位（弧度）
 * @param frame_point_count 帧点数
 * @return 最后一点之后的相位（弧度，已限制在 [0, 2π)）
*/
static float fill_sine_audio_frame_buffer(float *audio_frame_buffer, int sample_rate,
                                    int sine_freq_hz, float position,
                                    uint32_t frame_point_count)
{
    const float phase_delta = 2.0f * SDL_PI_F * (float)sine_freq_hz / (float)sample_rate;
    float *p = audio_frame_buffer;

    for (uint32_t i = 0; i < frame_point_count; i++) {
        *p++ = SDL_sinf(position);
        position += phase_delta;
        if (position >= 2.0f * SDL_PI_F) {
                position -= 2.0f * SDL_PI_F;
            }
    }

    return position;
}

/**
 * @brief 绘制波形
 * @param renderer 渲染器
 * @param wave 波形（取值约 [-1, 1]）
 * @param wave_count 波形点数
 * @param wave_width 波形宽度
 * @param wave_height 波形高度
 * @param wave_x 波形区域左上角 x
 * @param wave_y 波形区域左上角 y
*/
static void draw_wave(SDL_Renderer *renderer, float *wave, uint32_t wave_count,
                      uint32_t wave_width, uint32_t wave_height,
                      float wave_x, float wave_y)
{
    float mid_y;
    float half_h;
    uint32_t i;

    if (wave == NULL || wave_count < 2 || wave_width == 0 || wave_height == 0) {
        return;
    }

    mid_y = wave_y + (float)wave_height * 0.5f;
    half_h = (float)wave_height * 0.5f;

    for (i = 0; i < wave_count - 1; i++) {
        float x1 = wave_x + (float)i * (float)wave_width / (float)(wave_count - 1);
        float x2 = wave_x + (float)(i + 1) * (float)wave_width / (float)(wave_count - 1);
        /* y 轴向下，所以用 mid - value * half，使 +1 在上方 */
        float y1 = mid_y - wave[i] * half_h;
        float y2 = mid_y - wave[i + 1] * half_h;
        SDL_RenderLine(renderer, x1, y1, x2, y2);
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
    float audio_frame_buffer[ 44100 / 10 ];    // 每帧100ms
    float sine_position = 0;

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
        if (SDL_GetAudioStreamQueued(stream) < (spec.freq * (int)sizeof(float)) / 2)
        {
            sine_position = fill_sine_audio_frame_buffer( audio_frame_buffer , spec.freq , 100 , sine_position , sizeof(audio_frame_buffer) / sizeof(float) );
            SDL_PutAudioStreamData(stream, audio_frame_buffer, (int)sizeof(audio_frame_buffer));

            SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR , 255 );
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, WAVE_COLOR , 255 );
            draw_wave(renderer, audio_frame_buffer, sizeof(audio_frame_buffer) / sizeof(float), 800, 500, 0, 0);

            SDL_RenderPresent(renderer);
        }

        SDL_Delay( 1 );
    }

    SDL_DestroyAudioStream(stream);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
