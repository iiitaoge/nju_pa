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

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = 0x10000; // 流缓冲区大小
  outl(AUDIO_SBUF_SIZE_ADDR, cfg->bufsize);
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
    uint32_t count = inl(AUDIO_COUNT_ADDR);  // 当前已使用的缓冲区大小
    uint32_t len = ctl->buf.end - ctl->buf.start;
    uint32_t sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);  // 获取缓冲区的总大小
    uint8_t* sbuf = (uint8_t *)(uintptr_t)AUDIO_SBUF_ADDR;  // 获取缓冲区基址

    // 如果缓冲区可用空间不足，等待直到足够空间可用
    while (count + len > sbuf_size) {
        count = inl(AUDIO_COUNT_ADDR);  // 更新已用缓冲区大小
    }

    // 计算写入位置
    uint32_t write_pos = count % sbuf_size;

    // 判断是否需要分段写入
    if (write_pos + len > sbuf_size) {
        // 分段写入
        uint32_t first_part = sbuf_size - write_pos;
        memcpy(sbuf + write_pos, ctl->buf.start, first_part);
        memcpy(sbuf, ctl->buf.start + first_part, len - first_part);
    } else {
        // 单次写入
        memcpy(sbuf + write_pos, ctl->buf.start, len);
    }

    // 更新已使用的缓冲区大小
    count += len;
    outl(AUDIO_COUNT_ADDR, count);  // 更新 count 寄存器
}


