TARGET = walgo
SRC = walgo.c
CFLAGS = -O2 -Wall 

all: $(TARGET)

$(TARGET): $(SRC)
	gcc $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
