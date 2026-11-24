CC      = gcc
CFLAGS  = -std=gnu11 -Wall -Wextra -g -Iinclude

EXE     = .exe
RAYLIB  = -lraylib -lopengl32 -lgdi32 -lwinmm

# Raylib em pasta própria: RAYLIB_PATH=<pasta> (raylib.h e libraylib.a juntos)
# ou separados: RAYLIB_INC=<pasta do raylib.h> RAYLIB_LIB=<pasta do libraylib.a>
ifdef RAYLIB_PATH
  RAYLIB_INC ?= $(RAYLIB_PATH)
  RAYLIB_LIB ?= $(RAYLIB_PATH)
endif
ifdef RAYLIB_INC
  CFLAGS += -I$(RAYLIB_INC)
endif
ifdef RAYLIB_LIB
  RAYLIB := -L$(RAYLIB_LIB) $(RAYLIB)
endif

LOGICA  = src/entidades.c src/equipe.c src/mapa.c src/regras.c src/ressurreicao.c \
          src/batalha.c src/arquivos.c src/pontuacao.c src/lore.c
JOGO    = src/main.c src/interface.c src/assets.c $(LOGICA)

all: cursedsouls$(EXE)

cursedsouls$(EXE): $(JOGO) $(wildcard include/*.h)
	$(CC) $(CFLAGS) $(JOGO) -o $@ $(RAYLIB) -lm

teste: tests/teste_logico.c $(LOGICA) $(wildcard include/*.h)
	$(CC) $(CFLAGS) tests/teste_logico.c $(LOGICA) -o teste_logico$(EXE)
	./teste_logico$(EXE)

run: cursedsouls$(EXE)
	./cursedsouls$(EXE)

clean:
	rm -f cursedsouls cursedsouls.exe teste_logico teste_logico.exe

.PHONY: all teste run clean
