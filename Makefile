PREFIX = /usr/local

CFLAGS += `pkg-config --cflags vterm libcrypto`
LDFLAGS += `pkg-config --libs vterm libcrypto`
LDFLAGS += -lutil

.PHONY: all clean install uninstall

all: tuitest

tuitest: tuitest.c keymap.h
	$(CC) $(CFLAGS) -o tuitest tuitest.c  $(LDFLAGS)

clean:
	rm tuitest

install: tuitest
	install -m 755 tuitest $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/tuitest
