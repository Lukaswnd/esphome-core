#ifndef ESPHOME_UART_COMPONENT_H
#define ESPHOME_UART_COMPONENT_H

#include "esphome/defines.h"

#ifdef USE_UART

#include <HardwareSerial.h>
#include "esphome/component.h"

#include "esphome/helpers.h"

#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3
#include <HWCDC.h>
#else
class HWCDC {};
#endif

ESPHOME_NAMESPACE_BEGIN

class UARTComponent : public Component, public Stream {
 public:
  virtual void write_byte(uint8_t data) = 0;

  virtual void write_array(const uint8_t *data, size_t len) = 0;

  virtual void write_str(const char *str) = 0;

  virtual bool peek_byte(uint8_t *data) = 0;

  virtual bool read_byte(uint8_t *data) = 0;

  virtual bool read_array(uint8_t *data, size_t len) = 0;

 protected:
  virtual bool check_read_timeout_(size_t len = 1) = 0;
};

template<class SerialClass, typename = std::enable_if_t<std::disjunction_v<std::is_same<SerialClass, HardwareSerial>,
                                                                           std::is_same<SerialClass, HWCDC>>>>
class UARTComponent_IMPL : public UARTComponent {
 public:
  UARTComponent_IMPL(SerialClass &serial) : m_serial(serial) {}

  void setup() override {}

  void dump_config() override {}

  void write_byte(uint8_t data) override {
    this->m_serial->write(data);
    // ESP_LOGVV(TAG, "    Wrote 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data), data);
  }

  void write_array(const uint8_t *data, size_t len) override {
    this->m_serial->write(data, len);
    for (size_t i = 0; i < len; i++) {
      // ESP_LOGVV(TAG, "    Wrote 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data[i]), data[i]);
    }
  }

  void write_str(const char *str) override {
    this->m_serial->write(str);
    // ESP_LOGVV(TAG, "    Wrote \"%s\"", str);
  }

  bool peek_byte(uint8_t *data) override {
    if (!this->check_read_timeout_())
      return false;
    *data = this->m_serial->peek();
    return true;
  }

  bool read_byte(uint8_t *data) override {
    if (!this->check_read_timeout_())
      return false;
    *data = this->m_serial->read();
    // ESP_LOGVV(TAG, "    Read 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(*data), *data);
    return true;
  }

  bool read_array(uint8_t *data, size_t len) override {
    if (!this->check_read_timeout_(len))
      return false;
    this->m_serial->readBytes(data, len);
    for (size_t i = 0; i < len; i++) {
      // ESP_LOGVV(TAG, "    Read 0b" BYTE_TO_BINARY_PATTERN " (0x%02X)", BYTE_TO_BINARY(data[i]), data[i]);
    }

    return true;
  }

  int available() override { return this->m_serial->available(); }

  void flush() override {
    // ESP_LOGVV(TAG, "    Flushing...");
    this->serial->flush();
  }

  float get_setup_priority() const override { return setup_priority::PRE_HARDWARE; }

  size_t write(uint8_t data) override {
    this->write_byte(data);
    return 1;
  }
  int read() override {
    uint8_t data;
    if (!this->read_byte(&data))
      return -1;
    return data;
  }
  int peek() override {
    uint8_t data;
    if (!this->peek_byte(&data))
      return -1;
    return data;
  }

 protected:
  bool check_read_timeout_(size_t len = 1) override {
    if (this->available() >= len)
      return true;

    uint32_t start_time = millis();
    while (this->available() < len) {
      if (millis() - start_time > 1000) {
        // ESP_LOGE(TAG, "Reading from UART timed out at byte %u!", this->available());
        return false;
      }
    }
    return true;
  }
  SerialClass &m_serial;
};

#ifdef ARDUINO_ARCH_ESP32
extern uint8_t next_uart_num;
#endif

class UARTDevice : public Stream {
 public:
  UARTDevice(UARTComponent *parent);

  void write_byte(uint8_t data);

  void write_array(const uint8_t *data, size_t len);

  void write_str(const char *str);

  bool read_byte(uint8_t *data);
  bool peek_byte(uint8_t *data);

  bool read_array(uint8_t *data, size_t len);

  int available() override;

  void flush() override;

  size_t write(uint8_t data) override;
  int read() override;
  int peek() override;

 protected:
  UARTComponent *parent_;
};

ESPHOME_NAMESPACE_END

#endif  // USE_UART

#endif  // ESPHOME_UART_COMPONENT_H
