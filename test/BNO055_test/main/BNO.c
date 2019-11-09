#include "BNO.h"

//Initialise and power ON, bno
esp_err_t write_byte(i2c_port_t i2c_num,int8_t main_add, int8_t reg_add, int8_t data){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( main_add << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_add, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    vTaskDelay(10/portTICK_RATE_MS);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t read_byte(i2c_port_t i2c_num,int8_t main_add, int8_t reg_add, uint8_t* data_rd, size_t size){
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( main_add << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_add, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( main_add << 1 ) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}


esp_err_t bno_init(i2c_port_t i2c_num){
    uint8_t a;
    esp_err_t ret=read_byte(I2C_MASTER_NUM,0x28,0x00,&a,1);
    if(a == 57)
    {
        ret=ESP_OK;
    }
    printf("a:%d\tret:%d\n", a,ret);
    return ret;
}

esp_err_t bno_read_mag(i2c_port_t i2c_num, uint8_t* data_rd, size_t size){
    write_to_mag(CNTL_1, 0x00);
    esp_err_t ret = write_to_mag(CNTL_1, 0x16);
    read_from_mag(MAG_START_ADDR, 6, data_rd);
    return ret;
}

//Initialise the I2C bus and install driver to specified pins
void i2c_master_init(){
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

//Proceed only when MPU is initialised
void start_bno(){   
    int ret = bno_init(I2C_MASTER_NUM);
    
    //CHECK IF MPU IS ACTIVE
    while(ret != ESP_OK)     {
        printf("INIT FAILED... Retry\n");
        vTaskDelay(100/ portTICK_RATE_MS);
        ret = bno_init(I2C_MASTER_NUM);
    }
    printf("INIT SUCESS...\n"); 
}

void shift_buf_mag(uint8_t* buf_1, int16_t* buf_2){
    buf_2[0] = ((buf_1[1] << 8) + buf_1[0]);
    buf_2[1] = ((buf_1[3] << 8) + buf_1[2]);
    buf_2[2] = ((buf_1[5] << 8) + buf_1[4]);
}

void get_euler(float *x, float *y, float *z)
{
    uint8_t temp[6];
    int16_t raw[3];
    esp_err_t ret =read_byte(I2C_MASTER_NUM, 0x28, 0x1A, temp, 6);
    vTaskDelay(50/portTICK_RATE_MS);
    shift_buf_mag(raw, temp);
    *x=raw[0];
    *y=raw[1];
    *z=raw[2];
}

// void get_mag_raw(int16_t* raw_value){
//     uint8_t mag_rd[BUFF_SIZE];
//     int ret = bno_read_mag(I2C_MASTER_NUM, mag_rd, BUFF_SIZE);
//     shift_buf_mag(mag_rd, raw_value, BUFF_SIZE/2);
// }

// void get_yaw(float* angle){
//     double mx, my;
//     int16_t mag_raw[BUFF_SIZE / 2];
//     get_mag_raw(mag_raw);
//     if(!mag_raw[0] && !mag_raw[1])  //Check if mpu requires re-initialization
//         start_mpu();
//     mx = (double)mag_raw[0] - offset_x;
//     my = (double)mag_raw[1] - offset_y;
//     mx *= scale_x;
//     my *= scale_y;
//     *angle = atan2(my,mx) * (180 / 3.141592654) + 180;
// }