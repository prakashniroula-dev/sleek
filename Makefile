########################################################################
####################### Makefile Template (2‑level) ####################
########################################################################

# Compiler settings - Can be customized.
CC = gcc
CXXFLAGS = -std=c11 -Wall -Isrc/include
LDFLAGS = 

# Makefile settings - Can be customized.
APPNAME = sleek
EXT = .c
SRCDIR = src
OBJDIR = obj

############## Do not change anything from here downwards! #############
# Find .c files in src/ and in src/*/ (two levels)
SRC = $(wildcard $(SRCDIR)/*$(EXT)) \
      $(wildcard $(SRCDIR)/*/*$(EXT))

OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
DEP = $(OBJ:$(OBJDIR)/%.o=$(OBJDIR)/%.d)

# UNIX-based OS variables & settings
RM = rm
DELOBJ = $(OBJ)
# Windows OS variables & settings
DEL = del
EXE = .exe
# Windows alternative (not used unless you call cleanw)
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)

########################################################################
####################### Targets beginning here #########################
########################################################################

all: $(APPNAME)

# Builds the app
$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Creates the dependency rules
%.d: $(SRCDIR)/%$(EXT)
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:%.d=$(OBJDIR)/%.o) >$@

# Includes all .h files
-include $(DEP)

# Building rule for .o files – creates obj/ subdirectories as needed
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT)
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -o $@ -c $<

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) -rf $(OBJDIR) $(APPNAME)

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) $(DEP)

#################### Cleaning rules for Windows OS #####################
# Cleans complete project
.PHONY: cleanw
cleanw:
	$(DEL) /Q $(WDELOBJ) $(DEP) $(APPNAME)$(EXE)
	# Remove obj directory (Windows rmdir)
	-if exist $(OBJDIR) rmdir /S /Q $(OBJDIR)

# Cleans only all files with the extension .d
.PHONY: cleandepw
cleandepw:
	$(DEL) $(DEP)