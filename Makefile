all: build/matching-service
.PHONY: all clean

LINK_CMD = $(CXX)
COMPILE_CMD = $(CXX) -c -std=c++11 -Wall -Wpedantic -I src

OBJECTS = \
	build/main.o \
	build/matching-handler.o \
	build/rpc.o

build/matching-service: $(OBJECTS)
	$(LINK_CMD) -o $@ $^

build/%.o: src/%.cpp | build
	$(COMPILE_CMD) -o $@ $<

build:
	mkdir -p build
clean:
	rm -rf build
