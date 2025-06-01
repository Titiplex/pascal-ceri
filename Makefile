# Makefile racine généré partiellement automatiquement

# — outils et flags —
CXX         ?= g++
OPTFLAGS    ?=
CXXFLAGS    ?= -ggdb -std=gnu++23 -Wall $(OPTFLAGS)

# includes
INCLUDE_DIR          := include
PARSER_INCLUDE_DIR   := $(INCLUDE_DIR)/parser
CXXFLAGS   += -I$(INCLUDE_DIR) \
              -I$(PARSER_INCLUDE_DIR) \
              -I$(INCLUDE_DIR)/tokeniser \
              -I$(INCLUDE_DIR)/utils

FLEX        ?= flex
AR          ?= ar
RM          := rm -rf

# — répertoires de sortie —
LIB_DIR     := lib
BIN_DIR     := bin

# — sources/utilitaires —
UTILS_SRCS  := $(wildcard utils/*.cpp)
UTILS_OBJS  := $(UTILS_SRCS:.cpp=.o)
UTILS_LIB   := $(LIB_DIR)/libutils.a

# — tokeniser : flex + lib —
TOKENISER_L     := tokeniser/tokeniser.l
TOKENISER_CPP   := tokeniser/tokeniser.cpp
TOKENISER_HDR   := $(INCLUDE_DIR)/tokeniser.h
TOKENISER_OBJS  := $(TOKENISER_CPP:.cpp=.o)
TOKENISER_LIB   := $(LIB_DIR)/libtokeniser.a

# — parser : .cpp + umbrella header + lib —
PARSER_SRCS     := $(wildcard parser/src/*.cpp)
PARSER_OBJS     := $(patsubst parser/src/%.cpp,parser/%.o,$(PARSER_SRCS))
PARSER_LIB      := $(LIB_DIR)/libparser.a
INTERNAL_HDRS   := $(wildcard parser/include/*.h)
UMB_HEADER      := $(INCLUDE_DIR)/parser.h

# — exécutable pascal —
PAS_EXEC        := $(BIN_DIR)/pascal

# — cible par défaut —
.PHONY: all clean
all: $(PAS_EXEC)

# — création des dossiers —
.PHONY: directories
directories:
	@mkdir -p $(LIB_DIR) $(BIN_DIR) $(PARSER_INCLUDE_DIR)

# — génération du header ombrelle parser.h —
#    (sur modification parser/include/)
$(UMB_HEADER): $(INTERNAL_HDRS) | directories
	@echo "// Auto-généré par Make — ne pas modifier" > $@
	@echo "#pragma once"               >> $@
	@cp parser/include/*.h $(PARSER_INCLUDE_DIR)
	@for h in $(PARSER_INCLUDE_DIR)/*.h; do \
	  echo "#include \"parser/$$(basename $$h)\"" >> $@; \
	done

# — utils library —
$(UTILS_LIB): $(UTILS_OBJS) | directories
	$(AR) rcs $@ $^

utils/%.o: utils/%.cpp $(INCLUDE_DIR)/utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# — tokeniser (Flex) + library —
$(TOKENISER_CPP) $(TOKENISER_HDR):
	$(FLEX) -d -o $(TOKENISER_CPP) $(TOKENISER_L)

$(TOKENISER_LIB): $(TOKENISER_CPP) $(TOKENISER_OBJS) | directories
	$(AR) rcs $@ $^

tokeniser/%.o: tokeniser/%.cpp $(TOKENISER_HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# — parser library (dépend du header ombrelle) —
$(PARSER_LIB): $(UMB_HEADER) $(PARSER_OBJS) | directories
	$(AR) rcs $@ $^

parser/%.o: parser/src/%.cpp $(UMB_HEADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# — exécutable pascal (lié à nos libs) —
$(PAS_EXEC): $(UTILS_LIB) $(TOKENISER_LIB) $(PARSER_LIB) compiler/src/main.cpp | directories
	$(CXX) $(CXXFLAGS) $< \
	  -L$(LIB_DIR) -lutils -ltokeniser -lparser \
	  -o $@

# — nettoyage —
clean:
	$(RM) $(LIB_DIR) $(BIN_DIR)
	$(RM) utils/*.o tokeniser/*.o parser/*.o
	$(RM) include/parser/*.h
	$(RM) $(UMB_HEADER)
	$(RM) $(TOKENISER_CPP)
