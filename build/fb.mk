USE_FB = $(TARGET_IS_KOBO)

ifeq ($(USE_FB),y)
USE_CONSOLE = y
USE_MEMORY_CANVAS = y
FREETYPE = y
LIBPNG = y
LIBJPEG = y
EGL = n
OPENGL = n
ENABLE_SDL = n
endif
