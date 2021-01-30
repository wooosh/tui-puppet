BUILDDIR = build
TARGET = tuitest
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

CFLAGS += `pkg-config --cflags vterm libcrypto`
CFLAGS += -g
LDFLAGS += `pkg-config --libs vterm libcrypto`
LDFLAGS += -lutil

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

-include $(DEP)

build/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -MMD -MP -o $@ -c $< $(LDFLAGS) 

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	$(RM) -r $(BUILDDIR)
	$(RM) $(TARGET)
