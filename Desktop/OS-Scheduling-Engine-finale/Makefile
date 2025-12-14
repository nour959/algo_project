
CC = gcc

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

CFLAGS = -Wall -Wextra -g -std=c99 -fPIC -pthread $(GTK_CFLAGS)

LDFLAGS = $(GTK_LIBS) -ldl -lpthread -lm   

SRCDIR  = src
INCLDIR = includes
POLDIR  = policies
CONFIGDIR = config

SRC_FILES = main.c parser.c simulation.c utils.c gui.c
SRC       = $(addprefix $(SRCDIR)/, $(SRC_FILES))

POL_SOURCES = $(wildcard $(POLDIR)/*.c)
POL_SO      = $(POL_SOURCES:.c=.so)

OBJECTS    = $(SRC:.c=.o)
EXECUTABLE = ordonnanceur

.PHONY: all clean re run dynamic_libs install

all: $(EXECUTABLE) $(POL_SO)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -rdynamic $^ $(LDFLAGS) -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLDIR) -c $< -o $@

$(POLDIR)/%.so: $(POLDIR)/%.c
	@echo "Construction de la politique $@"
	$(CC) $(CFLAGS) -I$(INCLDIR) -c $< -o $(POLDIR)/$*.o
	$(CC) -shared $(POLDIR)/$*.o -o $@
	rm -f $(POLDIR)/$*.o

dynamic_libs: $(POL_SO)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	rm -f $(POLDIR)/*.o $(POLDIR)/*.so

re: clean all

run: all
	@echo "Lancement avec un fichier de test (exemple : processes.txt)"
	./$(EXECUTABLE) $(CONFIGDIR)/processes.txt   # change le fichier si tu veux

INSTALL_DIR = /usr/local/bin
install: $(EXECUTABLE) $(POL_SO)
	@echo "Installation dans $(INSTALL_DIR)..."
	@if [ -w $(INSTALL_DIR) ]; then \
		cp $(EXECUTABLE) $(INSTALL_DIR)/; \
		mkdir -p $(INSTALL_DIR)/policies; \
		cp $(POL_SO) $(INSTALL_DIR)/policies/; \
		echo "Installation réussie dans $(INSTALL_DIR)"; \
	else \
		echo "Pas de droits root → copie locale dans ./bin"; \
		mkdir -p bin/policies; \
		cp $(EXECUTABLE) bin/; \
		cp $(POL_SO) bin/policies/; \
		echo "Exécutable disponible avec ./bin/ordonnanceur"; \
	fi