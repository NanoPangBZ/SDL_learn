#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 420
#define WINDOW_MIN_WIDTH  320
#define WINDOW_MIN_HEIGHT 200

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

    /**
     * @brief 窗口标志位演示
     * @note 
     * - BORDERLESS:  移除窗口的边框
     * - TRANSPARENT: 保留窗口的透明度
     * - RESIZABLE:  允许用户通过边缘调整窗口大小
     * - FULLSCREEN: 全屏模式
    */
    const SDL_WindowFlags window_flags = SDL_WINDOW_TRANSPARENT |
                                         SDL_WINDOW_RESIZABLE;

    window = SDL_CreateWindow("SDL Window Flags Demo",
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              window_flags);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        goto cleanup;
    }
    SDL_SetWindowMinimumSize(window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT);

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        goto cleanup;
    }

    SDL_SetRenderVSync(renderer, 1);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        int width;
        int height;
        SDL_GetWindowSizeInPixels(window, &width, &height);

        /**
         * @note 设置渲染混合模式为透明混合，让fill的矩形混合原来屏幕的像素
        */
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 64, 160, 255, 180);
        const SDL_FRect rectangle = {
            (float)width * 0.2f,
            (float)height * 0.2f,
            (float)width * 0.6f,
            (float)height * 0.6f
        };
        SDL_RenderFillRect(renderer, &rectangle);

        SDL_RenderPresent(renderer);
    }

cleanup:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return renderer ? 0 : 1;
}
