#include <am.h>

#define VGA_ADDR 0x21000000

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->has_accel = false;
    cfg->width = 640;
    cfg->height = 480;
    cfg->vmemsz = 0;    //???
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;

  if (w == 0 || h == 0) return;

  uint32_t *fb = (uint32_t *)VGA_ADDR;
  uint32_t *src = (uint32_t *)ctl->pixels;
  for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
        int haddr = x + j;         // 水平地址（列）
        int vaddr = (y + i) & 0x1FF;  // 垂直地址（行，取最低9位）
        fb[(haddr << 9) | vaddr] = src[i * w + j];
        // fb[(y + i) * 640 + (x + j)] = src[i*w + j];
      }
  }
  
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
