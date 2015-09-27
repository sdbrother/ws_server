CPPFLAGS += -L../../lib/boost/lib
CPPFLAGS += -I../../lib/boost
#CPPFLAGS += -L../../lib/libpq/lib 
#CPPFLAGS += -I../../lib/libpq/include
#CPPFLAGS += -lpq 


CPPFLAGS += -pthread
CPPFLAGS += -lpthread
CPPFLAGS += -lrt
CPPFLAGS += -lboost_system
CPPFLAGS += -lboost_thread


CPPFLAGS += -std=c++14
CPPFLAGS += -s
#CPPFLAGS += -ldl
#CPPFLAGS += -lrt
#CPPFLAGS += "/usr/lib/x86_64-linux-gnu/librt.a"
#CPPFLAGS += -lm
#CPPFLAGS += -lc 


ws_data: main.o
	g++ main.o $(CPPFLAGS) -o ws_data_write

main.o: main.cpp
	g++ -c main.cpp $(CPPFLAGS)
clean:
	rm -rf *.o ws_server
