/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Lucurious Labs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <lucom.h>
#include <wlu/vlucur/vkall.h>
#include <wlu/vlucur/gp.h>
#include <wlu/wclient/client.h>
#include <wlu/shader/shade.h>
#include <wlu/utils/log.h>

#include <wlu/utils/errors.h>

#include <signal.h>

/*
 * global struct used to copy data over
 * and pass it into the signal handler
 * I'm assuming that the user called the signal
 * handler each time a new process is created
 */
static struct wlu_sig_info {
  pid_t pid;

  uint32_t app_pos;
  vkcomp **apps;

  uint32_t wc_pos;
  wclient **wcs;

  uint32_t shader_mod_pos;
  struct app_shader {
    vkcomp *app;
    VkShaderModule *shader_mod;
  } *apsh;

  uint32_t shi_pos;
  wlu_shader_info **shinfos;
} wsi;

static void signal_handler(int sig) {

  wlu_log_me(WLU_DANGER, "[x] Process ID: %d | Received signal: %i", wsi.pid, sig);
  wlu_log_me(WLU_DANGER, "[x] Caught and freeing memory for");

  for (uint32_t i = 0; i < wsi.shader_mod_pos; i++) {
    if (wsi.apsh && wsi.apsh[i].shader_mod) {
      wlu_log_me(WLU_DANGER, "[x] shader module: %p", wsi.apsh[i].shader_mod);
      wlu_freeup_shader(wsi.apsh[i].app, wsi.apsh[i].shader_mod);
    }
  }

  for (uint32_t i = 0; i < wsi.shi_pos; i++) {
    if (wsi.shinfos && wsi.shinfos[i]) {
      wlu_log_me(WLU_DANGER, "[x] shader info: %p", wsi.shinfos[i]);
      wlu_freeup_shi(wsi.shinfos[i]);
    }
  }

  for (uint32_t i = 0; i < wsi.app_pos; i++) {
    if (wsi.apps && wsi.apps[i]) {
      wlu_log_me(WLU_DANGER, "[x] vkcomp struct: %p", wsi.apps[i]);
      wlu_freeup_vk(wsi.apps[i]);
    }
  }

  for (uint32_t i = 0; i < wsi.wc_pos; i++) {
    if (wsi.wcs && wsi.wcs[i]) {
      wlu_log_me(WLU_DANGER, "[x] wclient struct: %p", wsi.wcs[i]);
      wlu_freeup_wc(wsi.wcs[i]);
    }
  }

  wlu_freeup_watchme();

  wlu_log_me(WLU_SUCCESS, "Successfully freed up most allocated memory :)");

  exit(EXIT_FAILURE);
}

int wlu_watch_me(int sig, pid_t pid) {
  wsi.pid = pid;

  /* ignore whether it works or not */
  if (signal(sig, signal_handler) == SIG_IGN)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

/* Leave this the way it is future me... */
void wlu_add_watchme_info(
  uint32_t app_pos,
  vkcomp *app,
  uint32_t wc_pos,
  wclient *wc,
  uint32_t shader_mod_pos,
  VkShaderModule *shader_mod,
  uint32_t shi_pos,
  void *shinfo
) {

  if (app) {
    wsi.app_pos = app_pos;
    wsi.apps = realloc(wsi.apps, wsi.app_pos * sizeof(vkcomp));
    wsi.apps[wsi.app_pos-1] = app;
  }

  if (wc) {
    wsi.wc_pos = wc_pos;
    wsi.wcs = realloc(wsi.wcs, wsi.wc_pos * sizeof(wclient));
    wsi.wcs[wsi.wc_pos-1] = wc;
  }

  if (shader_mod) {
    wsi.shader_mod_pos = shader_mod_pos;
    wsi.apsh = realloc(wsi.apsh, wsi.shader_mod_pos * sizeof(struct app_shader));
    wsi.apsh[wsi.shader_mod_pos-1].app = app;
    wsi.apsh[wsi.shader_mod_pos-1].shader_mod = shader_mod;
  }

  if (shinfo) {
    wsi.shi_pos = shi_pos;
    wsi.shinfos = realloc(wsi.shinfos, wsi.shi_pos * sizeof(wclient));
    wsi.shinfos[wsi.shi_pos-1] = shinfo;
  }
}

void wait_seconds(int seconds) {
  sleep(seconds);
}

void wlu_freeup_watchme() {
  if (wsi.apsh) { free(wsi.apsh); wsi.apsh = NULL; }
  if (wsi.shinfos) { free(wsi.shinfos); wsi.shinfos = NULL; }
  if (wsi.apps) { free(wsi.apps); wsi.apps = NULL; }
  if (wsi.wcs) { free(wsi.wcs); wsi.wcs = NULL; }
}
