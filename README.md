# Controle de Braço Robótico (4-DOF) — emClaw — BeagleBone Black & PCA9685

Sistema embarcado em C para controle em tempo real de um braço robótico/garra com **4 servomotores** (eixos independentes ou em conjunto), utilizando a placa **BeagleBone Black** (Linux Embarcado / Buildroot), leitura analógica via subsistema **IIO (ADC)** e acionamento PWM via barramento **I2C** com o controlador **PCA9685**.

---

## Visão Geral

O projeto estabelece uma ponte de hardware em espaço de usuário (*User Space*) no Linux:
1. **Entrada (Sensores):** Lê 4 potenciômetros conectados aos canais analógicos (`AIN0` a `AIN3`) da BeagleBone Black através da interface `sysfs` do subsistema *Industrial I/O* (`/sys/bus/iio/devices/iio:device0/`).
2. **Processamento:** Converte a leitura digital de 12 bits (`0` a `4095`) em ângulos (`0°` a `180°`), mapeia para largura de pulso em microssegundos (`510 µs` a `2400 µs`) e calcula os *ticks* de 12 bits exigidos para uma frequência de operação de **50 Hz**.
3. **Atuação (Saída):** Envia pacotes de controle via barramento **I2C2** (`/dev/i2c-2`) para o módulo **PCA9685** (endereço `0x40`), atualizando simultaneamente os 4 canais de saída PWM (`LED0` a `LED3`).
4. **Encerramento Seguro (*Graceful Shutdown*):** Intercepta sinais do sistema (`SIGINT` / `Ctrl+C` e `SIGTSTP` / `Ctrl+Z`) para interromper o loop principal e fechar o descritor de arquivo do barramento I2C de forma limpa.

---

## Arquitetura do Código

O projeto foi modularizado separando definições de hardware, mapa de registradores e lógica de controle:

```text
.
├── main.c          # Loop principal de controle, leitura dos 4 ADCs e tratamento de sinais
├── functions.c     # Implementação das rotinas I2C, leitura ADC, setup e escrita PWM
├── functions.h     # Protótipos das funções de controle
├── drivers.h       # Caminhos de dispositivos Linux (/dev/i2c-2, sysfs IIO) e constantes ADC
├── pca_regs.h      # Mapa de registradores do PCA9685 (MODE1, PRESCALE, LEDx) e limites PWM
└── README.md       # Documentação do projeto
```

### Principais Funções
* `int setup(void)`: Abre `/dev/i2c-2`, vincula o endereço escravo `0x40` via `ioctl(I2C_SLAVE)`, coloca o PCA9685 em modo *Sleep*, configura o `PRESCALE` (`0x79`) para **50 Hz** e ativa o modo *Auto-Increment* (`0x20`).
* `int read_potentiometer(unsigned int canal)`: Lê o arquivo `in_voltageX_raw` correspondente ao canal desejado (`0` a `3`) e retorna o valor inteiro (`0` a `4095`).
* `int servo_write(int file, unsigned int canal, float angle)`: Converte o ângulo em largura de pulso ($\mu\text{s}$) e *ticks* de 12 bits, transmitindo 5 bytes via `write()` para os registradores `LEDn_ON` e `LEDn_OFF`.
* `int reg_write(int file, unsigned char reg, unsigned char val)`: Escreve 1 byte em um registrador interno específico do PCA9685.

---

## Esquema de Ligação (Hardware)

### Alerta Crítico de Segurança (ADC)
> **NUNCA alimente os potenciômetros com 5V ou 3.3V!**  
> Os pinos analógicos (`AIN`) da BeagleBone Black operam com **tensão máxima de 1,8V**. Tensões superiores danificarão permanentemente o conversor ADC do processador AM335x. Utilize exclusivamente o pino **`VDD_ADC` (`P9_32`)** como referência positiva e **`GNDA_ADC` (`P9_34`)** como terra analógico.

### 1. Conexão do Módulo PCA9685 (Barramento I2C2)
| Pino PCA9685 | Pino BeagleBone Black | Descrição |
| :--- | :--- | :--- |
| **GND** | `P9_01` ou `P9_02` | Terra comum (DGND) |
| **VCC** | `P9_03` ou `P9_04` | Alimentação lógica (`3.3V`) |
| **SCL** | `P9_19` | Clock I2C (`I2C2_SCL`) |
| **SDA** | `P9_20` | Dados I2C (`I2C2_SDA`) |
| **V+** | *Fonte Externa 5V* | Alimentação de potência exclusiva para os 4 servomotores |

