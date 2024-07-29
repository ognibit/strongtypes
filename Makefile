#CFLAGS=-Wall -Wextra -pedantic -g -DTYPE_DECIMAL_DIGITS=4 -DTYPE_DECIMAL_POWER=10000
#For C++ change to -std=c++20
CFLAGS=-Wall -Wextra -pedantic -g -std=c99 -Og -fsanitize=undefined
LFLAGS=-lm -lubsan
TARGET=tests


%.o : %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET) : main.o strongtypes.o
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	$(RM) $(TARGET) *.o

release: CFLAGS=-Wall -Wextra -pedantic -g -std=c99 -O2 -DNDEBUG
release: LFLAGS=-lm
release: clean
release: $(TARGET)

lint:
	cppcheck --enable=warning,style,performance,portability,unusedFunction .

