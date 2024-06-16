# Compiler
CC = aarch64-linux-gnu-g++

SYSROOT = /home/victor/coding/projects/ninkasi/sysroot

TARGET_HOST = victor@ninkasi

INCLUDES = \
	-I./include \
	-I./include/mavlink \
	-I./websocketpp \
 	-I$(SYSROOT)/lib  \
	-I$(SYSROOT)/usr/lib \
	-I$(SYSROOT)/usr/include  \
	-I$(SYSROOT)/usr/include/libcamera \
	-I$(SYSROOT)/usr/include/libcamera-apps \
	-I$(SYSROOT)/usr/include/opencv4 \
	-I$(SYSROOT)/usr/include/aarch64-linux-gnu

LINKER_LIBS = \
	-L$(SYSROOT)/usr/lib \
	-L$(SYSROOT)/lib \
	-L$(SYSROOT)/usr/lib/aarch64-linux-gnu

# Compiler flags
CCFLAGS = -Wall -g -std=c++17 -O0 --sysroot=$(SYSROOT) $(INCLUDES)
CCFLAGS += -Wno-address-of-packed-member  # This is known warning from mavlink

LIBCAMERA_LIBS = -lcamera -lcamera-base

# Linker flags
LDFLAGS = $(LINKER_LIBS) -lpthread -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lopencv_imgproc -larmadillo -llapack -lblas $(LIBCAMERA_LIBS)


# Source files directory and specific source files
SRC = src
SRCS = \
	$(SRC)/mavcom.cpp \
	$(SRC)/utils.cpp \
	$(SRC)/command_server.cpp \
	$(SRC)/telemetry_server.cpp \
	$(SRC)/mavproxy.cpp \
	$(SRC)/simple_camera.cpp \
	$(SRC)/video_server.cpp \
	$(SRC)/mapped_framebuffer.cpp \
	$(SRC)/ap.cpp \
	$(SRC)/ws_server.cpp \
	$(SRC)/log_server.cpp \
	$(SRC)/mission/mission.cpp \
	$(SRC)/mission/state.cpp \
	$(SRC)/main.cpp


# Object files directory
BUILD_DIR = build
OBJS = $(patsubst $(SRC)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Dependency files
DEPS = $(OBJ:.o=.d)

# Executable name
TARGET = ninkasi

# Default rule
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	@$(CC) $(CCFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo
	@echo "Build finished: $(TARGET)"
	@echo

# Compile source files to object files in build directory
$(BUILD_DIR)/%.o: $(SRC)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "  CC $<"
	@$(CC) $(CCFLAGS) -c $< -o $@

-include $(DEPS)

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean rule
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

deploy:
	scp $(TARGET) video.py $(TARGET_HOST):/home/victor/ninkasi/

deploy_py:
	scp video.py $(TARGET_HOST):/home/victor/ninkasi/

sync:
	rsync -avz $(TARGET_HOST):/lib sysroot/
	rsync -avz $(TARGET_HOST):/usr/include sysroot/usr/
	rsync -avz $(TARGET_HOST):/usr/lib sysroot/usr/
	rsync -avz $(TARGET_HOST):/usr/local sysroot/usr/
	rsync -avz $(TARGET_HOST):/etc/alternatives sysroot/etc/

fix_broken_symlinks:
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/libblas.so.3 /home/victor/projects/ninkasi/sysroot/etc/alternatives/libblas.so.3-aarch64-linux-gnu
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/libblas.so /home/victor/projects/ninkasi/sysroot/etc/alternatives/libblas.so-aarch64-linux-gnu
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/include/aarch64-linux-gnu/cblas-atlas.h /home/victor/projects/ninkasi/sysroot/usr/include/aarch64-linux-gnu/cblas.h
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/libblas.a /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/libblas.a
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/libblas.so /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/libblas.so
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/libblas.so.3 /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/libblas.so.3
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig/blas-atlas.pc /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig/blas.pc
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig/lapack-atlas.pc /home/victor/projects/ninkasi/sysroot/etc/alternatives/lapack.pc-aarch64-linux-gnu
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/liblapack.so /home/victor/projects/ninkasi/sysroot/etc/alternatives/liblapack.so-aarch64-linux-gnu
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/liblapack.a /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/liblapack.a
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/liblapack.so /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/liblapack.so
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/atlas/liblapack.so.3.10.3 /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/liblapack.so.3
	ln -sf /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig/lapack-atlas.pc /home/victor/projects/ninkasi/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig/lapack.pc


gen_telemetry:
	tools/convert_telemetry.py -i $(SRC)/telemetry.h -o frontend/frontkasi/src/Telemetry.js

# export DESTDIR=/home/victor/coding/projects/ninkasi/include/uWebSockets && export QUIC=1 && export_BORINGSSL=1 && make && make install
# cp uSockets/*.a ../libs/

# Phony targets
.PHONY: all clean deploy deploy_py sync gen_telemetry
