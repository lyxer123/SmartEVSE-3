/*
;	 Project:       Smart EVSE
;
;

*/

#ifndef __EVSE_MODBUS
#define __EVSE_MODBUS

struct ModBus {
    uint8_t Address;
    uint8_t Function;
    uint16_t Register;
    uint16_t RegisterCount;
    uint16_t Value;
    uint8_t *Data;
    uint8_t DataLength;
    uint8_t Type;
    uint8_t RequestAddress;
    uint8_t RequestFunction;
    uint16_t RequestRegister;
    uint8_t Exception;
};

// definition of MBserver / MBclient class is done in evse.cpp
extern ModbusServerRTU MBserver;
extern ModbusClientRTU MBclient; 

void RS485SendBuf(uint8_t *buffer, uint8_t len);
uint8_t mapModbusRegister2ItemID();

// ########################### Modbus main functions ###########################

void ModbusReadInputRequest(uint8_t address, uint8_t function, uint16_t reg, uint16_t quantity);
void ModbusReadInputResponse(uint8_t address, uint8_t function, uint16_t *values, uint8_t count);
void ModbusWriteSingleRequest(uint8_t address, uint16_t reg, uint16_t value);
void ModbusWriteSingleResponse(uint8_t address, uint16_t reg, uint16_t value);
void ModbusWriteMultipleRequest(uint8_t address, uint16_t reg, uint16_t *values, uint8_t count);
void ModbusWriteMultipleResponse(uint8_t address, uint16_t reg, uint16_t count);
void ModbusException(uint8_t address, uint8_t function, uint8_t exception);
void ModbusDecode(uint8_t *buf, uint8_t len);

// ########################### EVSE modbus functions ###########################

signed int receiveMeasurement(uint8_t *buf, uint8_t pos, uint8_t Endianness, MBDataType dataType, signed char Divisor);
void requestEnergyMeasurement(uint8_t Meter, uint8_t Address);
signed int receiveEnergyMeasurement(uint8_t *buf, uint8_t Meter);
void requestPowerMeasurement(uint8_t Meter, uint8_t Address);
signed int receivePowerMeasurement(uint8_t *buf, uint8_t Meter);
void requestCurrentMeasurement(uint8_t Meter, uint8_t Address);
uint8_t receiveCurrentMeasurement(uint8_t *buf, uint8_t Meter, signed int *var);

void ReadItemValueResponse(void);
void WriteItemValueResponse(void);
void WriteMultipleItemValueResponse(void);


#endif