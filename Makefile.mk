# Makefile pour compilateur-pascal

# — outils et flags —
CXX      := g++
CXXFLAGS := -std=gnu++23 -Wall -Iutils -Itokeniser -Iparser/include
FLEX     := flex
AR       := ar
RM       := rm -rf

# — répertoires de sortie —
LIB_DIR := lib
BIN_DIR := bin

# — sources/utilitaires —
UTILS_SRCS := $(wildcard utils/*.cpp)
UTILS_OBJS := $(UTILS_SRCS:.cpp=.o)
UTILS_LIB  := $(LIB_DIR)/libutils.a

TOKENISER_L    := tokeniser/tokeniser.l
TOKENISER_CPP  := tokeniser/tokeniser.cpp
TOKENISER_HDR  := tokeniser/tokeniser.h
TOKENISER_OBJS := $(TOKENISER_CPP:.cpp=.o)
TOKENISER_LIB  := $(LIB_DIR)/libtokeniser.a

PARSER_SRCS := $(wildcard parser/src/*.cpp)
PARSER_OBJS := $(patsubst parser/src/%.cpp,parser/%.o,$(PARSER_SRCS))
PARSER_LIB  := $(LIB_DIR)/libparser.a

PAS_EXEC := $(BIN_DIR)/pascal

# — targets par défaut —
.PHONY: all clean
all: $(PAS_EXEC)

# — création des dossiers lib/ et bin/ —
.PHONY: directories
directories:
	@mkdir -p $(LIB_DIR) $(BIN_DIR)

# — utils library —
$(UTILS_LIB): directories $(UTILS_OBJS)
	$(AR) rcs $@ $^

utils/%.o: utils/%.cpp include/utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# — tokeniser : flex + lib —
# génère tokeniser.cpp et tokeniser.h
$(TOKENISER_CPP) $(TOKENISER_HDR): tokeniser/tokeniser.l
	$(FLEX) -d -o $(TOKENISER_CPP) $<
	@mv lex.yy.h $(TOKENISER_HDR)

$(TOKENISER_LIB): directories $(TOKENISER_OBJS)
	$(AR) rcs $@ $^

tokeniser/%.o: tokeniser/%.cpp include/tokeniser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# — ceriparser library —
$(PARSER_LIB): directories $(PARSER_OBJS)
	$(AR) rcs $@ $^

parser/%.o: parser/src/%.cpp $(wildcard parser/include/*.h)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# — exécutable pascal —
$(PAS_EXEC): directories $(UTILS_LIB) $(TOKENISER_LIB) $(PARSER_LIB) compiler/src/main.cpp
	$(CXX) $(CXXFLAGS) $< \
	  -L$(LIB_DIR) -lparser -lutils -ltokeniser \
	  -o $@

# — nettoyage —
clean:
	$(RM) $(LIB_DIR) $(BIN_DIR)
	$(RM) utils/*.o tokeniser/*.o parser/*.o
	$(RM) $(TOKENISER_CPP) lex.yy.h

