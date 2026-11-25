# Casos de teste manuais

Manuais: executar `./cursedsouls`.

# Resultados dos testes manuais

Executados por [Glêdson] em [24/11], no Windows 11.

| ID | Resultado obtido | Passou? |
|---|---|---|
| CT-12 | O espaço vira _, os símbolos são ignorados e o vazio mostra erro | Sim | 🆗
| CT-13 | Aparece "Nenhum jogo salvo para esse nome" e você fica na mesma tela | Sim | 🆗
| CT-14 | As opções de recomeçar e sair funcionaram corretamente | Sim | 🆗
| CT-17 | A lore, a animação de encontro e a fala exclusiva do chefe funcionaram conforme esperado | Sim | 🆗
| CT-19 | O jogo abre nos dois casos, com a forma simples no segundo | Sim | 🆗


| ID | Descrição | Entrada | Resultado esperado | Tipo |
|---|---|---|---|---|
| CT-12 | Nome inválido na interface | Digitar espaço/símbolos; ENTER com campo vazio | Espaço vira `_`; símbolos ignorados; vazio mostra erro | Manual |

| CT-13 | Continuar com nome sem save | Menu > Continuar > digitar um nome que nunca salvou | Aviso "Nenhum jogo salvo para esse nome" | Manual |

| CT-14 | Reiniciar/sair | Fim de jogo > Recomeçar / Sair | Volta ao nome do jogador / fecha o programa | Manual |

| CT-17 | Lore e encontro na interface | Novo Jogo (ENTER avança, ESC pula); andar na grama até um encontro; Pausa > Ver a história | Lore aparece só em jogo novo; animação do encontro toca e ENTER a pula; chefe tem fala própria | Manual |

| CT-19 | Sprites e reserva | Abrir o jogo com `assets/sprites/` completa; depois renomear um PNG (ex.: `especie_0.png`) e abrir de novo | Com os arquivos, aparecem os sprites; sem um deles, aquele item volta às formas simples e o jogo não trava | Manual |