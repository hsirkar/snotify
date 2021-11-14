all: snotify

snotify: snotify.c
	gcc snotify.c -o snotify

clean:
	rm snotify
