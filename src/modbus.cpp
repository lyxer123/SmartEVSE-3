/*
;	 Project:       Smart EVSE
;
;

*/

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include "ModbusServerRTU.h"
#include "ModbusClientRTU.h"
#include "driver/uart.h"

#include "evse.h"
#include "modbus.h"
#include "utils.h"

extern struct ModBus MB;


// ########################## Modbus helper functions ##########################

/**
 * Send single value over modbus
 * 
 * @param uint8_t address
 * @param uint8_t function
 * @param uint16_t register
 * @param uint16_t data
 */
void ModbusSend8(uint8_t address, uint8_t function, uint16_t reg, uint16_t data) {
    // 0x12345678 is a token to keep track of modbus requests/responses. currently unused.
    MBclient.addRequest(0x12345678, address, function, reg, data);
}

/**
 * Combine Bytes received over modbus
 * 
 * @param pointer to var
 * @param pointer to buf
 * @param uint8_t pos
 * @param uint8_t endianness:\n
 *        0: low byte first, low word first (little endian)\n
 *        1: low byte first, high word first\n
 *        2: high byte first, low word first\n
 *        3: high byte first, high word first (big endian)
 * @param MBDataType dataType: used to determine how many bytes should be combined
 */
void combineBytes(void *var, uint8_t *buf, uint8_t pos, uint8_t endianness, MBDataType dataType) {
    char *pBytes;
    pBytes = (char *)var;

    // ESP32 is little endian
    switch(endianness) {
        case ENDIANESS_LBF_LWF: // low byte first, low word first (little endian)
            *pBytes++ = (uint8_t)buf[pos + 0];
            *pBytes++ = (uint8_t)buf[pos + 1];
            if (dataType != MB_DATATYPE_INT16) {
                *pBytes++ = (uint8_t)buf[pos + 2];
                *pBytes   = (uint8_t)buf[pos + 3];
            }
            break;
        case ENDIANESS_LBF_HWF: // low byte first, high word first
            if (dataType != MB_DATATYPE_INT16) {
                *pBytes++ = (uint8_t)buf[pos + 2];
                *pBytes++ = (uint8_t)buf[pos + 3];
            }
            *pBytes++ = (uint8_t)buf[pos + 0];
            *pBytes   = (uint8_t)buf[pos + 1];
            break;
        case ENDIANESS_HBF_LWF: // high byte first, low word first
            *pBytes++ = (uint8_t)buf[pos + 1];
            *pBytes++ = (uint8_t)buf[pos + 0];
            if (dataType != MB_DATATYPE_INT16) {
                *pBytes++ = (uint8_t)buf[pos + 3];
                *pBytes   = (uint8_t)buf[pos + 2];
            }
            break;
        case ENDIANESS_HBF_HWF: // high byte first, high word first (big endian)
            if (dataType != MB_DATATYPE_INT16) {
                *pBytes++ = (uint8_t)buf[pos + 3];
                *pBytes++ = (uint8_t)buf[pos + 2];
            }
            *pBytes++ = (uint8_t)buf[pos + 1];
            *pBytes   = (uint8_t)buf[pos + 0];
            break;
        default:
            break;
    }
}



// ########################### Modbus main functions ###########################



/**
 * Request read holding (FC=3) or read input register (FC=04) to a device over modbus
 * 
 * @param uint8_t address
 * @param uint8_t function
 * @param uint16_t register
 * @param uint16_t quantity
 */
void ModbusReadInputRequest(uint8_t address, uint8_t function, uint16_t reg, uint16_t quantity) {
    MB.RequestAddress = address;
    MB.RequestFunction = function;
    MB.RequestRegister = reg;
    ModbusSend8(address, function, reg, quantity);
}

/**
 * Response read holding (FC=3) or read input register (FC=04) to a device over modbus
 * 
 * @param uint8_t address
 * @param uint8_t function
 * @param uint16_t pointer to values
 * @param uint8_t count of values
 */
void ModbusReadInputResponse(uint8_t address, uint8_t function, uint16_t *values, uint8_t count) {
    Serial.printf("ModbusReadInputResponse, to do!\n");
    //ModbusSend(address, function, count * 2u, values, count);
}

/**
 * Request write single register (FC=06) to a device over modbus
 * 
 * @param uint8_t address
 * @param uint16_t register
 * @param uint16_t value
 */
