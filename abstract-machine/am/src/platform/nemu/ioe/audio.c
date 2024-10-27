#include <am.h>
#include <nemu.h>

#include <klib.h>

// 这些都是初始化的
#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)


#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static int count = 0;
static int len = 0;

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = 0x10000; // 流缓冲区大小
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples); 
  // 写入完毕了
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = count;
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  len = ctl->buf.end - ctl->buf.start;
  uint32_t* audio = (uint32_t *)(uintptr_t)AUDIO_SBUF_ADDR;  // 写入流缓冲区
  memcpy(audio, ctl->buf.start, len);  // 把音频数据存到这个空间里面 sbuf
  count = inl(AUDIO_COUNT_ADDR);
  outl(AUDIO_COUNT_ADDR, count + len); // 必须先强转成 uint32
}
