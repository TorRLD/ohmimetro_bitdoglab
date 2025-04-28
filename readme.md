# OhmiColorMeter

## Ohmímetro Digital com Código de Cores para BitDogLab

---

### 📚 Visão Geral

O **OhmiColorMeter** é um instrumento de medição digital baseado na plataforma **BitDogLab** (RP2040) que transforma a identificação de resistores em uma experiência educativa e intuitiva. Utilizando o princípio do **divisor de tensão**, o dispositivo mede resistências, identifica o valor comercial mais próximo na série **E24**, e exibe o **código de cores** correspondente — ideal para estudantes e entusiastas de eletrônica.

---

### 🔎 Descrição Detalhada

Este projeto combina:

- Eletrônica analógica (divisores de tensão)
- Conversão analógico-digital (ADC)
- Processamento de dados embarcados
- Interface homem-máquina (HMI)

Criando assim uma ferramenta educacional poderosa para o aprendizado de componentes eletrônicos.

---

### ✨ Características Principais

- **Medição Precisa:** ADC de 12 bits para alta resolução.
- **Identificação Inteligente:** Determina o valor comercial mais próximo (série E24).
- **Códigos de Cores Automáticos:** Exibe as faixas de cores do resistor.
- **Interface Intuitiva:** Informações claras em um display OLED.
- **Operação Simples:** Apenas um botão para iniciar nova medição.
- **Feedback Sonoro:** Buzzer integrado para confirmação de ações.
- **Alta Confiabilidade:** Média de 500 leituras para redução de ruído.
- **Diagnóstico Técnico:** Exibição do valor bruto do ADC.

---

### 🛠️ Hardware Utilizado

| Componente                        | Descrição                                 |
| :-------------------------------- | :------------------------------------------ |
| **Plataforma**              | BitDogLab (RP2040)                          |
| **Microcontrolador**        | RP2040 (dual-core ARM Cortex-M0+ @ 133 MHz) |
| **Display**                 | OLED SSD1306 128x64 (I2C)                   |
| **Botão de Medição**     | GPIO 5                                      |
| **Buzzer**                  | GPIO 10                                     |
| **ADC de Medição**        | GPIO 28                                     |
| **I2C**                     | GPIO 14 (SDA), GPIO 15 (SCL)                |
| **Resistor de Referência** | 10kΩ (1% de precisão)                     |
| **Conexão de Teste**       | Terminais para resistor desconhecido        |

---

### ⚙️ Princípio de Funcionamento

1. Formação de divisor de tensão: resistor conhecido (10kΩ) + resistor desconhecido.
2. Leitura da tensão no divisor via ADC (GPIO28).
3. Cálculo da resistência desconhecida:

R_x = R_conhecido × ADC_valor / (ADC_resolução - ADC_valor)

4. Média de 500 leituras para precisão.
5. Identificação do valor comercial mais próximo (E24).
6. Determinação do código de cores e exibição no display.

---

### 🔄 Fluxo de Operação

1. Tela de boas-vindas + beep de inicialização.
2. Aguardar o botão A ser pressionado.
3. Realizar 500 leituras do ADC.
4. Calcular média e determinar resistência.
5. Identificar valor E24 e código de cores.
6. Atualizar o display com:

- Valor medido
- Valor comercial
- Código de cores
- Valor bruto do ADC

7. Retornar ao estado de espera.

---

### ⚡ Aspectos Técnicos Importantes

#### 1. Precisão do Divisor de Tensão

- **Melhor desempenho:** Quando R_x ≈ 10kΩ.
- **Desafios:**
- R_x muito pequeno: Leitura ADC próxima de 0.
- R_x muito grande: Pequenas variações ADC causam grandes erros.
- **Mitigações:**
- Média de leituras.
- Faixa recomendada: **1kΩ a 100kΩ**.

#### 2. Variação com Fontes de Alimentação

- Tensões de referência do ADC variam entre USB e bateria.
- **Nota:** Correção automática não implementada nesta versão.

#### 3. Algoritmo de Identificação de Valores Comerciais

- Normalização do valor lido para encontrar o valor mais próximo na série E24

#### 4. Determinação do Código de Cores

- Conversão dos dígitos da resistência para cores segundo o padrão:

| Dígito | Cor           |
| :------ | :------------ |
| 0       | Preto (PR)    |
| 1       | Marrom (MR)   |
| 2       | Vermelho (VM) |
| 3       | Laranja (LR)  |
| 4       | Amarelo (AM)  |
| 5       | Verde (VD)    |
| 6       | Azul (AZ)     |
| 7       | Violeta (VL)  |
| 8       | Cinza (CZ)    |
| 9       | Branco (BR)   |

---

### 🧩 Estrutura do Código

| Arquivo               | Função                                                  |
| :-------------------- | :-------------------------------------------------------- |
| **ohmimetro.c** | Programa principal: inicialização, medições e lógica |
| **ssd1306.h/c** | Biblioteca para controle do display OLED                  |
| **font.h**      | Definições de fontes para exibição                    |

---


### 🚀 Instalação e Configuração

#### Pré-requisitos

- SDK do Raspberry Pi Pico
- CMake
- GCC ARM Toolchain
- Placa BitDogLab ou hardware compatível com RP2040

---

#### 🔧 Como Compilar

```bash
# Clone o repositório
git clone https://github.com/TorRLD/ohmimetro_bitdoglab.git

# Acesse a pasta do projeto
cd ohmimetro_bitdoglab

# Crie e entre no diretório de build
mkdir build && cd build

# Gere os arquivos de compilação
cmake ..

# Compile o projeto
make

# Após a compilação, localize o arquivo .uf2 gerado dentro da pasta build/
# Conecte seu RP2040 em modo bootloader (pressione o botão BOOTSEL e conecte via USB)
# Copie o arquivo .uf2 para o dispositivo montado como unidade USB
```


### 📄 Licença

Este projeto está licenciado sob a [Licença MIT](LICENSE).
