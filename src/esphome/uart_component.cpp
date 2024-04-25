#include "esphome/defines.h"

#ifdef USE_UART

#include "esphome/uart_component.h"
#include "esphome/helpers.h"
#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

static const char *TAG = "uart";

#ifdef ARDUINO_ARCH_ESP32
uint8_t next_uart_num = 1;
#endif




void UARTDevice::write_byte(uint8_t data) { this->parent_->write_byte(data); }
void UARTDevice::write_array(const uint8_t *data, size_t len) { this->parent_->write_array(data, len); }
void UARTDevice::write_str(const char *str) { this->parent_->write_str(str); }
bool UARTDevice::read_byte(uint8_t *data) { return this->parent_->read_byte(data); }
bool UARTDevice::peek_byte(uint8_t *data) { return this->parent_->peek_byte(data); }
bool UARTDevice::read_array(uint8_t *data, size_t len) { return this->parent_->read_array(data, len); }
int UARTDevice::available() { return this->parent_->available(); }
void UARTDevice::flush() { return this->parent_->flush(); }
UARTDevice::UARTDevice(UARTComponent *parent) : parent_(parent) {}
size_t UARTDevice::write(uint8_t data) { return this->parent_->write(data); }
int UARTDevice::read() { return this->parent_->read(); }
int UARTDevice::peek() { return this->parent_->peek(); }

ESPHOME_NAMESPACE_END

#endif  // USE_UART
