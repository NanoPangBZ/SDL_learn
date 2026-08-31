#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

extern const unsigned char __embedded_font[];
extern const size_t __embedded_font_size;

#define BACKGROUND_COLOR    0, 0, 0
#define WAVE_COLOR          255, 255, 255
#define SPECTRUM_MAX_FREQUENCY 1000
#define SPECTRUM_FREQUENCY_STEP 0.5f
#define SPECTRUM_POINT_COUNT 2001
#define SPECTRUM_LABEL_STEP 50
#define CURVE_SEGMENTS_PER_POINT 8

/**
 * @brief 填充正弦波音频帧缓冲
 * @param audio_frame_buffer 音频帧缓冲
 * @param sample_rate 采样率
 * @param sine_freq_hz 正弦波频率
 * @param position 当前相位（弧度）
 * @param frame_point_count 帧点数
 * @param scale 幅度缩放
 * @return 最后一点之后的相位（弧度，已限制在 [0, 2π)）
*/
static float fill_sine_audio_frame_buffer(float *audio_frame_buffer, int sample_rate,
                                          int sine_freq_hz, float position,
                                          uint32_t frame_point_count,float scale)
{
    const float phase_delta = 2.0f * SDL_PI_F * (float)sine_freq_hz / (float)sample_rate;
    float *p = audio_frame_buffer;

    for (uint32_t i = 0; i < frame_point_count; i++) {
        *p++ = SDL_sinf(position) * scale;
        position += phase_delta;
        if (position >= 2.0f * SDL_PI_F) {
            position -= 2.0f * SDL_PI_F;
        }
    }

    return position;
}

/**
 * @brief 填充方波音频帧缓冲
 */
static float fill_square_audio_frame_buffer(float *audio_frame_buffer, int sample_rate,
                                            int square_freq_hz, float position,
                                            uint32_t frame_point_count, float scale)
{
    const float phase_delta = 2.0f * SDL_PI_F * (float)square_freq_hz /
                              (float)sample_rate;

    for (uint32_t i = 0; i < frame_point_count; ++i) {
        audio_frame_buffer[i] = position < SDL_PI_F ? scale : -scale;
        position += phase_delta;
        if (position >= 2.0f * SDL_PI_F) {
            position -= 2.0f * SDL_PI_F;
        }
    }

    return position;
}

