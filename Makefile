compile:
	gcc `pkg-config --cflags gtk+-3.0` -rdynamic -o point-drawer main.c `pkg-config --libs gtk+-3.0`

clear:
	rm point-drawer || true

run: clear compile
	./point-drawer
