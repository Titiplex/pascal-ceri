# Makefile pour examples/

# compilateur
PASCAL := ../bin/pascal

# Répertoires de génération
ASM_DIR := build
BIN_DIR := bin

# Liste des fichiers sources .p
PASCAL_SRCS := $(wildcard *.p)
# Correspondance .p → .s
ASM_SRCS     := $(patsubst %.p,$(ASM_DIR)/%.s,$(PASCAL_SRCS))
# Correspondance .p → executable (sans extension)
EXES         := $(patsubst %.p,$(BIN_DIR)/%,$(PASCAL_SRCS))

.PHONY: all clean
all: $(EXES)

# 1) Génère le dossier ASM_DIR si nécessaire
$(ASM_DIR):
	mkdir -p $(ASM_DIR)

# 2) Compile .p → .s (order-only dépendance sur le dossier)
$(ASM_DIR)/%.s: %.p | $(ASM_DIR)
	@echo "Compiling Pascal → ASM: $<"
	$(PASCAL) < $< > $@

# 3) Génère le dossier BIN_DIR si nécessaire
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# 4) Assemble .s → exécutable
$(BIN_DIR)/%: $(ASM_DIR)/%.s | $(BIN_DIR)
	@echo "Assembling ASM → EXE: $<"
	gcc -o $@ $<

# 5) Nettoyage
clean:
	rm -rf $(ASM_DIR) $(BIN_DIR)
