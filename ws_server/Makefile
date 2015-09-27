CPPFLAGS += -L../../lib/boost/lib
CPPFLAGS += -I../../lib/boost/include
CPPFLAGS += -L../../lib/libpq/lib 
CPPFLAGS += -I../../lib/libpq/include

CPPFLAGS += -L../../lib/openssl/lib
CPPFLAGS += -I../../lib/openssl/include
CPPFLAGS += -I../../lib/websocketpp

CPPFLAGS += -lpq 
CPPFLAGS += -lboost_system
CPPFLAGS += -lboost_thread
CPPFLAGS += -lssl
CPPFLAGS += -lcrypt
CPPFLAGS += -lcrypto
CPPFLAGS += -lpthread
CPPFLAGS += -std=c++14
CPPFLAGS += -s
CPPFLAGS += -ldl
CPPFLAGS += -lrt

ws_server: main.o ws_server.o
	g++ main.o ws_server.o $(CPPFLAGS) -o ws_server

main.o: main.cpp
	g++ -c main.cpp $(CPPFLAGS)

ws_server.o: ws_server.cpp
	g++ -c ws_server.cpp $(CPPFLAGS)
clean:
	rm -rf *.o ws_server
