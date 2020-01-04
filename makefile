TARGET      := example

SDL2 	:= `sdl2-config --cflags --libs` -l SDL2_image -l SDL2_ttf
MATH 	:= -lm 
PTHREAD := -l pthread

# development
DEVELOPMENT = -D CENGINE_DEBUG

DEFINES = $(DEVELOPMENT)

CC          := gcc

SRCDIR      := src
INCDIR      := include
BUILDDIR    := objs
TARGETDIR   := bin
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

CFLAGS      := -g $(DEFINES)
LIB         := $(MATH) $(PTHREAD) $(SDL2)
INC         := -I $(INCDIR) -I /usr/local/include
INCDEP      := -I $(INCDIR)

SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

all: directories $(TARGET)

run: 
	./$(TARGETDIR)/$(TARGET)

remake: clean all

directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)	@$(RM) -rf $(TARGETDIR)

# full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

# link
$(TARGET): $(OBJECTS)
	$(CC) $^ $(LIB) -o $(TARGETDIR)/$(TARGET)

# compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(LIB) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

# non-file Targets
.PHONY: all remake clean cleaner resources