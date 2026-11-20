CC = gcc
CFLAGS = -Wall -g
SRC = fichiers/year_data.c fichiers/crash_data.c fichiers/age_data.c fichiers/main.c
TARGET = crashTech

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
