#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

/* Defined by the C source generated from the TTF at build time. */
extern const unsigned char __embedded_font[];
extern const size_t __embedded_font_size;

/**
 * @note SDL_Surface 是便于 CPU 读取、修改和转换的二维像素缓冲区。
 * @note SDL_Texture 是便于渲染器高效显示的图像资源。
*/


int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_IOStream *font_stream = NULL;
    TTF_Font *font = NULL;
    SDL_Surface *text_surface = NULL;
    SDL_Texture *text_texture = NULL;
    bool running = true;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) {
        SDL_Log("Initialization failed: %s", SDL_GetError());
        goto cleanup;
    }

    if (!SDL_CreateWindowAndRenderer("helloWorld", 800, 500, 0,
                                     &window, &renderer)) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        goto cleanup;
    }

    /* 直接打开内存中的ttf文件流 */
    font_stream = SDL_IOFromConstMem(__embedded_font, __embedded_font_size);

    /* 打开字体 */
    font = TTF_OpenFontIO(font_stream, true, 36.0f);
    if (!font) {
        SDL_Log("TTF_OpenFontIO failed: %s", SDL_GetError());
        goto cleanup;
    }

    /* 关闭文件流 */
    font_stream = NULL; /* TTF_CloseFont will close it. */

    /* 渲染文本到surface */
    text_surface = TTF_RenderText_Blended(font, "HelloWorld", 0,
                                          (SDL_Color){ 255, 255, 255, 128 });
    if (!text_surface )
    {
        SDL_Log("Text rendering failed: %s", SDL_GetError());
        goto cleanup;
    }

    /* 创建纹理(texture) */
    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    /* 使用黑色清空渲染器 */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /* 设置纹理的渲染位置和大小 */
    SDL_FRect destination;
    memset(&destination, 0, sizeof(SDL_FRect));
    destination.x = (800.0f - text_surface->w) / 2.0f;
    destination.y = (500.0f - text_surface->h) / 2.0f;
    destination.w = (float)text_surface->w;
    destination.h = (float)text_surface->h;

    /* 使用渲染器渲染纹理 */
    SDL_RenderTexture(renderer, text_texture, NULL, &destination);

    /* 显示渲染器内容到窗口 */
    SDL_RenderPresent(renderer);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                running = false;
            }
        }
        
        SDL_Delay(16);
    }

cleanup:
    SDL_DestroyTexture(text_texture);
    SDL_DestroySurface(text_surface);
    TTF_CloseFont(font);
    if (font_stream) SDL_CloseIO(font_stream);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return text_texture ? 0 : 1;
}
