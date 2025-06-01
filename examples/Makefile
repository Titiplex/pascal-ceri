# examples/Makefile généré partiellement automatiquement

# chemin vers le compilateur Pascal produit
PASCAL := ../bin/pascal.exe

# répertoires pour asm et binaires d’exemples
ASM_DIR := build
.PRECIOUS: $(ASM_DIR)/%.s
BIN_DIR := bin

# fichiers .p et cibles associées
PASCAL_SRCS := $(wildcard *.p)
ASM_SRCS    := $(patsubst %.p,$(ASM_DIR)/%.s,$(PASCAL_SRCS))
EXES        := $(patsubst %.p,$(BIN_DIR)/%,$(PASCAL_SRCS))

.PHONY: all clean
all: $(EXES)

# créer dossier asm si besoin
$(ASM_DIR):
	mkdir -p $(ASM_DIR)

# .p → .s via pascal avec -o plutôt que redirection
$(ASM_DIR)/%.s: %.p | $(ASM_DIR)
	echo "Compiling Pascal → ASM: $< to $@"
	$(PASCAL) $< -o $@
	@echo "Testing ..."
	@test -s $@ || { echo "ERR: $@ est vide"; }

# créer dossier bin si besoin
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# .s → exécutable natif
$(BIN_DIR)/%: $(ASM_DIR)/%.s | $(BIN_DIR)
	@echo "Assembling ASM → EXE: $<"
	gcc -ggdb -no-pie -fno-pie -o $@ $<

clean:
	rm -rf $(ASM_DIR) $(BIN_DIR)
