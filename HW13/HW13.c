#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17


#define I2CLED_PORT i2c1
#define I2CLED_SDA 16
#define I2CLED_SCL 17
// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B

#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75


// It's better to initialize global variables at runtime (e.g., in main or a setup/init function) rather than at declaration,
// especially when the initialization depends on hardware or system state (like current time).
// Instead, declare the variable here:
uint64_t t_start;

void imu_init();
void imu_read_data(char label);

void imu_init()
{

    printf("IMU init\n");
    uint8_t data[2];
    data[0] = PWR_MGMT_1;
    data[1] = 0x00; // wake up the device
    i2c_write_blocking(I2C_PORT, 0x68, data, 2, false);
    printf("IMU wake up done\n");
    
    data[0] = GYRO_CONFIG;
    data[1] = 0xF8; // set gyro range to +/- 250 degrees/s
    i2c_write_blocking(I2C_PORT,0x68, data, 2, false);
    printf("IMU gyro config done\n");
    
    data[0] = ACCEL_CONFIG;
    data[1] = 0xE0; // set accel range to +/- 2g
    i2c_write_blocking(I2C_PORT, 0x68 , data, 2, false);
    printf("IMU accel config done\n");
    printf("IMU init done\n");
}

int16_t[] imu_read_data(char label)
{
    uint8_t data[12];
    uint8_t addresses[12] = {
        ACCEL_XOUT_L, ACCEL_XOUT_H,
        ACCEL_YOUT_L, ACCEL_YOUT_H,
        ACCEL_ZOUT_L, ACCEL_ZOUT_H,
        GYRO_XOUT_L, GYRO_XOUT_H,
        GYRO_YOUT_L, GYRO_YOUT_H,
        GYRO_ZOUT_L, GYRO_ZOUT_H
    };
    int num_addresses = sizeof(addresses) / sizeof(addresses[0]);
    uint64_t t_end = to_us_since_boot(get_absolute_time());

    uint64_t elapsed = t_end - t_start;
    for (int i = 0; i < num_addresses; i++) {
        uint8_t reg = addresses[i];
        // Write register address
        i2c_write_blocking(I2C_PORT, 0x68, &reg, 1, true);
        // Read one byte from each address
        i2c_read_blocking(I2C_PORT, 0x68, &data[i], 1, false);
    }
    


    int16_t accel_x = (data[1] << 8) | data[0];
    int16_t accel_y = (data[3] << 8) | data[2];
    int16_t accel_z = (data[5] << 8) | data[4];
    int16_t gyro_x  = (data[7] << 8) | data[6];
    int16_t gyro_y  = (data[9] << 8) | data[8];
    int16_t gyro_z  = (data[11] << 8) | data[10];
    printf("Accel X: %d, Y: %d, Z: %d | Gyro X: %d, Y: %d, Z: %d\n", accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
    

    return {accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z};
    
    // Write to CSV file with headers if file is new
    
    // FILE *fp = fopen("imu_data.csv", "r");
    // int file_exists = (fp != NULL);
    // if (fp) fclose(fp);

    // fp = fopen("imu_data.csv", "a");
    // if (fp != NULL) {
    //     if (!file_exists) {
    //         fprintf(fp, "accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,label\n");
    //     }
    //     fprintf(fp, "%d,%d,%d,%d,%d,%d,%c\n", accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, label);
    //     fclose(fp);
    // } else {
    //     printf("Failed to open imu_data.csv for writing\n");
    // }
    


}
void sketchvector(float x_accel, float y_accel){
    ssd1306_clear();
    ssd1306_drawPixel(60, 15, 1); // around the middle of the screen
    ssd1306_update();
    // Use descriptive variable names and combine logic for efficiency
    int x_center = 60;
    int y_center = 15;
    int x_length = (int)(30.0f * fabsf(x_accel));
    int y_length = (int)(10.0f * fabsf(y_accel));

    int x_dir = (x_accel >= 0.0f) ? -1 : 1;
    int y_dir = (y_accel >= 0.0f) ? 1 : -1;

    for (int i = 0; i < x_length; i++) {
        ssd1306_drawPixel(x_center + i * x_dir, y_center, 1);
    }
    for (int j = 0; j < y_length; j++) {
        ssd1306_drawPixel(x_center, y_center + j * y_dir, 1);
    }
    ssd1306_update();
}
int main()
{
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start 2!\n");
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    imu_init();
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    i2c_init(I2CLED_PORT, 400*1000);

    gpio_set_function(I2CLED_SCL, GPIO_FUNC_I2C);
    gpio_set_function(I2CLED_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(I2CLED_SCL);
    gpio_pull_up(I2CLED_SDA);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c



    t_start = to_us_since_boot(get_absolute_time());
    uint16_t accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z;
    while (true) {
        sleep_ms(5000);
        printf("Hello, world!\n");



        // int c = getchar_timeout_us(0); // non-blocking read
        // char label = 'N'; // default label
        // if (c != PICO_ERROR_TIMEOUT && c != EOF) {
        //     label = (char)c;
        // }
        char label = '1'; // default label
        accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z = imu_read_data(label);
        sketchvector((float)accel_x / 16384.0f, (float)accel_y / 16384.0f); 
        
    }
}
