# Makefile – Version FINALE pour le projet Ordonnanceur (Décembre 2025)

CC = gcc

# Détection automatique des flags GTK+ 3.0
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

# Flags de compilation
CFLAGS = -Wall -Wextra -g -std=c99 -fPIC -pthread $(GTK_CFLAGS)

# Flags de linkage
LDFLAGS = $(GTK_LIBS) -ldl -lpthread -lm   # -ldl pour dlopen, -lm pour les constantes comme G_PI

# Répertoires
SRCDIR  = src
INCLDIR = includes
POLDIR  = policies
CONFIGDIR = config

# Fichiers sources principaux (gui.c est ton fichier avec l'IHM Gantt)
SRC_FILES = main.c parser.c simulation.c utils.c gui.c
SRC       = $(addprefix $(SRCDIR)/, $(SRC_FILES))

# Bibliothèques dynamiques (.so) des politiques
POL_SOURCES = $(wildcard $(POLDIR)/*.c)
POL_SO      = $(POL_SOURCES:.c=.so)

# Objets et exécutable
OBJECTS    = $(SRC:.c=.o)
EXECUTABLE = ordonnanceur

.PHONY: all clean re run dynamic_libs install

# Cible par défaut
all: $(EXECUTABLE) $(POL_SO)

# Liaison finale de l'exécutable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -rdynamic $^ $(LDFLAGS) -o $@

# Compilation des fichiers .c → .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLDIR) -c $< -o $@

# ----------------------------------------------------
# Création des bibliothèques partagées (.so) pour les politiques
# ----------------------------------------------------
$(POLDIR)/%.so: $(POLDIR)/%.c
	@echo "Construction de la politique $@"
	$(CC) $(CFLAGS) -I$(INCLDIR) -c $< -o $(POLDIR)/$*.o
	$(CC) -shared $(POLDIR)/$*.o -o $@
	rm -f $(POLDIR)/$*.o

dynamic_libs: $(POL_SO)

# Nettoyage complet
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	rm -f $(POLDIR)/*.o $(POLDIR)/*.so

# Recompiler tout proprement
re: clean all

# Règle pratique pour exécuter directement après compilation
run: all
	@echo "Lancement avec un fichier de test (exemple : processes.txt)"
	./$(EXECUTABLE) $(CONFIGDIR)/processes.txt   # change le fichier si tu veux

# Installation (root ou locale)
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