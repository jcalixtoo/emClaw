#include "functions.h"
#include "pca_regs.h"
#include "drivers.h"

int reg_write(int file, unsigned char reg, unsigned char val)
{
    unsigned char buf[2];
    buf[0] = reg;
    buf[1] = val;
    if(write(file, buf, 2) != 2)
    {
        perror("FALHA AO ESCREVER NO BARRAMENTO I2C\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int read_potentiometer(unsigned int canal)
{
    char adc_path[128];
    snprintf(adc_path, sizeof(adc_path), ADC_PATH_FORMAT, canal);

    int file = open(adc_path, O_RDONLY);
    if(file < 0)
    {
        perror("\nERRO AO ABRIR O BARRAMENTO ADC\n");
        return -1;
    }

    char buf[16];
    ssize_t bytes_read = read(file, buf, sizeof(buf) - 1);
    close(file);

    if(bytes_read <= 0)
    {
        return -1;
    }

    buf[bytes_read] = '\0';

    char *end;
    long adc_val = strtol(buf, &end, 10);
    if(end == buf)
    {
        return -1;
    }

    return (int)adc_val;
}

int servo_write(int file, unsigned int canal, float adc_value)
{
    if(canal > 15)
    {
        fprintf(stderr, "Canal invalido: %u\n", canal);
        return EXIT_FAILURE;
    }

    if(adc_value < 510.0f) adc_value = MIN_PWM;
    if(adc_value > 2400.0f) adc_value = MAX_PWM;
    
    // ADC (0..ADC_MAX_VALUE) -> largura de pulso em us (MIN_PWM..MAX_PWM)
    uint32_t pulse_us = MIN_PWM + (uint32_t)(((float)adc_value / (float)ADC_MAX_VALUE) * (float)(MAX_PWM - MIN_PWM));

    // Largura de pulso (us) -> ticks de 12 bits (50 Hz: 20000 us = 4096 ticks)
    uint16_t ticks = (uint16_t)(((uint64_t)pulse_us * 4096U) / 20000U);


    uint8_t base_reg = LED0_ON_L + (4 * canal);
    uint8_t tx_buff[5];

    tx_buff[0] = base_reg;
    tx_buff[1] = 0x00;                  // LED_ON_L (Início do pulso no tick 0)
    tx_buff[2] = 0x00;                  // LED_ON_H
    tx_buff[3] = ticks & 0xFF;          // LED_OFF_L (Byte baixo do término do pulso)
    tx_buff[4] = (ticks >> 8) & 0xFF;   // LED_OFF_H (Byte alto do término do pulso)

    if(write(file, tx_buff, 5) != 5)
    {
        perror("error ao atualizar pos do servo\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int setup(void)
{
    int file;

/*      PRIMEIRAS CONFIGURAÇÕES PARA GARANTIR QUE TÁ CERTO ABRIR O ARQUIVO E TALZ*/
    file = open(I2C_BUS_PATH, O_RDWR);
    if(file < 0)
    {
        perror("ERRO AO ABRIR O BARRAMENTO I2C");
        close(file);
        return EXIT_FAILURE;
    }

    //define um descritor de arquivo para o i2c. fazendo-o entender como alvo para as syscalls
    if(ioctl(file, I2C_SLAVE, PCA_SLAVE_ADRESS) < 0)
    {
        perror("falha ao definir o enderço do escravo");
        close(file);
        return -1;
    }
/*          FIM  DAS VERIFICAÇÕES E TALZ     */

    printf("\n!!COMUNICACAO BEM SUCEDIDA COM 0x%02X!!\n", PCA_SLAVE_ADRESS);
    
    //0x10 = 0b0001_0000,ou seja escrevo 1 no bit 4 para sinalizar que ficará em sleep
    reg_write(file, MODE1, 0x10);
    
    //escrevo 0x79 para configurar o prescale para definir que os servo-motores atuem em 50hz de frequencia, como especificado no datasheet
    reg_write(file, PRESCALE, 0x79);

    //0x00, ou seja estou retornando para o mode sleep para não mexer mais no valor do prescale e ativando o auto-increment
    //0x20 = 0b0010_0000
    reg_write(file, MODE1, 0x20);

    usleep(10000); // 10 ms

    return file;
}