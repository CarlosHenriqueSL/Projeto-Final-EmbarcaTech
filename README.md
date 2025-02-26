# Projeto Final**
**EMBARCATECH - UNIDADE 07**

## Aluno: 
- **Carlos Henrique Silva Lopes**

## **Link do vídeo de Ensaio**

## **Descrição do Projeto**
Este repositório contém o projeto final do curso em sistemas embarcados, EmbarcaTech. Esse projeto foi feito visando simular um sistema de irrigação usando a placa BitDogLab e seus componentes.

## Funcionalidades

- **Joystick:**  
  Utiliza ADC para ler o eixo Y (GPIO26) e ajustar a intensidade da irrigação em incrementos de 20%, com zona morta para evitar ajustes acidentais.
- **LED Vermelho (PWM):**  
  O brilho é ajustado proporcionalmente à intensidade selecionada, com controle via PWM no GPIO13.
- **Matriz de LEDs WS2812 (PIO):**  
  Exibe padrões pré-definidos para diferentes níveis de irrigação (0%, 20%, 40%, 60%, 80% e 100%), otimizando o uso dos recursos da CPU.
- **Display SSD1306 (I2C):**  
  Exibe mensagens como “Intensidade: X%” e o estado do sistema, conforme a leitura do joystick.
- **Botão A (GPIO5):**  
  Alterna o estado do sistema (ligado/desligado) com debounce de 500 ms, desativando ou ativando os LEDs e o display.


## Configuração do Projeto

1. Clone este repositório para sua máquina:

   ```
   https://github.com/CarlosHenriqueSL/Projeto-Final-EmbarcaTech.git
   cd Projeto-Final-EmbarcaTech
   ```

2. Certifique-se de que o **Pico SDK** está configurado no VS Code.

3. Abra o projeto no **VS Code** e configure os arquivos `CMakeLists.txt` adequados para compilar o código.

4. Execute o projeto no simulador **Wokwi** ou faça o upload para o hardware físico do Raspberry Pi Pico W.


## Estrutura do Código

### Principais Arquivos
- **`ProjetoFinal.c:`** Contém a lógica principal para fazer a leitura do joystick, do botão, e mostrar as informações na matriz de LEDs, display e LED RGB.
- **`diagram.json:`** Configuração física do projeto no simulador Wokwi.
- **`inc/:`** Pasta onde fica a lógica para desenhar no display ssd1306.
- **`numeros/:`** Pasta onde ficam as porcentagens que serão exibidas na matriz de LEDs.
- **`README.md:`** Documentação detalhada do projeto.

## Testes e Validação

1. Ao movimentar o joystick, a intensidade da irrigação deve ser ajustada em incrementos de 20%, com feedback visual no display e na matriz de LEDs.
 
2. O LED vermelho deve exibir um brilho proporcional à intensidade selecionada.
   
3. A matriz de LEDs WS2812 deve exibir os padrões correspondentes aos níveis de irrigação configurados.
  
4. Ao pressionar o botão A (GPIO5), o sistema deve alternar entre os estados ligado e desligado, com debounce eficaz, desativando ou ativando os LEDs e o display.

