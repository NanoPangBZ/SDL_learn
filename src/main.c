#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

extern const unsigned char __embedded_font[];
extern const size_t __embedded_font_size;

#define BACKGROUND_COLOR    0, 0, 0
#define SPECTRUM_MAX_FREQUENCY 1000
#define SPECTRUM_FREQUENCY_STEP 0.5f
#define SPECTRUM_POINT_COUNT 2001
#define SPECTRUM_LABEL_STEP 50
#define CURVE_SEGMENTS_PER_POINT 8
#define FFT_WINDOW_SIZE 16384
#define FFT_HOP_SIZE 1024
#define CAPTURE_CHANNELS 1
#define SPECTRUM_AMPLITUDE_GAIN 20.0f

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
 * @brief 在左上角绘制峰值频率文字
 */
static void draw_peak_frequency(SDL_Renderer *renderer, TTF_Font *font,
                                float peak_frequency, float peak_amplitude)
{
    char label[64];
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_FRect destination;

    if (font == NULL) {
        return;
    }

    SDL_snprintf(label, sizeof(label), "Peak: %.1f Hz  Amp: %.3f",
                 peak_frequency, peak_amplitude);
    surface = TTF_RenderText_Blended(font, label, 0,
                                     (SDL_Color){ 240, 240, 240, 255 });
    if (surface == NULL) {
        return;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture != NULL) {
        destination.x = 12.0f;
        destination.y = 8.0f;
        destination.w = (float)surface->w;
        destination.h = (float)surface->h;
        SDL_RenderTexture(renderer, texture, NULL, &destination);
        SDL_DestroyTexture(texture);
    }
    SDL_DestroySurface(surface);
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
 * @param sample_rate 采样率
 * @param fre_point_buffer 频率点缓冲
 * @param fre_point_count 频率点数
*/
static void wave_fft(const float *wave_buffer, int sample_rate,
                     fre_point_t *fre_point_buffer, uint32_t fre_point_count)
{
    static float real[FFT_WINDOW_SIZE];
    static float imaginary[FFT_WINDOW_SIZE];
    uint32_t i;
    uint32_t reversed = 0;

    if (wave_buffer == NULL || fre_point_buffer == NULL || sample_rate <= 0) {
        return;
    }

    /* Hann 窗减少频谱泄漏；16384 点窗口比原来的约 4410 点窗口更宽。 */
    for (i = 0; i < FFT_WINDOW_SIZE; ++i) {
        const float window = 0.5f - 0.5f * SDL_cosf(
            2.0f * SDL_PI_F * (float)i / (float)(FFT_WINDOW_SIZE - 1));
        real[i] = wave_buffer[i] * window;
        imaginary[i] = 0.0f;
    }

    /* 原地 radix-2 Cooley-Tukey FFT：O(N log N)。 */
    for (i = 1; i < FFT_WINDOW_SIZE; ++i) {
        uint32_t bit = FFT_WINDOW_SIZE >> 1;
        while (reversed & bit) {
            reversed ^= bit;
            bit >>= 1;
        }
        reversed ^= bit;
        if (i < reversed) {
            const float temporary_real = real[i];
            const float temporary_imaginary = imaginary[i];
            real[i] = real[reversed];
            imaginary[i] = imaginary[reversed];
            real[reversed] = temporary_real;
            imaginary[reversed] = temporary_imaginary;
        }
    }

    for (uint32_t length = 2; length <= FFT_WINDOW_SIZE; length <<= 1) {
        const float angle = -2.0f * SDL_PI_F / (float)length;
        const float step_real = SDL_cosf(angle);
        const float step_imaginary = SDL_sinf(angle);
        const uint32_t half_length = length >> 1;

        for (i = 0; i < FFT_WINDOW_SIZE; i += length) {
            float twiddle_real = 1.0f;
            float twiddle_imaginary = 0.0f;
            for (uint32_t j = 0; j < half_length; ++j) {
                const uint32_t even = i + j;
                const uint32_t odd = even + half_length;
                const float odd_real = real[odd] * twiddle_real -
                                       imaginary[odd] * twiddle_imaginary;
                const float odd_imaginary = real[odd] * twiddle_imaginary +
                                            imaginary[odd] * twiddle_real;
                const float next_twiddle_real = twiddle_real * step_real -
                                                twiddle_imaginary * step_imaginary;

                real[odd] = real[even] - odd_real;
                imaginary[odd] = imaginary[even] - odd_imaginary;
                real[even] += odd_real;
                imaginary[even] += odd_imaginary;
                twiddle_imaginary = twiddle_real * step_imaginary +
                                    twiddle_imaginary * step_real;
                twiddle_real = next_twiddle_real;
            }
        }
    }

    for (i = 0; i < fre_point_count; ++i) {
        const float exact_bin = fre_point_buffer[i].frequency *
                                (float)FFT_WINDOW_SIZE / (float)sample_rate;
        const uint32_t lower_bin = (uint32_t)exact_bin;
        const uint32_t upper_bin = SDL_min(lower_bin + 1,
                                           FFT_WINDOW_SIZE / 2);
        const float fraction = exact_bin - (float)lower_bin;
        const float lower_magnitude = SDL_sqrtf(real[lower_bin] * real[lower_bin] +
                                                imaginary[lower_bin] * imaginary[lower_bin]);
        const float upper_magnitude = SDL_sqrtf(real[upper_bin] * real[upper_bin] +
                                                imaginary[upper_bin] * imaginary[upper_bin]);
        /* Hann 窗的 coherent gain 约为 0.5，单边谱比例因此为 4/N。 */
        fre_point_buffer[i].amplitude =
            (lower_magnitude + (upper_magnitude - lower_magnitude) * fraction) *
            (4.0f / (float)FFT_WINDOW_SIZE);
    }
}

/**
 * @brief 查找与默认播放设备同名的环回录制设备（WASAPI loopback）
 */
static SDL_AudioDeviceID find_system_loopback_device(void)
{
    const char *playback_name =
        SDL_GetAudioDeviceName(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
    int recording_count = 0;
    SDL_AudioDeviceID *recording_devices =
        SDL_GetAudioRecordingDevices(&recording_count);
    SDL_AudioDeviceID loopback = 0;
    int i;

    if (recording_devices == NULL || recording_count <= 0) {
        SDL_Log("No recording devices available: %s", SDL_GetError());
        return 0;
    }

    if (playback_name != NULL) {
        for (i = 0; i < recording_count; ++i) {
            const char *name = SDL_GetAudioDeviceName(recording_devices[i]);
            if (name != NULL && SDL_strcmp(name, playback_name) == 0) {
                loopback = recording_devices[i];
                break;
            }
        }
    }

    if (loopback == 0) {
        SDL_Log("No loopback device matching playback '%s'. "
                "Available recording devices:",
                playback_name != NULL ? playback_name : "(unknown)");
        for (i = 0; i < recording_count; ++i) {
            SDL_Log("  [%d] %s", i, SDL_GetAudioDeviceName(recording_devices[i]));
        }
    }

    SDL_free(recording_devices);
    return loopback;
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_AudioStream *stream = NULL;
    TTF_Font *font = NULL;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID loopback_device = 0;
    bool running = true;
    bool fullscreen = false;
    float peak_frequency = 0.0f;
    float peak_amplitude = 0.0f;

    /* 16384 点分析窗口，每次滑动 1024 点。 */
    static float spectrum_input[FFT_WINDOW_SIZE];
    static float capture_chunk[FFT_HOP_SIZE * CAPTURE_CHANNELS];
    fre_point_t frequency_points[SPECTRUM_POINT_COUNT];
    float spectrum_wave[SPECTRUM_POINT_COUNT];

    (void)argc;
    (void)argv;

    /* Windows WASAPI：把播放设备也列为录制设备，从而启用系统输出环回。 */
    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) || !TTF_Init()) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("SDL3 System Audio Spectrum", 800, 500,
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

    loopback_device = find_system_loopback_device();
    if (loopback_device == 0) {
        SDL_Log("Failed to find system audio loopback device. "
                "Set SDL_AUDIO_INCLUDE_MONITORS=1 before init (already set).");
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    if (!SDL_GetAudioDeviceFormat(loopback_device, &spec, NULL)) {
        SDL_Log("SDL_GetAudioDeviceFormat failed: %s", SDL_GetError());
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    /* 统一转换为单声道 F32，便于 FFT。 */
    spec.format = SDL_AUDIO_F32;
    spec.channels = CAPTURE_CHANNELS;

    stream = SDL_OpenAudioDeviceStream(loopback_device, &spec, NULL, NULL);
    if (!stream) {
        SDL_Log("SDL_OpenAudioDeviceStream (loopback) failed: %s", SDL_GetError());
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Log("Capturing system audio: '%s', %d Hz, %d channels",
            SDL_GetAudioDeviceName(loopback_device),
            spec.freq, spec.channels);
    SDL_ResumeAudioStreamDevice(stream);

    for (uint32_t i = 0; i < SPECTRUM_POINT_COUNT; ++i) {
        frequency_points[i].frequency = (float)(i * SPECTRUM_FREQUENCY_STEP);
        frequency_points[i].amplitude = 0.0f;
        spectrum_wave[i] = -1.0f;
    }

    while (running) {
        SDL_Event event;
        const int hop_bytes =
            FFT_HOP_SIZE * CAPTURE_CHANNELS * (int)sizeof(float);
        bool spectrum_updated = false;

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

        /* 尽量排空积压，只保留最新数据，保证频谱实时。 */
        while (SDL_GetAudioStreamAvailable(stream) >= hop_bytes) {
            const int got = SDL_GetAudioStreamData(stream, capture_chunk, hop_bytes);
            if (got < hop_bytes) {
                break;
            }

            SDL_memmove(spectrum_input,
                        spectrum_input + FFT_HOP_SIZE,
                        (FFT_WINDOW_SIZE - FFT_HOP_SIZE) * sizeof(float));
            SDL_memcpy(spectrum_input + (FFT_WINDOW_SIZE - FFT_HOP_SIZE),
                       capture_chunk,
                       FFT_HOP_SIZE * sizeof(float));
            spectrum_updated = true;
        }

        if (spectrum_updated) {
            wave_fft(spectrum_input, spec.freq,
                     frequency_points, SPECTRUM_POINT_COUNT);

            peak_frequency = 0.0f;
            peak_amplitude = 0.0f;
            for (uint32_t i = 0; i < SPECTRUM_POINT_COUNT; ++i) {
                const float scaled_amplitude = SDL_clamp(
                    frequency_points[i].amplitude * SPECTRUM_AMPLITUDE_GAIN,
                    0.0f, 1.0f);
                /* draw_wave 使用 [-1, 1]，将幅值 [0, 1] 映射为底部到顶部。 */
                float display_value = scaled_amplitude * 2.0f - 1.0f;
                spectrum_wave[i] = SDL_clamp(display_value, -1.0f, 1.0f);
                if (scaled_amplitude > peak_amplitude) {
                    peak_amplitude = scaled_amplitude;
                    peak_frequency = frequency_points[i].frequency;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR, 255);
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
            draw_peak_frequency(renderer, font, peak_frequency, peak_amplitude);
        }
        SDL_RenderPresent(renderer);

        SDL_DelayNS(SDL_MS_TO_NS(1));
    }

    SDL_DestroyAudioStream(stream);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
