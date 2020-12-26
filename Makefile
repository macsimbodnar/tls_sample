# Get the operating system
UNAME := $(shell uname)

# Set the compiler flags
CXXFLAGS += -Wall -Wno-deprecated -Werror -std=c++17
# Set the linker flags 
LDLIBS += -lssl -lcrypto

# In case we compile on macos (Darwin is the name) we needs to set the openssl dir
ifeq ($(UNAME), Darwin)
LDFLAGS += -L/usr/local/opt/openssl/lib
CXXFLAGS += -I/usr/local/opt/openssl/include
endif

all: server client

server: server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(DEPS) $(LDFLAGS) $(LDLIBS)

client: client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(DEPS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) *.o server client