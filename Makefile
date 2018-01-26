OUTDIR = bin
OUTFILE = $(OUTDIR)/fonttest
CC = gcc
EXT = c

CPPFLAGS :=
CFLAGS :=
LDFLAGS :=
#DEBUG_BUILD = -g
#DEBUG_TEST = gdb --eval-command=run
 
ifeq ($(OS),Windows_NT)
	LDFLAGS +=  -lmingw32 -lSDLmain -lSDL.dll -lglu32 -lopengl32 -static
else
	UNAME_S := $(shell uname -s)

	ifeq ($(UNAME_S),Linux)
		LDFLAGS += -lm -lSDLmain -lSDL -lGL -lGLU
	endif

	ifeq ($(UNAME_S),Darwin)
		LDFLAGS += -framework SDL -framework Cocoa
	endif
endif

objs := $(patsubst %.$(EXT),$(OUTDIR)/%.o,$(wildcard *.$(EXT)))
deps := $(objs:.o=.dep)

.PHONY: all test
all: $(OUTFILE)

-include $(deps)

$(OUTDIR)/%.o: %.$(EXT)
	@mkdir -p $(@D)
	$(CC) $(DEBUG_BUILD) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MM $< | sed -e '1,1 s|[^:]*:|$@:|' > $(OUTDIR)/$*.dep

$(OUTFILE) : $(objs)
	$(CC) $(DEBUG_BUILD) $^ $(LDFLAGS) -o $@

test: $(OUTFILE)
	@$(DEBUG_TEST) $(OUTFILE)

clean:
	@rm -f $(deps) $(objs) $(OUTFILE) $(OUTFILE).exe
	@rmdir --ignore-fail-on-non-empty $(OUTDIR)