void ModbusWriteSingleRequest(uint8_t address, uint16_t reg, uint16_t value) {
    MB.RequestAddress = address;
    MB.RequestFunction = 0x06;
    MB.RequestRegister = reg;
    ModbusSend8(address, 0x06, reg, value);  
}

/**
 * Response write single register (FC=06) to a device over modbus
 * 
 * @param uint8_t address
 * @param uint16_t register
 * @param uint16_t value
 */
void ModbusWriteSingleResponse(uint8_t address, uint16_t reg, uint16_t value) {
    ModbusSend8(address, 0x06, reg, value);  
}


/**
 * Request write multiple register (FC=16) to a device over modbus
 * 
 * @param uint8_t address
 * @param uint16_t register
 * @param uint8_t pointer to data
 * @param uint8_t count of data
 */
void ModbusWriteMultipleRequest(uint8_t address, uint16_t reg, uint16_t *values, uint8_t count) {

    MB.RequestAddress = address;
    MB.RequestFunction = 0x10;
    MB.RequestRegister = reg;
    // 0x12345678 is a token to keep track of modbus requests/responses. currently unused.
    MBclient.addRequest(0x12345678, address, 0x10, reg, (uint16_t) count, count * 2u, values);
}

/**
 * Response write multiple register (FC=16) to a device over modbus
 * 
 * @param uint8_t address
 * @param uint16_t register
 * @param uint16_t count
 */
void ModbusWriteMultipleResponse(uint8_t address, uint16_t reg, uint16_t count) {
    ModbusSend8(address, 0x10, reg, count);
}

/**
 * Response an exception
 * 
 * @param uint8_t address
 * @param uint8_t function
 * @param uint8_t exeption
 */
void ModbusException(uint8_t address, uint8_t function, uint8_t exception) {
    //uint16_t temp[1];
    Serial.print("ModbusException, to do!\n");
    //ModbusSend(address, function, exception, temp, 0);
}

/**
 * Decode received modbus packet
 * 
 * @param uint8_t pointer to buffer
 * @param uint8_t length of buffer
 */
