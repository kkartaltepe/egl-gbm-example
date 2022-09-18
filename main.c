// Build with: gcc main.c -o example -lEGL -lgbm
#include <fcntl.h>
#include <stdio.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gbm.h>

static const EGLint cfg_attribs[] = {
    EGL_RED_SIZE,  8, EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE, 8, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE,
};

static const EGLint ctx_attribs[] = {
    EGL_CONTEXT_MAJOR_VERSION, 4, EGL_CONTEXT_MINOR_VERSION, 5, EGL_NONE,
};

int main(int arv, char **argc) {
  int drm_fd = open("/dev/dri/renderD128", O_RDWR);
  if (drm_fd < 0) {
    perror("Failed to open drm render node");
    return 1;
  }

  struct gbm_device *gbm_device = gbm_create_device(drm_fd);
  if (!gbm_device) {
    fprintf(stderr, "Failed to create gbm device\n");
    return 1;
  }

  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
  eglGetPlatformDisplayEXT =
      (void *)eglGetProcAddress("eglGetPlatformDisplayEXT");

  EGLint major, minor;
  EGLDisplay dpy =
      eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, gbm_device, NULL);
  if (dpy == EGL_NO_DISPLAY || !eglInitialize(dpy, &major, &minor)) {
    fprintf(stderr, "loading EGL on GBM failed\n");
    return 1;
  }

  printf("Initialized EGL: %d.%d\n", major, minor);
  eglBindAPI(EGL_OPENGL_API);
  EGLint num_configs;
  EGLConfig cfg;
  eglChooseConfig(dpy, cfg_attribs, &cfg, 1, &num_configs);
  EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctx_attribs);
  if (ctx == EGL_NO_CONTEXT ||
      !eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
    fprintf(stderr, "failed to make EGL context\n");
    return 1;
  }

  eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE,
                 EGL_NO_CONTEXT);
  eglDestroyContext(dpy, ctx);
  eglTerminate(dpy); // Crashes on NVIDIA
  eglReleaseThread();
}
