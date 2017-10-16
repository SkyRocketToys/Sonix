#

CC := gcc
CFLAG += -Wall -Os -I$(SDK_CONF_DIR)

TARGET := phy_sf

OBJ += package_image.o
OBJ += pad_to_tail.o
OBJ += insert.o
OBJ += md5.o
OBJ += checksum.o
OBJ += check.o
OBJ += main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAG) -o $@ $(OBJ)

%.o: %.c
	$(CC) -c $(CFLAG)  $<

clean:
	$(RM) $(OBJ) $(TARGET)