void ModbusDecode(uint8_t * buf, uint8_t len) {
    // Clear old values
    MB.Address = 0;
    MB.Function = 0;
    MB.Register = 0;
    MB.RegisterCount = 0;
    MB.Value = 0;
    MB.DataLength = 0;
    MB.Type = MODBUS_INVALID;
    MB.Exception = 0;

#ifdef LOG_INFO_MODBUS
    Serial.print("Received packet");
#endif
#ifdef LOG_DEBUG_MODBUS
    Serial.printf(" (%i bytes) ", len);
    for (uint8_t x=0; x<len; x++) Serial.printf("%02x ", buf[x]);
    Serial.print("\n");
#endif

    // Modbus error packets length is 5 bytes
    if (len == 3) {
        MB.Type = MODBUS_EXCEPTION;
        // Modbus device address
        MB.Address = buf[0];
        // Modbus function
        MB.Function = buf[1];
        // Modbus Exception code
        MB.Exception = buf[2];
    // Modbus data packets minimum length is 8 bytes
    } else if (len >= 6) {
        // Modbus device address
        MB.Address = buf[0];
        // Modbus function
        MB.Function = buf[1];

#ifdef LOG_DEBUG_MODBUS
            Serial.printf(" valid Modbus packet: Address %02x Function %02x", MB.Address, MB.Function);
#endif
        switch (MB.Function) {
            case 0x03: // (Read holding register)
            case 0x04: // (Read input register)
                if (len == 6) {
                    // request packet
                    MB.Type = MODBUS_REQUEST;
                    // Modbus register
                    MB.Register = (uint16_t)(buf[2] <<8) | buf[3];
                    // Modbus register count
                    MB.RegisterCount = (uint16_t)(buf[4] <<8) | buf[5];
                } else {
                    // Modbus datacount
                    MB.DataLength = buf[2];
                    if (MB.DataLength == len - 3) {
                        // packet length OK
                        // response packet
                        MB.Type = MODBUS_RESPONSE;
#ifdef LOG_WARN_MODBUS
                    } else {
                        Serial.print("Invalid modbus FC=04 packet\n");
#endif
                    }
                }
                break;
            case 0x06:
                // (Write single register)
                if (len == 6) {
                    // request and response packet are the same
                    MB.Type = MODBUS_OK;
                    // Modbus register
                    MB.Register = (uint16_t)(buf[2] <<8) | buf[3];
                    // Modbus register count
                    MB.RegisterCount = 1;
                    // value
                    MB.Value = (uint16_t)(buf[4] <<8) | buf[5];
#ifdef LOG_WARN_MODBUS
                } else {
                    Serial.print("Invalid modbus FC=06 packet\n");
#endif
                }
                break;
            case 0x10:
                // (Write multiple register))
                // Modbus register
                MB.Register = (uint16_t)(buf[2] <<8) | buf[3];
                // Modbus register count
                MB.RegisterCount = (uint16_t)(buf[4] <<8) | buf[5];
                if (len == 6) {
                    // response packet
                    MB.Type = MODBUS_RESPONSE;
                } else {
                    // Modbus datacount
                    MB.DataLength = buf[6];
                    if (MB.DataLength == len - 7) {
                        // packet length OK
                        // request packet
                        MB.Type = MODBUS_REQUEST;
#ifdef LOG_WARN_MODBUS
                    } else {
                        Serial.print("Invalid modbus FC=16 packet\n");
#endif
                    }
                }
                break;
            default:
                break;
        }

        // MB.Data
        if (MB.Type && MB.DataLength) {
            // Set pointer to Data
            MB.Data = buf;
            // Modbus data is always at the end ahead the checksum
            MB.Data = MB.Data + (len - MB.DataLength);
        }
        
        // Request - Response check
        switch (MB.Type) {
            case MODBUS_REQUEST:
                MB.RequestAddress = MB.Address;
                MB.RequestFunction = MB.Function;
                MB.RequestRegister = MB.Register;
                break;
            case MODBUS_RESPONSE:
                // If address and function identical with last send or received request, it is a valid response
                if (MB.Address == MB.RequestAddress && MB.Function == MB.RequestFunction) {
                    if (MB.Function == 0x03 || MB.Function == 0x04) 
                        MB.Register = MB.RequestRegister;
                }
                MB.RequestAddress = 0;
                MB.RequestFunction = 0;
                MB.RequestRegister = 0;
                break;
            case MODBUS_OK:
                // If address and function identical with last send or received request, it is a valid response
                if (MB.Address == MB.RequestAddress && MB.Function == MB.RequestFunction && MB.Address != BROADCAST_ADR) {
                    MB.Type = MODBUS_RESPONSE;
                    MB.RequestAddress = 0;
                    MB.RequestFunction = 0;
                    MB.RequestRegister = 0;
                } else {
                    MB.Type = MODBUS_REQUEST;
                    MB.RequestAddress = MB.Address;
                    MB.RequestFunction = MB.Function;
                    MB.RequestRegister = MB.Register;
                }
            default:
                break;
        }
    }
#ifdef LOG_DEBUG_MODBUS
    if(MB.Type) {
        Serial.printf(" Register %04x", MB.Register);
    }
#endif
#ifdef LOG_INFO_MODBUS
    switch (MB.Type) {
        case MODBUS_REQUEST:
            Serial.println(" Request");
            break;
        case MODBUS_RESPONSE:
            Serial.println(" Response");
            break;
    }
#endif
}



// ########################### EVSE modbus functions ###########################


/**
 * Send measurement request over modbus
 * 
 * @param uint8_t Meter
 * @param uint8_t Address
 * @param uint16_t Register
 * @param uint8_t Count
 */
void requestMeasurement(uint8_t Meter, uint8_t Address, uint16_t Register, uint8_t Count) {
    ModbusReadInputRequest(Address, EMConfig[Meter].Function, Register, (EMConfig[Meter].DataType == MB_DATATYPE_INT16 ? Count : (Count * 2u)));
}

/**
 * Decode measurement value
 * 
 * @param pointer to buf
 * @param uint8_t Count
 * @param uint8_t Endianness
 * @param MBDataType dataType
 * @param signed char Divisor
 * @return signed int Measurement
 */
