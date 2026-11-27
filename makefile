CC = gcc
CFLAGS = -Wall -g
SRC = fichiers/year_data.c fichiers/crash_data.c fichiers/age_data.c fichiers/main.c
TARGET = crashTech

CSV_DIR = dossier_csv/

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) CRASH_DATA $(CSV_DIR)/nom_du_fichier.csv

clean:
	rm -f $(TARGET)
