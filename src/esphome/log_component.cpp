#include "esphome/log_component.h"

#ifdef ARDUINO_ARCH_ESP32
#include <esp_log.h>
#endif
#include <HardwareSerial.h>

#include "esphome/log.h"

ESPHOME_NAMESPACE_BEGIN

static const char *TAG = "logger";

int HOT LogComponent::log_vprintf_(int level, const char *tag, const char *format, va_list args) {  // NOLINT
  if (level > this->level_for(tag))
    return 0;

  int ret = vsnprintf(this->tx_buffer_.data(), this->tx_buffer_.capacity(), format, args);
  this->log_message_(level, tag, this->tx_buffer_.data(), ret);
  return ret;
}
#ifdef USE_STORE_LOG_STR_IN_FLASH
int LogComponent::log_vprintf_(int level, const char *tag, const __FlashStringHelper *format, va_list args) {  // NOLINT
  if (level > this->level_for(tag))
    return 0;

  // copy format string
  const char *format_pgm_p = (PGM_P) format;
  size_t len = 0;
  char *write = this->tx_buffer_.data();
  char ch = '.';
  while (len < this->tx_buffer_.capacity() && ch != '\0') {
    *write++ = ch = pgm_read_byte(format_pgm_p++);
    len++;
  }
  if (len == this->tx_buffer_.capacity())
    return -1;

  // now apply vsnprintf
  size_t offset = len + 1;
  size_t remaining = this->tx_buffer_.capacity() - offset;
  char *msg = this->tx_buffer_.data() + offset;
  int ret = vsnprintf(msg, remaining, this->tx_buffer_.data(), args);
  this->log_message_(level, tag, msg, ret);
  return ret;
}
#endif

int HOT LogComponent::level_for(const char *tag) {
  // Uses std::vector<> for low memory footprint, though the vector
  // could be sorted to minimize lookup times. This feature isn't used that
  // much anyway so it doesn't matter too much.
  for (auto &it : this->log_levels_) {
    if (it.tag == tag) {
      return it.level;
    }
  }
  return this->global_log_level_;
}
void HOT LogComponent::log_message_(int level, const char *tag, char *msg, int ret) {
  if (ret <= 0)
    return;
  // remove trailing newline
  if (msg[ret - 1] == '\n') {
    msg[ret - 1] = '\0';
  }
  if (m_uart)
    this->m_uart->println(msg);
  this->log_callback_.call(level, tag, msg);
}

LogComponent::LogComponent(UARTDevice* uart, size_t tx_buffer_size)
    : m_uart{uart} {
  this->set_tx_buffer_size(tx_buffer_size);
}

void LogComponent::pre_setup() {
  global_log_component = this;
  esp_log_set_vprintf(esp_idf_log_vprintf_);
  if (this->global_log_level_ >= ESPHOME_LOG_LEVEL_VERBOSE) {
    esp_log_level_set("*", ESP_LOG_VERBOSE);
  }

  ESP_LOGI(TAG, "Log initialized");
}
void LogComponent::set_global_log_level(int log_level) { this->global_log_level_ = log_level; }
void LogComponent::set_log_level(const std::string &tag, int log_level) {
  this->log_levels_.push_back(LogLevelOverride{tag, log_level});
}
size_t LogComponent::get_tx_buffer_size() const { return this->tx_buffer_.capacity(); }
void LogComponent::set_tx_buffer_size(size_t tx_buffer_size) { this->tx_buffer_.reserve(tx_buffer_size); }
UARTDevice* LogComponent::get_uart() const { return this->m_uart; }
void LogComponent::add_on_log_callback(std::function<void(int, const char *, const char *)> &&callback) {
  this->log_callback_.add(std::move(callback));
}
float LogComponent::get_setup_priority() const { return setup_priority::HARDWARE - 1.0f; }
const char *LOG_LEVELS[] = {"NONE", "ERROR", "WARN", "INFO", "DEBUG", "VERBOSE", "VERY_VERBOSE"};
#ifdef ARDUINO_ARCH_ESP32
const char *UART_SELECTIONS[] = {"UART0", "UART1", "UART2"};
#endif
#ifdef ARDUINO_ARCH_ESP8266
const char *UART_SELECTIONS[] = {"UART0", "UART1", "UART0_SWAP"};
#endif
void LogComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Logger:");
  ESP_LOGCONFIG(TAG, "  Level: %s", LOG_LEVELS[this->global_log_level_]);
  for (auto &it : this->log_levels_) {
    ESP_LOGCONFIG(TAG, "  Level for '%s': %s", it.tag.c_str(), LOG_LEVELS[it.level]);
  }
}
int LogComponent::get_global_log_level() const { return this->global_log_level_; }

LogComponent *global_log_component = nullptr;

ESPHOME_NAMESPACE_END