signed int receiveMeasurement(uint8_t *buf, uint8_t Count, uint8_t Endianness, MBDataType dataType, signed char Divisor) {
    float dCombined;
    signed int lCombined;

    if (dataType == MB_DATATYPE_FLOAT32) {
        combineBytes(&dCombined, buf, Count * (dataType == MB_DATATYPE_INT16 ? 2u : 4u), Endianness, dataType);
        if (Divisor >= 0) {
            lCombined = (signed int)(dCombined / (signed int)pow_10[(unsigned)Divisor]);
        } else {
            lCombined = (signed int)(dCombined * (signed int)pow_10[(unsigned)-Divisor]);
        }
    } else {
        combineBytes(&lCombined, buf, Count * (dataType == MB_DATATYPE_INT16 ? 2u : 4u), Endianness, dataType);
        if (dataType == MB_DATATYPE_INT16) {
            lCombined = (signed int)((int16_t)lCombined); /* sign extend 16bit into 32bit */
        }
        if (Divisor >= 0) {
            lCombined = lCombined / (signed int)pow_10[(unsigned)Divisor];
        } else {
            lCombined = lCombined * (signed int)pow_10[(unsigned)-Divisor];
        }
    }

    return lCombined;
}

/**
 * Send Energy measurement request over modbus
 * 
 * @param uint8_t Meter
 * @param uint8_t Address
 */
void requestEnergyMeasurement(uint8_t Meter, uint8_t Address) {
   switch (Meter) {
        case EM_SOLAREDGE:
            // Note:
            // - SolarEdge uses 16-bit values, except for this measurement it uses 32bit int format
            // - EM_SOLAREDGE should not be used for EV Energy Measurements
            ModbusReadInputRequest(Address, EMConfig[Meter].Function, EMConfig[Meter].ERegister, 2);
            break;
        default:
            requestMeasurement(Meter, Address, EMConfig[Meter].ERegister, 1);
            break;
    }
}

/**
 * Read energy measurement from modbus
 * 
 * @param pointer to buf
 * @param uint8_t Meter
 * @return signed int Energy (Wh)
 */
signed int receiveEnergyMeasurement(uint8_t *buf, uint8_t Meter) {
    switch (Meter) {
        case EM_SOLAREDGE:
            // Note:
            // - SolarEdge uses 16-bit values, except for this measurement it uses 32bit int format
            // - EM_SOLAREDGE should not be used for EV Energy Measurements
            return receiveMeasurement(buf, 0, EMConfig[Meter].Endianness, MB_DATATYPE_INT32, EMConfig[Meter].EDivisor - 3);
        default:
            return receiveMeasurement(buf, 0, EMConfig[Meter].Endianness, EMConfig[Meter].DataType, EMConfig[Meter].EDivisor - 3);
    }
}

/**
 * Send Power measurement request over modbus
 * 
 * @param uint8_t Meter
 * @param uint8_t Address
 */
void requestPowerMeasurement(uint8_t Meter, uint8_t Address) {
    requestMeasurement(Meter, Address, EMConfig[Meter].PRegister, 1);
}

/**
 * Read Power measurement from modbus
 * 
 * @param pointer to buf
 * @param uint8_t Meter
 * @return signed int Power (W)
  */
signed int receivePowerMeasurement(uint8_t *buf, uint8_t Meter) {
    switch (Meter) {
        case EM_SOLAREDGE:
        {
            // Note:
            // - SolarEdge uses 16-bit values, with a extra 16-bit scaling factor
            // - EM_SOLAREDGE should not be used for EV power measurements, only PV power measurements are supported
            int scalingFactor = -(int)receiveMeasurement(
                        buf,
                        1,
                        EMConfig[Meter].Endianness,
                        EMConfig[Meter].DataType,
                        0
            );
            return receiveMeasurement(buf, 0, EMConfig[Meter].Endianness, EMConfig[Meter].DataType, scalingFactor);
        }
        default:
            return receiveMeasurement(buf, 0, EMConfig[Meter].Endianness, EMConfig[Meter].DataType, EMConfig[Meter].PDivisor);
    }
}

/**
 * Send current measurement request over modbus
 * 
 * @param uint8_t Meter
 * @param uint8_t Address
 */
