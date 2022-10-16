#include "render_loop.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>

static uint32_t sim_boot_time;
static bool show_fps_disp;

static t_plot_func *plot_update_func;

static SDL_Window *window;
static SDL_Renderer *renderer;


void rl_setPlotUpdateFunc(t_plot_func *plotUpdFunc)
{
  plot_update_func = plotUpdFunc;
}


void rl_setEnableFPSDisplay(bool enable)
{
  show_fps_disp = enable;
}


static int64_t rl_getRealMsTime(void)
{
  struct timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  int64_t res = (int64_t)(time.tv_sec * (time_t)1000);
  res += (int64_t)(time.tv_nsec / (long)1000000);
  return res;
}

static void updateFPSDisp(t_plot *plot, int64_t frame_time_ms)
{
  char buffer[10];

  if (!show_fps_disp)
    return;

  plot_selectFont(plot, PLOT_FONT_ID_FUTURAL, FIX_1 / 15);

  t_fixp fps = 1000 * FIX_1 / (frame_time_ms);
  formatFixedPoint(fps, 10, PLUS_NONE, false, 2, 0, buffer);
  plot_moveTo(plot, -FIX_1, FIX_1*15/16);
  plot_putString(plot, "FPS: ");
  plot_putString(plot, buffer);
}

void rl_update(void)
{
  static int64_t last_start_time = 0;
  int64_t start_time = rl_getRealMsTime();

  #define PLOT_BUFFER_SZ 256
  uint8_t plot_buffer[PLOT_BUFFER_SZ];
  t_plot plot;

  #define RENDER_BUFFER_SZ 0x10000
  uint32_t render_buffer[RENDER_BUFFER_SZ];
  t_plotRender render;

  plot_init(&plot, plot_buffer, PLOT_BUFFER_SZ);
  plot_update_func(&plot);
  updateFPSDisp(&plot, last_start_time-start_time);

  plot_renderInit(&render, render_buffer, RENDER_BUFFER_SZ);
  plot_render(&plot, &render);

  SDL_SetRenderDrawColor(renderer, 45, 66, 88, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  int cur_window_w, cur_window_h;
  SDL_GetWindowSize(window, &cur_window_w, &cur_window_h);
  float off_x = (float)cur_window_w / 2.0;
  float off_y = (float)cur_window_h / 2.0;
  float scale_x = (float)cur_window_w * 9.0 / 10.0;
  float scale_y = (float)cur_window_h * 9.0 / 10.0;
  float scale = MIN(scale_x, scale_y) / 2.0;

  SDL_SetRenderDrawColor(renderer, 206, 255, 210, SDL_ALPHA_OPAQUE);
  for (unsigned i=0; i<render.i; i++) {
    unsigned x = render.xyBuf[i] & 0xFFF;
    unsigned y = (render.xyBuf[i] >> 16) & 0xFFF;
    float fx = (float)((int)x - (int)OFF_X) / (float)AMP_X;
    float fy = (float)((int)y - (int)OFF_Y) / (float)AMP_Y;

    SDL_FRect rect;
    rect.x = fx * scale + off_x - 1.0;
    rect.y = -fy * scale + off_y - 1.0;
    rect.w = 2.0;
    rect.h = 2.0;
    SDL_RenderDrawRectF(renderer, &rect);
  }

  SDL_RenderPresent(renderer);

  int64_t end_time = rl_getRealMsTime();

  SDL_Event event;
  while (SDL_WaitEventTimeout(&event, MAX(0, (1000/61) - (end_time-start_time)))) {
    if (event.type == SDL_QUIT) {
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
      exit(0);
    }

    end_time = rl_getRealMsTime();
  }

  last_start_time = start_time;
}


uint32_t rl_getMsTime(void)
{
  return (uint32_t)rl_getRealMsTime() - sim_boot_time;
}


void rl_init(t_plot_func *plotUpdFunc)
{
  sim_boot_time = (uint32_t)rl_getRealMsTime();
  
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer);
  SDL_SetWindowTitle(window, "stm32-scopeclock");

  plot_update_func = plotUpdFunc;
  #ifdef DEBUG
  show_fps_disp = true;
  #endif
}
