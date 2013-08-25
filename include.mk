cppcheckflags = -I.                                                                    \
				-I..                                                                   \
				-I/usr/include/SDL                                                     \
				--enable=warning,performance,portability,unusedFunction,missingInclude \
				--force                                                                \
				--quiet

cppcheck:
	@echo "`cppcheck --version`"
	cppcheck gluem croppy src $(cppcheckflags)

gourceflags =	-c 0.8                           \
				--seconds-per-day 0.001          \
				--logo ./win32/w32icon_64x64.png \
				--auto-skip-seconds 0.0001       \
				--title "FreedroidRPG"           \
				--key                            \
				--camera-mode overview           \
				--highlight-all-users            \
				--file-idle-time 0               \
				--hide progress,mouse,filenames  \
				--stop-at-end                    \
				--max-files 99999999999          \
				--max-file-lag 0.000001          \
				--bloom-multiplier 1.3           \
				-1280x720

ffmpegflags =	-f image2pipe                    \
				-vcodec ppm                      \
				-i -                             \
				-y                               \
				-vcodec libx264                  \
				-preset medium                   \
				-crf 22                          \
				-pix_fmt yuv420p                 \
				-threads 2                       \
				-b:v 3000k                       \
				-maxrate 8000k                   \
				-bufsize 10000k FreedroidRPG.mp4


gource:
	gource $(gourceflags)

gource_ffmpeg:
	gource $(gourceflags) --output-ppm-stream - | ffmpeg $(ffmpegflags)