/**
 * @brief 音频混合
 * @param frame_buffer_1 音频帧缓冲1
 * @param frame_buffer_2 音频帧缓冲2
 * @param frame_buffer_out 输出音频帧缓冲
 * @param frame_point_count 帧点数
*/
static void audio_mix( float* frame_buffer_1 ,float* frame_buffer_2 , float* frame_buffer_out , uint32_t frame_point_count )
{
    float *p_1 = frame_buffer_1;
    float *p_2 = frame_buffer_2;
    float *p_out = frame_buffer_out;

    for (uint32_t i = 0; i < frame_point_count; i++) {
        p_out[i] = p_1[i] + p_2[i];
        if( p_out[i] > 1.0f )
        {
            p_out[i] = 1.0f;
        }
        else if( p_out[i] < -1.0f )
        {
            p_out[i] = -1.0f;
        }
    }
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
 * @param min_color 最低点颜色
 * @param max_color 最高点颜色
*/
static void draw_wave(SDL_Renderer *renderer, float *wave, uint32_t wave_count,
    uint32_t wave_width, uint32_t wave_height,
    float wave_x, float wave_y, SDL_Color min_color, SDL_Color max_color,
    TTF_Font *font)
{
    const float padding_left = 50.0f;
    const float padding_right = 45.0f;
    const float padding_top = 20.0f;
    const float padding_bottom = 50.0f;
    float plot_x;
    float plot_y;
    float plot_width;
    float plot_height;
    float mid_y;
    float half_h;
    SDL_FPoint *points;
    uint32_t curve_point_count;
    uint32_t curve_index = 0;
    uint32_t i;

    if (wave == NULL || wave_count < 2 ||
        (float)wave_width <= padding_left + padding_right ||
        (float)wave_height <= padding_top + padding_bottom) {
        return;
    }

    curve_point_count = (wave_count - 1) * CURVE_SEGMENTS_PER_POINT + 1;
    points = (SDL_FPoint *)SDL_malloc(sizeof(*points) * curve_point_count);
    if (points == NULL) {
        return;
    }

    plot_x = wave_x + padding_left;
    plot_y = wave_y + padding_top;
    plot_width = (float)wave_width - padding_left - padding_right;
    plot_height = (float)wave_height - padding_top - padding_bottom;
    mid_y = plot_y + plot_height * 0.5f;
    half_h = plot_height * 0.5f;

    /* X 轴、每 50 Hz 一个刻度及其频率标签。 */
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderLine(renderer, plot_x, plot_y + plot_height,
                   plot_x + plot_width, plot_y + plot_height);
    for (i = 0; i <= SPECTRUM_MAX_FREQUENCY; i += SPECTRUM_LABEL_STEP) {
        const float tick_x = plot_x + plot_width * (float)i /
                                      (float)SPECTRUM_MAX_FREQUENCY;
        SDL_RenderLine(renderer, tick_x, plot_y + plot_height,
                       tick_x, plot_y + plot_height + 7.0f);

        if (font != NULL) {
            char label[16];
            SDL_Surface *surface;
            SDL_Texture *texture;
            SDL_FRect destination;

            SDL_snprintf(label, sizeof(label), "%u", i);
            surface = TTF_RenderText_Blended(font, label, 0,
                                              (SDL_Color){ 180, 180, 180, 255 });
            if (surface != NULL) {
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture != NULL) {
                    destination.x = tick_x - (float)surface->w * 0.5f;
                    destination.y = plot_y + plot_height + 10.0f;
                    destination.w = (float)surface->w;
                    destination.h = (float)surface->h;
                    SDL_RenderTexture(renderer, texture, NULL, &destination);
                    SDL_DestroyTexture(texture);
                }
                SDL_DestroySurface(surface);
            }
        }
    }

    for (i = 0; i < wave_count - 1; ++i) {
        const float p0 = wave[i == 0 ? 0 : i - 1];
        const float p1 = wave[i];
        const float p2 = wave[i + 1];
        const float p3 = wave[i + 2 < wave_count ? i + 2 : wave_count - 1];
        uint32_t segment;

        for (segment = 0; segment < CURVE_SEGMENTS_PER_POINT; ++segment) {
            const float t = (float)segment / (float)CURVE_SEGMENTS_PER_POINT;
            const float t2 = t * t;
            const float t3 = t2 * t;
            float value = 0.5f * ((2.0f * p1) +
                         (-p0 + p2) * t +
                         (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                         (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);

            value = SDL_clamp(value, -1.0f, 1.0f);
            points[curve_index].x = plot_x +
                ((float)i + t) * plot_width / (float)(wave_count - 1);
            /* y 轴向下，所以用 mid - value * half，使 +1 在上方 */
            points[curve_index].y = mid_y - value * half_h;
            ++curve_index;
        }
    }

    points[curve_index].x = plot_x + plot_width;
    points[curve_index].y = mid_y - SDL_clamp(wave[wave_count - 1], -1.0f, 1.0f) * half_h;
    ++curve_index;

    for (i = 0; i + 1 < curve_index; ++i) {
        const float average_y = (points[i].y + points[i + 1].y) * 0.5f;
        const float value = SDL_clamp((mid_y - average_y) / half_h,
                                      -1.0f, 1.0f);
        const float ratio = (value + 1.0f) * 0.5f;
        const Uint8 red = (Uint8)((float)min_color.r +
                          ((float)max_color.r - (float)min_color.r) * ratio);
        const Uint8 green = (Uint8)((float)min_color.g +
                            ((float)max_color.g - (float)min_color.g) * ratio);
        const Uint8 blue = (Uint8)((float)min_color.b +
                           ((float)max_color.b - (float)min_color.b) * ratio);
        const Uint8 alpha = (Uint8)((float)min_color.a +
                            ((float)max_color.a - (float)min_color.a) * ratio);

        SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
        SDL_RenderLine(renderer, points[i].x, points[i].y,
                       points[i + 1].x, points[i + 1].y);
    }
    SDL_free(points);
}

/**
 * @brief 频率点结构体
*/
typedef struct fre_point_t{
    float frequency;
    float amplitude;
}fre_point_t;

/**
 * @brief FFT
 * @param wave_buffer 波形缓冲
 * @param point_count 波形点数
 * @param sample_rate 采样率
 * @param fre_point_buffer 频率点缓冲
 * @param fre_point_count 频率点数
*/
static void wave_fft( float* wave_buffer, uint32_t point_count , int sample_rate , fre_point_t* fre_point_buffer , uint32_t fre_point_count )
{
    uint32_t point_index;

    if (fre_point_buffer == NULL) {
        return;
    }

    /*
     * frequency 由调用方指定，因此它不一定落在普通 FFT 的整数频率桶上。
     * 这里使用 Goertzel 算法计算每个指定频率处的 DFT，避免计算无用频点。
     */
    for (point_index = 0; point_index < fre_point_count; ++point_index) {
        const float frequency = fre_point_buffer[point_index].frequency;
        float previous = 0.0f;
        float previous_previous = 0.0f;
        float coefficient;
        float omega;
        float real;
        float imaginary;
        float magnitude;
        float scale;
        uint32_t sample_index;

        fre_point_buffer[point_index].amplitude = 0.0f;

        if (wave_buffer == NULL || point_count == 0 || sample_rate <= 0 ||
            frequency < 0.0f || frequency > (float)sample_rate * 0.5f) {
            continue;
        }

        omega = 2.0f * SDL_PI_F * frequency / (float)sample_rate;
        coefficient = 2.0f * SDL_cosf(omega);

        for (sample_index = 0; sample_index < point_count; ++sample_index) {
            const float current = wave_buffer[sample_index] +
                                  coefficient * previous - previous_previous;
            previous_previous = previous;
            previous = current;
        }

        real = previous - previous_previous * SDL_cosf(omega);
        imaginary = previous_previous * SDL_sinf(omega);
        magnitude = SDL_sqrtf(real * real + imaginary * imaginary);

        /* 单边幅度谱：DC 和 Nyquist 不需要乘 2。 */
        scale = (frequency == 0.0f || frequency == (float)sample_rate * 0.5f)
                    ? 1.0f / (float)point_count
                    : 2.0f / (float)point_count;
        fre_point_buffer[point_index].amplitude = magnitude * scale;
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_AudioStream *stream = NULL;
    TTF_Font *font = NULL;
    SDL_AudioSpec spec;
    bool running = true;
    bool fullscreen = false;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) || !TTF_Init()) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("SDL3 100Hz Sine", 800, 500,
                                     SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFontIO(SDL_IOFromConstMem(__embedded_font,
                                             __embedded_font_size),
                          true, 14.0f);
    if (font == NULL) {
        SDL_Log("TTF_OpenFontIO failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
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
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_ResumeAudioStreamDevice(stream);

    /* 音频帧缓冲 */
    static float audio_frame_buffer[ 4410 ];

    static float ch0_audio_frame_buffer[ 4410 ];
    static float ch1_audio_frame_buffer[ 4410 ];
    static float ch2_audio_frame_buffer[ 4410 ];
    static float ch3_audio_frame_buffer[ 4410 ];
    
    float ch0_audio_sin_position = 0;
    float ch1_audio_sin_position = 1;
    float ch2_audio_sin_position = 2;
    float ch3_audio_square_position = 0;
    fre_point_t frequency_points[SPECTRUM_POINT_COUNT];
    float spectrum_wave[SPECTRUM_POINT_COUNT];

    for (uint32_t i = 0; i < SPECTRUM_POINT_COUNT; ++i) {
        frequency_points[i].frequency = (float)(i * SPECTRUM_FREQUENCY_STEP);
        frequency_points[i].amplitude = 0.0f;
        spectrum_wave[i] = -1.0f;
    }

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN &&
                 event.key.key == SDLK_ESCAPE)) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN &&
                       event.key.key == SDLK_F11 && !event.key.repeat) {
                fullscreen = !fullscreen;
                if (!SDL_SetWindowFullscreen(window, fullscreen)) {
                    SDL_Log("SDL_SetWindowFullscreen failed: %s", SDL_GetError());
                    fullscreen = !fullscreen;
                }
            }
        }

        /* 当前SDL音频流中剩余的音频小于一帧的一半，则提交新的音频帧缓冲 */
        if (SDL_GetAudioStreamQueued(stream) < (spec.freq * (int)sizeof(float)) / 2)
        {
            ch0_audio_sin_position = fill_sine_audio_frame_buffer( ch0_audio_frame_buffer , spec.freq , 100 , ch0_audio_sin_position , sizeof(ch0_audio_frame_buffer) / sizeof(float) , 0.5 );
            ch1_audio_sin_position = fill_sine_audio_frame_buffer( ch1_audio_frame_buffer , spec.freq , 400 , ch1_audio_sin_position , sizeof(ch1_audio_frame_buffer) / sizeof(float) , 0.2 );
            ch2_audio_sin_position = fill_sine_audio_frame_buffer( ch2_audio_frame_buffer , spec.freq , 110 , ch2_audio_sin_position , sizeof(ch2_audio_frame_buffer) / sizeof(float) , 0.7 );
            ch3_audio_square_position = fill_square_audio_frame_buffer( ch3_audio_frame_buffer , spec.freq , 110 , ch3_audio_square_position , sizeof(ch3_audio_frame_buffer) / sizeof(float) , 1.0 );
            audio_mix( ch0_audio_frame_buffer , ch1_audio_frame_buffer , audio_frame_buffer , sizeof(audio_frame_buffer) / sizeof(float) );
            audio_mix( audio_frame_buffer , ch2_audio_frame_buffer , audio_frame_buffer , sizeof(audio_frame_buffer) / sizeof(float) );
            audio_mix( audio_frame_buffer , ch3_audio_frame_buffer , audio_frame_buffer , sizeof(audio_frame_buffer) / sizeof(float) );
            wave_fft(audio_frame_buffer,
                     sizeof(audio_frame_buffer) / sizeof(audio_frame_buffer[0]),
                     spec.freq, frequency_points, SPECTRUM_POINT_COUNT);

            for (uint32_t i = 0; i < SPECTRUM_POINT_COUNT; ++i) {
                /* draw_wave 使用 [-1, 1]，将幅值 [0, 1] 映射为底部到顶部。 */
                float display_value = frequency_points[i].amplitude * 2.0f - 1.0f;
                spectrum_wave[i] = SDL_clamp(display_value, -1.0f, 1.0f);
            }

            SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR , 255 );
            SDL_RenderClear(renderer);
            {
                int output_width;
                int output_height;
                const SDL_Color minimum_color = { 0, 70, 45, 255 };
                const SDL_Color maximum_color = { 255, 0, 0, 255 };

                if (SDL_GetRenderOutputSize(renderer, &output_width, &output_height) &&
                    output_width > 0 && output_height > 0) {
                    draw_wave(renderer, spectrum_wave, SPECTRUM_POINT_COUNT,
                              (uint32_t)output_width, (uint32_t)output_height,
                              0.0f, 0.0f, minimum_color, maximum_color, font);
                }
            }
            SDL_RenderPresent(renderer);
            SDL_PutAudioStreamData(stream, audio_frame_buffer, (int)sizeof(audio_frame_buffer));
        }

        SDL_DelayNS( 100 );
    }

    SDL_DestroyAudioStream(stream);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
