# Cursed Souls — Ecos Amaldiçoados

Jogo de exploração, batalha por turnos e coleta de criaturas em **C + Raylib**.
Você não captura criaturas: primeiro as **derrota**, depois tenta **ressuscitar** a essência delas.

> Derrote. Ressuscite. Conquiste.

## Compilar e executar (Windows)

1. Instale a Raylib pelo instalador oficial (github.com/raysan5/raylib/releases, arquivo `raylib_installer_*.exe`).
   Ele instala em `C:\raylib` junto com o compilador GCC (w64devkit).
2. Abra `C:\raylib\w64devkit\w64devkit.exe`, entre na pasta do projeto e rode:
   ```
   make RAYLIB_PATH=C:/raylib/raylib/src
   ./cursedsouls.exe
   ```
   Se `raylib.h` e `libraylib.a` estiverem em pastas diferentes:
   `make RAYLIB_INC=<pasta do raylib.h> RAYLIB_LIB=<pasta do libraylib.a>`.
3. Testes da lógica (não precisam da Raylib): `make teste`.

> O professor autorizou o uso da Raylib e dispensou a execução em Linux.

Créditos, origem da arte e uso de ferramentas de IA: veja `CREDITOS.md`.

## Controles

| Tela | Teclas |
|---|---|
| Menus | ↑/↓ (ou W/S), ENTER |
| Exploração | setas ou WASD, **E** equipe, **ESC** pausa |
| Batalha | ↑/↓ + ENTER; **ESC** volta na troca |
| Ressurreição | **ENTER** tenta, **C** usa Correntes da Maldição (+20%), **ESC** desiste |
| Equipe | ENTER define a ativa; marque uma reserva e escolha o slot para trocar |

## O que está implementado

- Menu inicial (Novo Jogo, Continuar, Instruções, Ranking, Sair), nome do jogador e escolha da 1ª criatura.
  "Continuar" pede o nome do jogador e carrega só o save dele (`saves/<nome>.sav`).
- **História de abertura** (texto em `src/lore.c`) ao iniciar um jogo novo; dá para rever em Pausa > Ver a história.
- **Animação de encontro** antes de cada batalha (mais longa, com fala, contra o Guardião).
- **Sprites** em `assets/sprites/` (`especie_0.png` a `especie_6.png`, `jogador.png`, `jogador_batalha.png`,
  `guardiao_mapa.png`, `guardiao_batalha.png`). Se um arquivo faltar, o jogo desenha aquele item com formas simples.
  Tiles do mapa (`parede_floresta/ruinas/covil.png`, `grama_floresta/ruinas.png`, `arvore.png`) e fundos
  (`fundo_menu.png`, `fundo_batalha_floresta/ruinas/covil.png`, 400x300, ampliados 2x) também são opcionais:
  sem eles, o jogo desenha com formas simples.
  O jogador anda com `jogador_baixo.png`, `jogador_cima.png`, `jogador_esquerda.png` e `jogador_direita.png`
  (32 de altura; os quadros ficam lado a lado, ex.: 64x32 = 2 quadros). A fogueira e os portais (`fogueira.png`,
  `portal_proximo.png`, `portal_anterior.png`) são tiras de 3 quadros de 32x32, e `titulo.png` é o logo do menu.
- 3 regiões em visão superior: Floresta Corrompida, Ruínas Amaldiçoadas e Covil do Guardião (chefe).
  Colisão, grama com encontros aleatórios, fogueiras (curam a equipe), transição entre áreas.
- 7 espécies, 4 afinidades (Fogo > Gelo > Caos > Sombria > Fogo), 5 raridades, 2 habilidades cada.
- **Chefe invocador:** o Guardião Corrompido é um personagem (não uma criatura) com equipe própria de 3 criaturas
  (Espectro do Caos Nv 10, Corvo Ruinoso Nv 11, Wendigo Gélido Nv 12). Ele envia uma por vez; a vitória só vem quando
  todas caem. Não dá para fugir, e as criaturas dele não podem ser ressuscitadas. A equipe fica na tabela
  `EQUIPE_CHEFE` em `src/entidades.c`.
- Batalha por turnos: atacar, habilidade (gasta energia; efeitos de reduzir defesa e drenar vida), trocar, poção, fugir.
  Ordem por velocidade; inimigos seguem regras simples.
- **Ressurreição probabilística** (Comum 75%, Incomum 55%, Rara 35%, Épica 20%, Lendária 10%)
  + habilidade **Correntes da Maldição** (+20%, cargas limitadas).
- Equipe ativa de até 3 criaturas + armazenamento dinâmico (`realloc`).
- XP, níveis (até 20) e pontuação; vitória ao derrotar o Guardião, derrota se a equipe cair.
- **Save completo** (`saves/<nome>.sav`) e **ranking** (`saves/ranking.txt`, top 10, com a melhor pontuação de cada nome) — são arquivos distintos.
- Validação de entradas, checagem de limites em vetores/matrizes e leitura defensiva do save.

Itens opcionais do documento (sons, animações de habilidade, mais regiões) ficaram fora. Criaturas, personagens,
fundos e quase todos os tiles usam as imagens de `assets/sprites/`; só o chão de cada região é desenhado em código.
O item "Opções" do menu virou **Instruções** + **Ranking**.

## Estrutura

```
src/        main.c (laço e telas) · interface.c (desenho Raylib) · assets.c (carrega os sprites) · mapa.c · entidades.c · equipe.c
            batalha.c · regras.c · ressurreicao.c · arquivos.c · pontuacao.c · lore.c
include/    um .h por módulo
tests/      teste_logico.c (14 grupos de teste, 232 verificações)
docs/       casos_de_teste.md · relatório.pdf
assets/     sprites/ (imagens do jogo)
saves/      <nome>.sav (um por jogador) e ranking.txt (gerados em execução)
CREDITOS.md origem da arte e uso de ferramentas
relatorio.pdf  relatório da disciplina
```

Fluxo: `main.c` só orquestra; toda regra de jogo está nos módulos sem Raylib, por isso é testável isoladamente
e fácil de dividir entre integrantes (um módulo por pessoa).

## Balanceamento inicial

Todos os números (chances, crescimento de 12%/nível, dano `poder × (10+ATQ) / (10+DEF)`, 15% de encontro na grama)
são valores de partida para ajuste. Estão concentrados em `entidades.c` (espécies), `ressurreicao.c`, `regras.c`
e `main.c` (`ENCONTRO_PCT`).