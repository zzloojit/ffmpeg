client:surface.c sdl.c decode.c
	gcc surface.c sdl.c decode.c -o client -lpthread `pkg-config sdl --cflags --libs` -I/home/suncloud/ffmpeg/ffmpeg_build/include -L/home/suncloud/ffmpeg/ffmpeg_build/lib -lavdevice -lavformat -lavfilter  -lavcodec -lswresample -lswscale -lavutil -lm -lpthread -lz -lx264 -g
clean:
	rm client