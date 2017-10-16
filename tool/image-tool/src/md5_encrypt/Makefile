#CC = arm-linux-gcc
CC := gcc
CFLAG += -Os -I$(SDK_CONF_DIR)

TARGET := md5_encrypt

OBJ := snx_encrypt_decrypt.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAG) -o $@ $(OBJ)

%.o: %.c
	$(CC) -c $(CFLAG)  $<

clean:
	rm -rf $(OBJ) ./$(TARGET)
