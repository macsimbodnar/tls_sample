CXXFLAGS = -Wall -Wno-deprecated -Werror -std=c++17
LDLIBS = -lssl -lcrypto

all: server client

server: server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(DEPS) $(LDFLAGS) $(LDLIBS)

client: client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(DEPS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) *.o server client