### 2. Conexão dos Potenciômetros (Entradas Analógicas IIO)
* **Terminal 1 (Extremidade):** `P9_32` (`VDD_ADC` — `1.8V`)
* **Terminal 3 (Extremidade):** `P9_34` (`GNDA_ADC` — `GND`)
* **Terminais Centrais (Cursores):**
  * **Potenciômetro 0 (Base / Junta 0):** `P9_39` (`AIN0`) $\rightarrow$ Controla **Canal 0 (`LED0`)**
  * **Potenciômetro 1 (Ombro / Junta 1):** `P9_40` (`AIN1`) $\rightarrow$ Controla **Canal 1 (`LED1`)**
  * **Potenciômetro 2 (Cotovelo / Junta 2):** `P9_37` (`AIN2`) $\rightarrow$ Controla **Canal 2 (`LED2`)**
  * **Potenciômetro 3 (Garra / Efetuador):** `P9_38` (`AIN3`) $\rightarrow$ Controla **Canal 3 (`LED3`)**

---

## Pré-requisitos de Sistema (Kernel & Device Tree)

Para que o código opere corretamente na BeagleBone Black com sistema Linux customizado (Buildroot):

1. **Device Tree (`.dts`):** O controlador `&i2c2` deve estar habilitado (`status = "okay"`) com os pinos `P9_19` e `P9_20` multiplexados em `MUX_MODE3`.
2. **Driver ADC no Kernel:** Certifique-se de que o subsistema Industrial I/O e o driver do ADC estejam habilitados no Kernel (`=y` no `menuconfig`):
   * `CONFIG_IIO=y`
   * `CONFIG_TI_AM335X_ADC=y`
3. **Validação rápida na placa:**
   ```bash
   # Verificar se o PCA9685 responde no endereço 0x40
   i2cdetect -y -r 2

   # Verificar se os canais analógicos estão disponíveis
   ls /sys/bus/iio/devices/iio:device0/in_voltage*_raw
   ```

---

## 🛠️ Compilação e Execução

### 1. Compilação Cruzada (Máquina Host)
Utilize o *toolchain* ARM para compilar os arquivos fonte:

```bash
arm-linux-gnueabihf-gcc -Wall -O2 main.c functions.c -o emclaw
```

### 2. Transferência para a BeagleBone Black
Copie o binário gerado para o diretório compartilhado via NFS ou envie via `scp`:

```bash
cp emclaw /home/$USER/nfsroot/rootfs/home/
```

### 3. Execução na Placa
No terminal serial ou SSH da BeagleBone Black:

```bash
cd /home
./emclaw
```

* Gire os potenciômetros para movimentar cada junta do braço robótico e a garra em tempo real (`~50 Hz` de taxa de atualização).
* Pressione **`Ctrl + C`** (`SIGINT`) ou **`Ctrl + Z`** (`SIGTSTP`) para encerrar a execução e liberar o barramento I2C com segurança.

---

## 📐 Parâmetros de Calibração (`pca_regs.h`)

Caso os servomotores utilizados (mg90s) apresentem limites mecânicos diferentes, ajuste as constantes de largura de pulso em `pca_regs.h`:

```c
#define MIN_PWM 510   // Largura de pulso em µs para 0°
#define MAX_PWM 2400  // Largura de pulso em µs para 180°
```

A conversão matemática aplicada internamente segue a relação:

$$\text{pulse}_{\mu\text{s}} = \text{MIN\_PWM} + \left(\frac{\text{ângulo}}{180.0}\right) \times (\text{MAX\_PWM} - \text{MIN\_PWM})$$

$$\text{ticks} = \left\lfloor \frac{\text{pulse}_{\mu\text{s}} \times 4096}{20000} \right\rfloor$$

---

## 📚 Contexto Acadêmico

Projeto desenvolvido na disciplina de **Técnicas de Programação de Sistemas Embarcados II (TPSE II)** — Curso de Engenharia de Computação, **Universidade Federal do Ceará (UFC) — Campus Quixadá**.
