# Projeto Reator

## O que é?

Este é um programa para Arduino, feito para controlar sensores e atuadores em um projeto que visa construir
um reator automatizado e controlado de maneira 100% digital. 

## Como usar?

Todo o controle é feito através do monitor serial. Basta carregar o programa na placa Arduino, montar os circuitos e usar. 

### Bombas

Para acionar as bombas é necessário digitar a letra 'B' seguido do número da bomba que se quer controlar (1 ou 2) e um valor de 0 à 100 que representa o percentual da vazão desejada.

Exemplo 1:

    B1100 

Liga a bomba número 1 em 100%

Exemplo 2:

    B20

Desativa a bomba 2.

### Ajuste de Temperatura

O ajuste da temperatura só pode ser feito com o conjunto resitência e sensor de temperatura devidamente ligados.
O controle de temperatura é feito através de liga/desliga onde a resistência aquece até atingir o valor de referência e desliga, religando assim que o valor sair da faixa de valores desejada. 

Para definir um valor de temperatura basta digitar:

    T60

Usando o comando acima o reator vai aquecer o fluído até a temperatura de 60ºC.

### Agitador 

Entre os atuadores disponíveis para controle está um agitador, o limite de operação do agitador vai de 0rpm até 156rpm. Para acioná-lo basta digitar o seguinte comando. 

    A100

O comando anterior aciona o agitador com a velocidade de 100 rotações por minuto. 

### Valvula 

O reator possuí uma válvula que controla a saída do reator. Esta valvula é controlada por um servo motor e abre dependendo do valor em percentual segundo o seguinte comando. 

    V100 

Abre a valvula completamente.

    V0

Fecha a valvula.

## Painel de controle 

O código do programa possuí uma área que deve ser modificada pelo usuário a medida que ele escolhe os sensores e atuadores que deseja usar. 

O painel de controle pode ser visualizado abaixo:

    //Define as chaves de controle
    boolean CH_LIGA_BOMBA1 = 0;
    boolean CH_LIGA_BOMBA2 = 0;
    boolean CH_LIGA_VALVULA = 0;
    boolean CH_LIBERA_SENSOR_TEMPERATURA = 0;
    boolean CH_RESISTENCIA = 0;
    boolean CH_HABILITA_AGITADOR = 0;

Para ativar ou desativar uma chave de controle basta definir ela como 0 (desativar) ou 1 (ativar).
