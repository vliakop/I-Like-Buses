MYSTATION_SRCS  = mystation.cpp structures/stats.cpp functions/my_functions.cpp
MYSTATION_OBJS  = $(MYSTATION_SRCS:.cpp=.o)
MANAGER_SRCS    = structures/LedgerRecord.cpp stations-manager.cpp
MANAGER_OBJS  = $(MANAGER_SRCS:.cpp=.o)
BUS_SRCS    = structures/stats.cpp functions/my_functions.cpp bus.cpp
BUS_OBJS  = $(BUS_SRCS:.cpp=.o)
COMPTROLLER_SRCS    = structures/stats.cpp comptroller.cpp
COMPTROLLER_OBJS  = $(COMPTROLLER_SRCS:.cpp=.o)
CC = g++
FLAG = -lpthread

all: mystation station-manager bus comptroller

mystation:  $(MYSTATION_OBJS)
	$(CC) $(MYSTATION_OBJS) $(FLAG) -o $@

station-manager:  $(MANAGER_OBJS)
	$(CC) $(MANAGER_OBJS) $(FLAG) -o $@

bus:  $(BUS_OBJS)
	$(CC) $(BUS_OBJS) $(FLAG) -o $@

comptroller:  $(COMPTROLLER_OBJS)
	$(CC) $(COMPTROLLER_OBJS) $(FLAG) -o $@

.cpp.o:
	$(CC) -c $< -o $@

clean:
	rm -f *.o
