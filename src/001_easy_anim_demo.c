#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

#define BACKGROUND_COLOR    0, 0, 0
#define DEFAULT_COLOR       255, 255, 255

/**
 * @brief 用水平弦画实心圆
 * @param renderer 渲染器
 * @param r 红色
 * @param g 绿色
 * @param b 蓝色
 * @param cx 圆心x坐标
 * @param cy 圆心y坐标
 * @param radius 半径
*/
static void draw_circle(SDL_Renderer *renderer, uint8_t r, uint8_t g, uint8_t b, int cx, int cy, int radius)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    for (uint32_t line = 0; line < (uint32_t)radius * 2; line++)
    {
        int32_t dy = (int32_t)line - radius;
        int32_t chord = 2 * (int32_t)sqrt((double)(radius * radius) - (double)(dy * dy));
        int32_t start_x = cx - chord / 2;
        int32_t end_x = cx + chord / 2;
        int32_t y = cy - radius + (int32_t)line;
        SDL_RenderLine(renderer, (float)start_x, (float)y, (float)end_x, (float)y);
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    bool running = true;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("SDL3 Hello World", 800, 500, 0,
                                     &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    /* 刷黑背景 */
    // 1、设置渲染器颜色
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR , 255 );
    // 2、清空渲染器
    SDL_RenderClear(renderer);
    // 3、提交渲染
    SDL_RenderPresent(renderer);

    /* 正弦波相位 */
    float sin_pos = 0;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN &&
                 event.key.key == SDLK_ESCAPE)) {
                running = false;
            }
        }

        sin_pos += 0.08f;

        //清屏
        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR , 255 );
        SDL_RenderClear(renderer);

        //画圆
        draw_circle(renderer, DEFAULT_COLOR, 400, 250, 150 + (int)(100 * sin(sin_pos)));

        //提交渲染
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