void requestCurrentMeasurement(uint8_t Meter, uint8_t Address) {
    switch(Meter) {
        case EM_SENSORBOX:
            ModbusReadInputRequest(Address, 4, 0, 20);
            break;
        case EM_EASTRON:
            // Phase 1-3 current: Register 0x06 - 0x0B (unsigned)
            // Phase 1-3 power:   Register 0x0C - 0x11 (signed)
            ModbusReadInputRequest(Address, 4, 0x06, 12);
            break;
        case EM_ABB:
            // Phase 1-3 current: Register 0x5B0C - 0x5B11 (unsigned)
            // Phase 1-3 power:   Register 0x5B16 - 0x5B1B (signed)
            ModbusReadInputRequest(Address, 3, 0x5B0C, 16);
            break;
        case EM_SOLAREDGE:
            // Read 3 Current values + scaling factor
            ModbusReadInputRequest(Address, EMConfig[Meter].Function, EMConfig[Meter].IRegister, 4);
            break;
        default:
            // Read 3 Current values
            requestMeasurement(Meter, Address, EMConfig[Meter].IRegister, 3);
            break;
    }  
}

/**
 * Read current measurement from modbus
 * 
 * @param pointer to buf
 * @param uint8_t Meter
 * @param pointer to Current (mA)
 * @return uint8_t error
 */
uint8_t receiveCurrentMeasurement(uint8_t *buf, uint8_t Meter, signed int *var) {
    uint8_t x, offset;

    // No CAL option in Menu
    CalActive = 0;

    switch(Meter) {
        case EM_SENSORBOX:
            // return immediately if the data contains no new P1 or CT measurement
            if (buf[3] == 0) return 0;  // error!!
            // determine if there is P1 data present, otherwise use CT data
            if (buf[3] & 0x80) offset = 4;                                      // P1 data present
            else offset = 7;                                                    // Use CTs
            // offset 16 is Smart meter P1 current
            for (x = 0; x < 3; x++) {
                // SmartEVSE works with Amps * 10
                var[x] = receiveMeasurement(buf, offset + x, EMConfig[Meter].Endianness, EMConfig[Meter].DataType, EMConfig[Meter].IDivisor - 3u);
                // When using CT's , adjust the measurements with calibration value
                if (offset == 7) {
                    if (x == 0) Iuncal = abs((var[x] / 10));                    // Store uncalibrated CT1 measurement (10mA)
                    var[x] = var[x] * (signed int)ICal / ICAL;
                    // When MaxMains is set to >100A, it's assumed 200A:50ma CT's are used.
                    if (MaxMains > 100) var[x] = var[x] * 2;                    // Multiply measured currents with 2
                    // very small negative currents are shown as zero.
                    if ((var[x] > -1) && (var[x] < 1)) var[x] = 0;
                    CalActive = 1;                                              // Enable CAL option in Menu
                }
            }
            // Set Sensorbox 2 to 3/4 Wire configuration (and phase Rotation) (v2.16)
            if (buf[1] >= 0x10 && offset == 7) {
                GridActive = 1;                                                 // Enable the GRID menu option
                if ((buf[1] & 0x3) != (Grid << 1) && (LoadBl < 2)) ModbusWriteSingleRequest(0x0A, 0x800, Grid << 1);
            } else GridActive = 0;
            break;
        case EM_SOLAREDGE:
        {
            // Need to handle the extra scaling factor
            int scalingFactor = -(int)receiveMeasurement(
                        buf,
                        3,
                        EMConfig[Meter].Endianness,
                        EMConfig[Meter].DataType,
                        0
            );
            // Now decode the three Current values using that scaling factor
            for (x = 0; x < 3; x++) {
                var[x] = receiveMeasurement(
                        buf,
                        x,
                        EMConfig[Meter].Endianness,
                        EMConfig[Meter].DataType,
                        scalingFactor - 3
                );
            }
            break;
        }
        default:
            for (x = 0; x < 3; x++) {
                var[x] = receiveMeasurement(
                        buf,
                        x,
                        EMConfig[Meter].Endianness,
                        EMConfig[Meter].DataType,
                        EMConfig[Meter].IDivisor - 3
                );
            }
            break;
    }

    // Get sign from power measurement on some electric meters
    switch(Meter) {
        case EM_EASTRON:
            for (x = 0; x < 3; x++) {
                if (receiveMeasurement(buf, x + 3u, EMConfig[Meter].Endianness, EMConfig[Meter].DataType, EMConfig[Meter].PDivisor) < 0) var[x] = -var[x];
            }
            break;
        case EM_ABB:
            for (x = 0; x < 3; x++) {
                if (receiveMeasurement(buf, x + 5u, EMConfig[Meter].Endianness, EMConfig[Meter].DataType, EMConfig[Meter].PDivisor) < 0) var[x] = -var[x];
            }
            break;
    }

    // all OK
    return 1;
}

