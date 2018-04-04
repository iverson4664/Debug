# export LD_LIBRARY_PATH="$LD_LIBRARY_PATH":/usr/lib64)

out = out
# src = $(shell \
# 	  find . -type f -iregex '^.*\.cpp$$' | \
# 	  sed 's!^./!!g')
src = $(wildcard *.cpp) $(wildcard */*.cpp)
obj = $(patsubst %.cpp, $(out)/%.o, $(src))
# obj = $(patsubst %.cpp, $(out)/%.o, $(notdir $(src)))
asmobj = $(patsubst %.cpp, $(out)/%.s, $(src))
# asmobj = $(patsubst %.cpp, $(out)/%.s, $(notdir $(src)))
# obj = $(src:.cpp = .o)
bin = $(out)/main
so = $(out)/libmain.so
cflags = -Wall -g -O3 -std=gnu++14 -m64 -I./utils
lflags = -std=gnu++14 -m64 -lpthread -ldl
inc = -I
libs = -L

all: $(out) $(bin) $(so)

# $(obj): | $(out)
$(out):
	@mkdir -p $@

$(bin): $(obj)
	gcc -lstdc++ $(lflags) $^ -o $@ $(inc) $(libs)

$(so): $(obj)
	g++ -shared -o $@ $(obj)

$(out)/%.o: %.cpp
	@[ ! -d $(dir $@)  ] & mkdir -p $(dir $@)
	gcc $(cflags) -c -fPIC $< -o $@

$(out)/%.s: %.cpp
	gcc $(cflags) -S $< -o $@

asm: $(out) $(asmobj) $(bin)

.PHONEY:clean asm

clean:
	@rm -rfv $(out)


# gcc -lstdc++ -g -O3 -std=c++11 main.o -o main
