# Casos de teste

Automáticos: `make teste` (arquivo `tests/teste_logico.c`). Manuais: executar `./cursedsouls`.

| ID | Descrição | Entrada | Resultado esperado | Tipo |
|---|---|---|---|---|
| CT-01 | Dano e afinidade | Fogo atacando Gelo; defesa enorme | x1,5 de dano; dano nunca < 1 | Auto |
| CT-02 | Chances de ressurreição | Cada raridade, com e sem Correntes | 75/55/35/20/10%; teto 95%; raridade inválida = 1% | Auto |
| CT-03 | Tentativa de ressurreição | Sorteios 74, 75, 100; sem cargas; criatura nula | 74 vence, 75 falha; 100, sem cargas e criatura nula = erro | Auto |
| CT-04 | Limite da equipe | Adicionar 4ª e +20 criaturas | Equipe fica em 3; resto vai ao armazenamento (realloc) | Auto |
| CT-05 | Troca inválida | Índice -1, 7, ativa, derrotada | Todas rejeitadas; troca válida aceita | Auto |
| CT-06 | Salvar/carregar | Estado com 3 ativas + 3 reservas | Todos os campos idênticos após carregar | Auto |
| CT-07 | Save corrompido | Inexistente, cabeçalho ruim, espécie 99, região 9, truncado | Erro sem alterar o estado atual | Auto |
| CT-08 | Mapa | Posições -1, 9999, borda, spawn | Colisão correta; fora dos limites = parede | Auto |
| CT-09 | XP e nível | XP exato, enorme, negativo | Sobe nível; trava em 20; negativo ignorado | Auto |
| CT-10 | Batalha | Habilidade sem energia, poção com HP cheio, fugir do chefe, derrota | Ação inválida não gasta turno; fim correto | Auto |
| CT-11 | Nome e ranking | "", "com espaco", nome longo; 13 jogadores; mesmo nome com pontuação menor e maior; pontuação abaixo do top 10 | Nomes inválidos rejeitados; top 10 ordenado; cada nome aparece uma vez com a melhor pontuação | Auto |
| CT-12 | Nome inválido na interface | Digitar espaço/símbolos; ENTER com campo vazio | Espaço vira `_`; símbolos ignorados; vazio mostra erro | Manual |
| CT-13 | Continuar com nome sem save | Menu > Continuar > digitar um nome que nunca salvou | Aviso "Nenhum jogo salvo para esse nome" | Manual |
| CT-14 | Reiniciar/sair | Fim de jogo > Recomeçar / Sair | Volta ao nome do jogador / fecha o programa | Manual |
| CT-15 | Guardião invocador | Chefe com 3 criaturas; derrubar uma por vez; tentar fugir | Envia a próxima a cada queda; vitória só após a 3ª; fuga bloqueada; bônus de pontos | Auto |
| CT-16 | Lore | Todos os painéis; índices -1 e acima do limite | Painéis não vazios; índice inválido devolve texto vazio | Auto |
| CT-17 | Lore e encontro na interface | Novo Jogo (ENTER avança, ESC pula); andar na grama até um encontro; Pausa > Ver a história | Lore aparece só em jogo novo; animação do encontro toca e ENTER a pula; chefe tem fala própria | Manual |
| CT-18 | Save por jogador | Salvar "TesteAna" e "TesteBeto"; nomes inválidos (vazio, `../x`, espaço) | Cada nome tem o seu arquivo; um não sobrescreve o outro; nome inválido não gera caminho | Auto |
| CT-19 | Sprites e reserva | Abrir o jogo com `assets/sprites/` completa; depois renomear um PNG (ex.: `especie_0.png`) e abrir de novo | Com os arquivos, aparecem os sprites; sem um deles, aquele item volta às formas simples e o jogo não trava | Manual |