/**
 * Map a Modbus register to an item ID (MENU_xxx or STATUS_xxx)
 * 
 * @return uint8_t ItemID
 */
uint8_t mapModbusRegister2ItemID() {
    uint16_t RegisterStart, ItemStart, Count;

    // Register 0x00*: Status
    if (MB.Register >= MODBUS_EVSE_STATUS_START && MB.Register < (MODBUS_EVSE_STATUS_START + MODBUS_EVSE_STATUS_COUNT)) {
        RegisterStart = MODBUS_EVSE_STATUS_START;
        ItemStart = STATUS_STATE;
        Count = MODBUS_EVSE_STATUS_COUNT;

    // Register 0x01*: Node specific configuration
    } else if (MB.Register >= MODBUS_EVSE_CONFIG_START && MB.Register < (MODBUS_EVSE_CONFIG_START + MODBUS_EVSE_CONFIG_COUNT)) {
        RegisterStart = MODBUS_EVSE_CONFIG_START;
        ItemStart = MENU_CONFIG;
        Count = MODBUS_EVSE_CONFIG_COUNT;

    // Register 0x02*: System configuration (same on all SmartEVSE in a LoadBalancing setup)
    } else if (MB.Register >= MODBUS_SYS_CONFIG_START && MB.Register < (MODBUS_SYS_CONFIG_START + MODBUS_SYS_CONFIG_COUNT)) {
        RegisterStart = MODBUS_SYS_CONFIG_START;
        ItemStart = MENU_MODE;
        Count = MODBUS_SYS_CONFIG_COUNT;

    } else {
        return 0;
    }
    
    if (MB.RegisterCount <= (RegisterStart + Count) - MB.Register) {
        return (MB.Register - RegisterStart + ItemStart);
    } else {
        return 0;
    }
}

/**
 * Read item values and send modbus response
 */
void ReadItemValueResponse(void) {
    uint8_t ItemID;
    uint8_t i;
    uint16_t values[MODBUS_MAX_REGISTER_READ];

    ItemID = mapModbusRegister2ItemID();
    if (ItemID) {
        for (i = 0; i < MB.RegisterCount; i++) {
            values[i] = getItemValue(ItemID + i);
        }
        ModbusReadInputResponse(MB.Address, MB.Function, values, MB.RegisterCount);
    } else {
        ModbusException(MB.Address, MB.Function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
    }
}

/**
 * Write item values and send modbus response
 */
void WriteItemValueResponse(void) {
    uint8_t ItemID;
    uint8_t OK = 0;

    ItemID = mapModbusRegister2ItemID();
    if (ItemID) {
        OK = setItemValue(ItemID, MB.Value);
    }

    if (OK && ItemID < STATUS_STATE) write_settings();

    if (MB.Address != BROADCAST_ADR || LoadBl == 0) {
        if (!ItemID) {
            ModbusException(MB.Address, MB.Function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        } else if (!OK) {
            ModbusException(MB.Address, MB.Function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        } else {
            ModbusWriteSingleResponse(MB.Address, MB.Register, MB.Value);
        }
    }
}

/**
 * Write multiple item values and send modbus response
 */
void WriteMultipleItemValueResponse(void) {
    uint8_t ItemID;
    uint16_t i, OK = 0, value;

    ItemID = mapModbusRegister2ItemID();
    if (ItemID) {
        for (i = 0; i < MB.RegisterCount; i++) {
            value = (MB.Data[i * 2] <<8) | MB.Data[(i * 2) + 1];
            OK += setItemValue(ItemID + i, value);
        }
    }

    if (OK && ItemID < STATUS_STATE) write_settings();

    if (MB.Address != BROADCAST_ADR || LoadBl == 0) {
        if (!ItemID) {
            ModbusException(MB.Address, MB.Function, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        } else if (!OK) {
            ModbusException(MB.Address, MB.Function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        } else  {
            ModbusWriteMultipleResponse(MB.Address, MB.Register, OK);
        }
    }
}
