# Projeto: Varal Inteligente com Sensor de Chuva

Este projeto consiste em um **varal automatizado** que utiliza um sensor de chuva para proteger as roupas, recolhendo-as automaticamente quando a chuva é detectada. O sistema utiliza a placa **LM393**, um motor de passo controlado pelos drivers **A4988** ou **DRV8825**, e um display OLED **SSD1306** para exibir informações em tempo real. O projeto foi desenvolvido para rodar no microcontrolador **Raspberry Pi Pico**.

## Componentes Utilizados
- **Microcontrolador**: Raspberry Pi Pico.
- **Motor de passo**: A4988 ou DRV8825.
- **Sensor de chuva**: LM393.
- **Display OLED**: SSD1306 (128x64) via comunicação I2C.
- **LEDs**: Indicadores visuais de status (vermelho, verde e azul).
- **Botão**: Simulação de ativação manual.
- **Placa de controle de motor de passo**: para o controle do varal.

## Funcionalidades
- **Detecção de chuva**: Quando o sensor detecta a presença de água (chuva), o motor de passo é ativado para recolher o varal.
- **Monitoramento de umidade**: A umidade do sensor de chuva é exibida no display OLED.
- **Exibição no display**: Mensagens sobre o status atual do sistema, como "Chuva detectada", "Ativando", "Desativando", e a porcentagem de umidade atual.
- **Controle do motor de passo**: O motor gira em direção horária ou anti-horária para movimentar o varal, com limite de passos para evitar desgaste.

## Estrutura do Código
- **Configuração dos pinos**: Definição dos pinos para os LEDs, motor, sensor e botão.
- **Estados do sistema**: Máquina de estados para gerenciar os modos "Chuva" e "Sem Chuva".
- **Leitura do sensor**: Utiliza o ADC do Pico para ler o valor analógico do sensor de chuva.
- **Exibição de mensagens**: Utiliza o display OLED SSD1306 via I2C para informar o status e as leituras do sensor.
- **Controle do motor de passo**: Geração de pulsos para movimentar o motor de acordo com as leituras do sensor.
  
## Como Funciona
1. O sistema inicia no estado **STAND BY**.
2. Ao detectar chuva, o sistema exibe no display "Chuva detectada" e aciona o motor de passo para recolher o varal.
3. Se não houver chuva, o display exibe "Sem chuva" e o motor gira na direção contrária para estender o varal.
4. O valor da umidade lida pelo sensor é atualizado no display constantemente.

## Compilação e Execução
1. Instale o SDK do Raspberry Pi Pico.
2. Compile o código utilizando `make` ou outra ferramenta apropriada para o seu ambiente.
3. Carregue o código no Raspberry Pi Pico via USB.
4. Conecte o hardware conforme as definições de pinos no código.

## Requisitos
- Raspberry Pi Pico SDK.
- Biblioteca para o display OLED SSD1306.
- Drivers para o motor de passo (A4988 ou DRV8825).
  
## Considerações Finais
Este projeto é uma solução eficiente e simples para automatizar o processo de recolhimento de roupas em varais, sendo útil em ambientes externos expostos à chuva.
