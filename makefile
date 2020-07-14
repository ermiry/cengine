# TARGET      := cengine
SLIB		:= libcengine.so

SDL2 	:= `sdl2-config --cflags --libs` -l SDL2_image -l SDL2_ttf
MATH 	:= -lm 
PTHREAD := -l pthread

# development
DEVELOPMENT 	:= -D CENGINE_DEBUG
CLIENT_DEFINES	:= -D CERVER_DEBUG -D CLIENT_DEBUG -D PACKETS_DEBUG -D AUTH_DEBUG

DEFINES = $(DEVELOPMENT) $(CLIENT_DEFINES)

CC          := gcc

SRCDIR      := src
INCDIR      := include
BUILDDIR    := objs
TARGETDIR   := bin

SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

CFLAGS      := -g $(DEFINES) -Wall -Wno-unknown-pragmas -Wfatal-errors -fPIC
LIB         := $(MATH) $(PTHREAD) $(SDL2)
INC         := -I $(INCDIR) -I /usr/local/include
INCDEP      := -I $(INCDIR)

SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

# all: directories $(TARGET)
all: directories $(SLIB)

# run: 
# 	./$(TARGETDIR)/$(TARGET)

install: $(SLIB)
	install -m 644 ./bin/libcengine.so /usr/local/lib/
	cp -R ./include/cengine /usr/local/include

uninstall:
	rm /usr/local/lib/libcengine.so
	rm -r /usr/local/include/cengine

directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

clean:
	@$(RM) -rf $(BUILDDIR)	@$(RM) -rf $(TARGETDIR)
	@$(RM) -rf ./examples/bin

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

# link
# $(TARGET): $(OBJECTS)
# 	$(CC) $^ $(LIB) -o $(TARGETDIR)/$(TARGET)

$(SLIB): $(OBJECTS)
	$(CC) $^ $(LIB) -shared -o $(TARGETDIR)/$(SLIB)

# compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) $(LIB) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

examples: ./examples/welcome.c
	@mkdir -p ./examples/bin
	$(CC) -I ./include -L ./bin ./examples/welcome.c -o ./examples/bin/welcome -l cengine

.PHONY: all clean examples