PROGRAM=vmdlib_exapmle.exe
OBJS=vmd.o example.o
CC=gcc
CCFLAGS=-O -Wall
CXX=g++
CXXFLAGS=-O -Wall

all: $(OBJS)
	$(CC) $(CCFLAGS) $(OBJS) -o $(PROGRAM)

.SUFFIXES: .o .cpp .c

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(PROGRAM)
