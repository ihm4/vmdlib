PROGRAM=vmdlib_exapmle.exe
OBJS=vmd.o example.o
CC=gcc
CCFLAGS=-O -Wall -DDEBUG
CXX=g++
CXXFLAGS=-O -Wall

all: $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) -o $(PROGRAM)

.SUFFIXES: .o .cpp .c

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(PROGRAM)
