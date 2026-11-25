#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/*
 * @brief Escreve um valor de 8 bits em um registrador específico de um dispositivo I2C.
 * 
 * @details Esta função encapsula a chamada de sistema write() para enviar um pacote 
 *          de dois bytes (endereço do registrador seguido do valor) para o barramento I2C.
 * 
 * @param[int] file Descritor de arquivo válido do barramento I2C (obtido via open()).
 * @param[int] reg  Endereço do registrador de destino no dispositivo escravo (0x00 a 0xFF).
 * @param[int] val  Valor de 8 bits a ser gravado no registrador.
 * 
 * @return int 
 *         - @ref EXIT_SUCCESS em caso de sucesso na escrita dos 2 bytes.
 *         - @ref EXIT_FAILURE se a operação de escrita falhar (o erro é logado via perror).
 * 
 * @note Requer que o descritor de arquivo esteja previamente aberto e configurado 
 *       para o endereço correto do escravo I2C (via ioctl I2C_SLAVE).
 */
int reg_write(int file, unsigned char reg, unsigned char val);

/*
 * @brief Lê o valor bruto atual do potenciômetro conectado ao canal ADC do sistema.
 * 
 * @details Abre o arquivo de interface do driver do ADC (definido por ADC_PATH),
 *          faz a leitura em modo texto do valor numérico bruto e fecha o arquivo.
 * 
 * @param[int] canal  Índice do canal do potenciômetro a ser lido.
 * 
 * @return int 
 *         - O valor inteiro lido do ADC (tipicamente na faixa de 0 a 4095 para conversores de 12 bits).
 *         - `-1` se houver falha ao abrir o arquivo do ADC ou se a leitura formatada falhar.
 * 
 */
int read_potentiometer(unsigned int canal);

/*
 * @brief Controla a posição angular de um servomotor conectado a um canal do driver PCA9685 (PWM).
 * 
 * @details Converte o ângulo desejado em microssegundos de pulso PWM com base nos limites 
 *          globais MIN_PWM e MAX_PWM, traduz para pulsos de clock (ticks de 12 bits) 
 *          e envia o comando via barramento I2C.
 * 
 * @param[int] file   Descritor de arquivo válido do barramento I2C do driver PWM.
 * @param[int] canal  Índice do canal do servo (geralmente de 0 a 15 no PCA9685).
 * @param[int] angle  Ângulo desejado para o eixo do servo em graus (limitado rigidamente entre 0.0f e 180.0f).
 * 
 * @return int 
 *         - @ref EXIT_SUCCESS se os registradores de controle do canal forem atualizados com sucesso.
 *         - @ref EXIT_FAILURE se houver erro de barramento na escrita I2C.
 * 
 * @note Ângulos fora do intervalo [0.0, 180.0] são saturados automaticamente para o limite mais próximo.
 */
int servo_write(int file, unsigned int canal, float angle);

/*
 *  @brief Abre o barramento I2C e faz a configuração inicial do módulo PCA9685.
 *
 *  @details A função abre o dispositivo do barramento I2C, associa o endereço do
 *           PCA9685 ao descritor (ioctl I2C_SLAVE) e escreve nos registradores
 *           MODE1 e PRESCALE para configurar a frequência de atuação dos servos
 *           motores em 50 Hz.
 *
 *  @return Descritor de arquivo do barramento I2C em caso de sucesso, ou -1 em
 *          caso de erro (o motivo é impresso com perror). Em caso de erro, nenhum
 *          descritor fica aberto.
 *
 *  @note   O descritor retornado pertence ao chamador. Ele deve ser usado nas
 *          operações seguintes com o PCA9685 (reg_write, etc.) e fechado com
 *          close() quando não for mais necessário. Não é possível fechá-lo
 *          dentro desta função sem inutilizar a comunicação posterior.
 */
int setup(void);

#endif