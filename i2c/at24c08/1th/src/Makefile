#source file
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
     
all : $(TARGET)
     
objs : $(OBJS)
     
rebuild: veryclean everything
     
clean :
	rm -fr *.so
	rm -fr *.o
	rm -fr $(TARGET)
     
veryclean : clean
	rm -fr $(TARGET)
     
$(TARGET) : $(OBJS)
	echo $(CC)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
