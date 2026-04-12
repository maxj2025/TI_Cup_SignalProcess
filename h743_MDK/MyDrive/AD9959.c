#include "AD9959.h"
#include "bsp_system.h" 

// --- 寄存器默认配置 ---
uint8_t FR1_DATA[3] = {0xD0, 0x00, 0x00}; 
uint8_t FR2_DATA[2] = {0x80, 0x00};       
uint8_t CFR_DATA[3] = {0x00, 0x03, 0x02}; 

// 频率因子 K = (2^32) / 500MHz
double ACC_FRE_FACTOR = 8.589934592;


void Intserve(void) {
    HAL_GPIO_WritePin(AD9959_PDC_GPIO_Port, AD9959_PDC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AD9959_CS_GPIO_Port, AD9959_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AD9959_SCLK_GPIO_Port, AD9959_SCLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AD9959_UPDATE_GPIO_Port, AD9959_UPDATE_Pin, GPIO_PIN_RESET);
}

void IntReset(void) {
    HAL_GPIO_WritePin(AD9959_RESET_GPIO_Port, AD9959_RESET_Pin, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(AD9959_RESET_GPIO_Port, AD9959_RESET_Pin, GPIO_PIN_SET);
    delay_us(5); 
    HAL_GPIO_WritePin(AD9959_RESET_GPIO_Port, AD9959_RESET_Pin, GPIO_PIN_RESET);
    delay_us(10); 
}

void IO_Update(void) {
    HAL_GPIO_WritePin(AD9959_UPDATE_GPIO_Port, AD9959_UPDATE_Pin, GPIO_PIN_RESET);
    delay_us(1);
    HAL_GPIO_WritePin(AD9959_UPDATE_GPIO_Port, AD9959_UPDATE_Pin, GPIO_PIN_SET);
    delay_us(1); 
    HAL_GPIO_WritePin(AD9959_UPDATE_GPIO_Port, AD9959_UPDATE_Pin, GPIO_PIN_RESET);
}

void AD9959_WriteData(uint8_t RegisterAddress, uint8_t NumberofRegisters, uint8_t *RegisterData) {
    uint8_t ControlValue = RegisterAddress;
    HAL_GPIO_WritePin(AD9959_CS_GPIO_Port, AD9959_CS_Pin, GPIO_PIN_RESET);
    for(int i=0; i<8; i++) {
        HAL_GPIO_WritePin(AD9959_SCLK_GPIO_Port, AD9959_SCLK_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AD9959_SDIO0_GPIO_Port, AD9959_SDIO0_Pin, (ControlValue & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        delay_us(1); 
        HAL_GPIO_WritePin(AD9959_SCLK_GPIO_Port, AD9959_SCLK_Pin, GPIO_PIN_SET);
        delay_us(1); 
        ControlValue <<= 1;
    }
    for (int i=0; i<NumberofRegisters; i++) {
        uint8_t ValueToWrite = RegisterData[i];
        for (int j=0; j<8; j++) {
            HAL_GPIO_WritePin(AD9959_SCLK_GPIO_Port, AD9959_SCLK_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(AD9959_SDIO0_GPIO_Port, AD9959_SDIO0_Pin, (ValueToWrite & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            delay_us(1);
            HAL_GPIO_WritePin(AD9959_SCLK_GPIO_Port, AD9959_SCLK_Pin, GPIO_PIN_SET);
            delay_us(1);
            ValueToWrite <<= 1;
        }
    }
    HAL_GPIO_WritePin(AD9959_CS_GPIO_Port, AD9959_CS_Pin, GPIO_PIN_SET);
}

void Write_CFTW0(uint32_t fre) {
    uint8_t data[4];
    uint32_t Temp = (uint32_t)(fre * ACC_FRE_FACTOR);
    data[3] = (uint8_t)Temp; data[2] = (uint8_t)(Temp >> 8);
    data[1] = (uint8_t)(Temp >> 16); data[0] = (uint8_t)(Temp >> 24);
    AD9959_WriteData(0x04, 4, data);
}

void Write_ACR(uint16_t Ampli) {
    uint8_t data[3] = {0, 0, 0};
    uint32_t A_temp = Ampli | 0x1000; 
    data[1] = (uint8_t)(A_temp >> 8); data[2] = (uint8_t)A_temp;
    AD9959_WriteData(0x06, 3, data);
}

void Write_CPOW0(uint16_t Phase) {
    uint8_t data[2];
    data[1] = (uint8_t)Phase; data[0] = (uint8_t)(Phase >> 8);
    AD9959_WriteData(0x05, 2, data);
}

void Write_LSRR(uint8_t rsrr, uint8_t fsrr) {
    uint8_t data[2] = {fsrr, rsrr};
    AD9959_WriteData(0x07, 2, data);
}

void Write_RDW(uint32_t r_delta) {
    uint8_t data[4];
    data[3] = (uint8_t)r_delta; data[2] = (uint8_t)(r_delta >> 8);
    data[1] = (uint8_t)(r_delta >> 16); data[0] = (uint8_t)(r_delta >> 24);
    AD9959_WriteData(0x08, 4, data);
}

void Write_FDW(uint32_t f_delta) {
    uint8_t data[4];
    data[3] = (uint8_t)f_delta; data[2] = (uint8_t)(f_delta >> 8);
    data[1] = (uint8_t)(f_delta >> 16); data[0] = (uint8_t)(f_delta >> 24);
    AD9959_WriteData(0x09, 4, data);
}


void Write_Profile_Fre(uint8_t profile, uint32_t data) {
    uint8_t pdata[4];
    uint32_t Temp = (uint32_t)(data * ACC_FRE_FACTOR);
    pdata[3] = (uint8_t)Temp; pdata[2] = (uint8_t)(Temp >> 8);
    pdata[1] = (uint8_t)(Temp >> 16); pdata[0] = (uint8_t)(Temp >> 24);
    AD9959_WriteData(0x0A + profile, 4, pdata);
}

void Write_Profile_Ampli(uint8_t profile, uint16_t data) {
    uint8_t pdata[4] = {0, 0, 0, 0};
    pdata[1] = (uint8_t)(data << 6); pdata[0] = (uint8_t)(data >> 2);
    AD9959_WriteData(0x0A + profile, 4, pdata);
}

void Write_Profile_Phase(uint8_t profile, uint16_t data) {
    uint8_t pdata[4] = {0, 0, 0, 0};
    pdata[1] = (uint8_t)(data << 2); pdata[0] = (uint8_t)(data >> 6);
    AD9959_WriteData(0x0A + profile, 4, pdata);
}


void AD9959_Init(void) {
    Intserve(); IntReset();
    AD9959_WriteData(0x01, 3, FR1_DATA);
    AD9959_WriteData(0x02, 2, FR2_DATA);
    IO_Update();
}

void AD9959_Set_Fre(uint8_t Channel, uint32_t Freq) {
    AD9959_WriteData(0x00, 1, &Channel);
    Write_CFTW0(Freq);
    IO_Update();
}

void AD9959_Set_Amp(uint8_t Channel, uint16_t Ampli) {
    AD9959_WriteData(0x00, 1, &Channel);
    Write_ACR(Ampli);
    IO_Update();
}

void AD9959_Set_Phase(uint8_t Channel, uint16_t Phase) {
    AD9959_WriteData(0x00, 1, &Channel);
    Write_CPOW0(Phase);
    IO_Update();
}

void AD9959_setsine(uint8_t Channel, uint32_t Freq, uint16_t Ampli, uint16_t Phase) {
    AD9959_WriteData(0x00, 1, &Channel);
    Write_CFTW0(Freq);
    Write_ACR(Ampli);
    Write_CPOW0(Phase);
    IO_Update();
}

void AD9959_Modulation_Init(uint8_t Channel, uint8_t Modulation, uint8_t Sweep_en, uint8_t Nlevel) {
    uint8_t FR1_local[3], CFR_local[3];
    for(int i=0; i<3; i++) { FR1_local[i] = FR1_DATA[i]; CFR_local[i] = CFR_DATA[i]; }
    AD9959_WriteData(0x00, 1, &Channel);
    FR1_local[1] = Nlevel;
    CFR_local[0] = Modulation; CFR_local[1] |= Sweep_en;
    AD9959_WriteData(0x01, 3, FR1_local);
    AD9959_WriteData(0x03, 3, CFR_local);
    IO_Update();
}

void AD9959_SetFre_Sweep(uint8_t Channel, uint32_t s_data, uint32_t e_data, uint32_t r_delta, uint32_t f_delta, uint8_t rsrr, uint8_t fsrr, uint16_t Ampli, uint16_t Phase) {
    AD9959_WriteData(0x00, 1, &Channel);
    Write_CPOW0(Phase); Write_ACR(Ampli);
    Write_LSRR(rsrr, fsrr);
    Write_RDW((uint32_t)(r_delta * ACC_FRE_FACTOR));
    Write_FDW((uint32_t)(f_delta * ACC_FRE_FACTOR));
    Write_CFTW0(s_data);
    Write_Profile_Fre(0, e_data);
    IO_Update();
}