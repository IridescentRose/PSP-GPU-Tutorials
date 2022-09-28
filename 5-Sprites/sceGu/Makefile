TARGET = sprites
OBJS = sprites.o ../../common/callbacks.o ../../common/common-sce.o

INCDIR = ../../gu2gl/
CFLAGS = -Wall -std=c11
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lpspgum -lpspgu -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Sprite Sample

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak