#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // 2、清空渲染器
    SDL_RenderClear(renderer);

    /* 画一个圆到正中心 */
    // 1、设置渲染器颜色
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // 2、用横线画实心圆
    uint32_t radius = 100;
    int32_t cx = 400;
    int32_t cy = 250;
    for (uint32_t line = 0; line < radius * 2; line++)
    {
        // 计算弦长: 2 * sqrt(r² - dy²)
        int32_t dy = (int32_t)line - (int32_t)radius;
        int32_t chord = 2 * (int32_t)sqrt((double)(radius * radius) - (double)(dy * dy));

        // 弦起始和结束点（相对窗口中心）
        int32_t start_x = cx - chord / 2;
        int32_t end_x = cx + chord / 2;
        int32_t y = cy - (int32_t)radius + (int32_t)line;

        // 画弦
        SDL_RenderLine(renderer, (float)start_x, (float)y, (float)end_x, (float)y);
    }

    /* 提交渲染 */
    SDL_RenderPresent(renderer);

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN &&
                 event.key.key == SDLK_ESCAPE)) {
                running = false;
            }
        }

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
