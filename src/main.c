#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500
#define IMAGE_BOX_SIZE 200.0f
#define IMAGE_PATH "D:\\underway\\SDL_learn\\resources\\img\\test.png"

/**
 * @brief 绘制图片
 * @param image_surface 图片表面
 * @param image_texture 图片纹理
 * @param width 图片宽度
 * @param height 图片高度
*/
static void draw_img(SDL_Surface *image_surface, SDL_Texture *image_texture,
                     uint32_t width, uint32_t height)
{
    SDL_Renderer *renderer = NULL;
    SDL_FRect destination = { 0 };
    float scale_x;
    float scale_y;
    float scale;

    if (!image_surface || !image_texture || width == 0 || height == 0) {
        SDL_Log("draw_img received invalid arguments");
        return;
    }

    renderer = SDL_GetRendererFromTexture(image_texture);
    if (!renderer) {
        SDL_Log("Failed to get renderer from texture: %s", SDL_GetError());
        return;
    }

    /* 将图片完整地等比缩放到 width x height 的边界框内。 */
    scale_x = (float)width / (float)image_surface->w;
    scale_y = (float)height / (float)image_surface->h;
    scale = SDL_min(scale_x, scale_y);

    destination.w = (float)image_surface->w * scale;
    destination.h = (float)image_surface->h * scale;
    destination.x = ((float)WINDOW_WIDTH - destination.w) / 2.0f;
    destination.y = ((float)WINDOW_HEIGHT - destination.h) / 2.0f;

    SDL_SetTextureScaleMode(image_texture, SDL_SCALEMODE_LINEAR);
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, image_texture, NULL, &destination);
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Surface *image_surface = NULL;
    SDL_Texture *image_texture = NULL;
    bool running = true;
    bool success = false;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        goto cleanup;
    }

    if (!SDL_CreateWindowAndRenderer("Image Demo", WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                                     &window, &renderer)) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        goto cleanup;
    }

    image_surface = SDL_LoadPNG(IMAGE_PATH);
    if (!image_surface) {
        SDL_Log("Failed to load %s: %s", IMAGE_PATH, SDL_GetError());
        goto cleanup;
    }

    image_texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    if (!image_texture) {
        SDL_Log("Texture creation failed: %s", SDL_GetError());
        goto cleanup;
    }

    draw_img(image_surface, image_texture,
             (uint32_t)IMAGE_BOX_SIZE, (uint32_t)IMAGE_BOX_SIZE);
    success = true;

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
    SDL_DestroyTexture(image_texture);
    SDL_DestroySurface(image_surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return success ? 0 : 1;
}
