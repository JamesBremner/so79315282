#source file search paths
VPATH = ./src ../sqliteClass/src ../../code_ext/sqlite ../raven-set

	# compiler include search paths
INCS = -I../sqliteClass/src \
	-I../../code_ext/sqlite \
	-I../raven-set \
	-I../../code_ext/boost/boost1_78 \

LIBS = \
	-lgdiplus -lgdi32 -lcomdlg32 -lstdc++fs \
	#-mwindows

static_link_flags = -static-libstdc++ -static-libgcc -static -lComctl32

# folders
ODIR = ./.vscode/obj

OPT = -O2
CCflags = -std=c++17 $(OPT) -D_mingw_

# sources
OBJ_starter = \
	$(ODIR)/main.o \
	$(ODIR)/sqliteClass.o \
	$(ODIR)/sqlite3.o \
	$(ODIR)/cRunWatch.o \

### COMPILE

$(ODIR)/sqlite3.o : 
	gcc	 -o $@ -c ../../code_ext/sqlite/sqlite3.c

$(ODIR)/%.o: %.cpp 
	g++ $< \
	-c -o $@  $(INCS) $(CCflags)

starter: $(OBJ_starter)
	g++ $(OPT) \
	-o bin/starter.exe $^  \
	$(static_link_flags) $(LIBS)

.PHONY: clean
clean:
	del /q .vscode\\obj\\*.o

