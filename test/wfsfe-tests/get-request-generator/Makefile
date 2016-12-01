CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		formatter.o

LIBS = -lxqilla -lxerces-c -lboost_system-mt

TARGET =	formatter

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
