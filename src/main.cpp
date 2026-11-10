#include <Arduino.h>
#include "FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "DHT20.h"
#include "global.h"
#include "mainserver.h"


#define CORE_1 1
#define I2C_SDA 11
#define I2C_SCL 12
#define BAUDRATE 115200
#define DHT20_INTERVAL 5000

DHT20 dht20; // The DHT20 Address is 0x38
long long prev_time = 0;
SemaphoreHandle_t serialMutex;
SemaphoreHandle_t dataSemaphore;
SemaphoreHandle_t dataMutex;

extern float glob_temperature;
extern float glob_humidity;

void internet_task(void *pvParameters);

void internet_task(void *pvParameters)
{
  while (1)
  {
    // Bị chặn ở đây đến khi main_server_task
    // gọi xSemaphoreGive()
    if (xSemaphoreTake(xBinarySemaphoreInternet,
                       portMAX_DELAY) == pdTRUE)
    {
      Serial.println("[NET] Internet ready");
      // kết nối CoreIOT (MQTT), NTP ...
    }
  }
}

void dht20Func(void* pvParameters)
{
    int status = 0;
    uint8_t dht20_address = 0x00;
    bool onceFlag = 0x00;
    Wire.begin(I2C_SDA,I2C_SCL);
    dht20_address = dht20.getAddress();
    Serial.print("The address of DHT20 is: ");
    Serial.println(dht20_address);
    dht20.begin();
    while(1)
    {
        long long cur_time = millis();
        if((cur_time - prev_time) >= DHT20_INTERVAL)
        {
            if(!onceFlag)
            {
                dht20_address = dht20.getAddress();
                Serial.print("The address of DHT20 is: ");
                Serial.println(dht20_address);
                onceFlag = 0x01;
            }
            prev_time = cur_time;
            status = dht20.read();
            if(status == DHT20_OK)
            {
                // Read temperature in Celsius
                float temperature = dht20.getTemperature();
                // Read humidity in %
                float humidity = dht20.getHumidity();
                /* Check the Not a Number when using float */
                if (isnan(temperature) || isnan(humidity)) {
                    Serial.println("Failed to read from DHT sensor!");
                    temperature = humidity =  -1;
                }
                if(xSemaphoreTake(serialMutex,portMAX_DELAY)){
                    Serial.print("Humidity: ");
                    Serial.print(humidity);
                    Serial.print(" %");
                    Serial.print("===||===");
                    Serial.print("Temperature: ");
                    Serial.print(temperature);
                    Serial.println(" Celsius");
                    xSemaphoreGive(serialMutex);
                }
                if (tb.connected()) {
                    bool tempResult = tb.sendTelemetryData(
                        "temperature",
                        glob_temperature
                    );
        
                    bool humidityResult = tb.sendTelemetryData(
                        "humidity",
                        glob_humidity
                    );
        
                    Serial.printf(
                        "[INFO] Telemetry | Temperature: %s | Humidity: %s\n",
                        tempResult ? "SUCCESS" : "FAILED",
                        humidityResult ? "SUCCESS" : "FAILED"
                    );
                }
                else {
                    Serial.println("[STATUS] TB NOT CONNECTED");
                }
            }
            else
            {
                Serial.println("Failed to read from DHT sensor! - Retrying");
            }
        }
    }
}

 

