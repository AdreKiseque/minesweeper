ifeq ($(OS),Windows_NT)
	LDFLAGS += -lopengl32 -lgdi32
else
	UNAME_S := $(shell uname)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS += -framework OpenGL -framework Cocoa
	else ifeq ($(UNAME_S),Linux)
		LDFLAGS += -lGLU -lGL -lX11 -lrt
	endif
endif

minesweeper : minesweeper.c graphics.c tigr/tigr.c
	gcc $^ -g -o $@ $(CFLAGS) $(LDFLAGS) -lm