void ledblinkyFunc(void* pvParameters)
{
    pinMode(GPIO_NUM_48, OUTPUT);
    while(1) {
        Serial.printf("Current Core: %d\n", xPortGetCoreID());
        digitalWrite(GPIO_NUM_48, HIGH);
        Serial.println("LED is ON");
        vTaskDelay(pdMS_TO_TICKS(1000));
        digitalWrite(GPIO_NUM_48, LOW);
        Serial.println("LED is OFF");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void dht20Func_Print(void* pvParameters)
{
    float tempPrint = 0.0;
    float humiPrint = 0.0;
    while(1)
    {
        if(xSemaphoreTake(dataSemaphore, portMAX_DELAY))
        {
            //Serial.println("Task Print: Take Semaphore dataSemaphore - Wake up");
            if(xSemaphoreTake(dataMutex, portMAX_DELAY))
            {
                //Serial.println("Task Print: Take Semaphore dataMutex - Receive Data");
                tempPrint = glob_temperature;
                humiPrint = glob_humidity;
                xSemaphoreGive(dataMutex);
                //Serial.println("Task Print: Give Semaphore dataMutex");
            }
            if(xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                //Serial.println("Task Print: Take Semaphore serialMutex - Print to Console");
                Serial.print("Humidity: ");
                Serial.print(humiPrint);
                Serial.print(" %");
                Serial.print("===||===");
                Serial.print("Temperature: ");
                Serial.print(tempPrint);
                Serial.println(" Celsius");
                xSemaphoreGive(serialMutex);
                //Serial.println("Task Print: Give Semaphore serialMutex");
            }
        }
    }
}

void dht20Func_Read(void* pvParameters)
{
    int status = 0;
    float tempRead = 0.0;
    float humiRead = 0.0;
    uint8_t dht20_address = 0x00;
    TickType_t lastWakeTime = xTaskGetTickCount();
    bool onceFlag = 0x00;

    Wire.begin(I2C_SDA,I2C_SCL);
    dht20.begin();
    while(1)
    {
        // long long cur_time = millis();
        // if((cur_time - prev_time) >= DHT20_INTERVAL)
        // {
        if(!onceFlag)
        {
            dht20_address = dht20.getAddress();
            Serial.print("The address of DHT20 is: ");
            Serial.println(dht20_address);
            onceFlag = 0x01;
        }
        // prev_time = cur_time;
        status = dht20.read();
        if(status == DHT20_OK)
        {
            // Read temperature in Celsius
            tempRead = dht20.getTemperature();
            // Read humidity in %
            humiRead = dht20.getHumidity();
            /* Check the Not a Number when using float */
            if (isnan(tempRead) || isnan(humiRead)) {
                Serial.println("Failed to read from DHT sensor!");
                tempRead = humiRead =  -1;
            }
            if(xSemaphoreTake(dataMutex, portMAX_DELAY))
            {
                //Serial.println("Task Read: Take Semaphore dataMutex - Read Data Form Sensor");
                glob_temperature = tempRead;
                glob_humidity = humiRead;
                xSemaphoreGive(dataMutex);
                //Serial.println("Task Read: Give Semaphore dataMutex");
            }
            //Serial.println("Task Read: Give Semaphore dataMutex - Inform to Task Print");
            xSemaphoreGive(dataSemaphore);
        }
        else
        {
            if(xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                //Serial.println("Task Read: Take Semaphore serialMutex");
                Serial.println("Failed to read from DHT sensor! - Retrying");
                xSemaphoreGive(serialMutex);
                //Serial.println("Task Read: Give Semaphore serialMutex");
            }
        }
        // }
        vTaskDelayUntil(&lastWakeTime,pdMS_TO_TICKS(DHT20_INTERVAL));
    }
}

void setup()
{
    Serial.begin(BAUDRATE);
    xBinarySemaphoreInternet = xSemaphoreCreateBinary();
    serialMutex = xSemaphoreCreateMutex();
    dataMutex = xSemaphoreCreateMutex();
    dataSemaphore = xSemaphoreCreateBinary();
    pinMode(GPIO_NUM_48, OUTPUT);
    //xTaskCreatePinnedToCore(ledblinkyFunc,"Led Blinky",2048,nullptr,2,nullptr,CORE_0);
    xTaskCreatePinnedToCore(dth20Func,"Temperature and Humidity",2048,nullptr,2,nullptr,CORE_1);
    xTaskCreatePinnedToCore(dht20Func_Read,"Temperature and Humidity",2048,nullptr,2,nullptr,CORE_1);
    xTaskCreatePinnedToCore(dht20Func_Print,"Temperature and Humidity",2048,nullptr,2,nullptr,CORE_1);
    xTaskCreate(main_server_task, "server",   8192, NULL, 2, NULL);
    xTaskCreate(internet_task,    "internet", 4096, NULL, 1, NULL);

}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000));}
