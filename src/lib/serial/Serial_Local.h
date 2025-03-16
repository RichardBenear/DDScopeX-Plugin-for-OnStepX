// -----------------------------------------------------------------------------------
// Local Serial for sending internal commands
#pragma once

#include "../../Common.h"

#if defined(SERIAL_LOCAL_MODE) && SERIAL_LOCAL_MODE == ON

class SerialLocal : public Stream {
  public:
    inline void begin() { begin(9600); }
    void begin(long baud);
    void end();

    void setTimeout(long timeMs) { UNUSED(timeMs); }

    // Sends a command for processing
    void transmit(const char *data);

    // Receives the last command's response, if one exists
    char *receive();

    int read(void);

    inline int receiveAvailable() {
      // Take semaphore (lock)
      while (!semaphore);  // Busy-wait until available
      semaphore = false; 

      int i = 0;
      int x = xmit_index;
      while (xmit_buffer[xmit_index] != 0) {
          i++;
          xmit_index++;
          xmit_index &= 0b1111111;
      }
      xmit_index = x;

      // Give semaphore (unlock)
      semaphore = true;  
      return i;
    }

    inline int available(void) { 
      // Take semaphore (lock)
      while (!semaphore);  // Busy-wait until available
      semaphore = false;

      uint8_t p = recv_head;
      uint8_t c = 0;
      while (recv_buffer[p] && p != recv_tail && c < 255) { 
          c++; 
          p++; 
      }

      // Give semaphore (unlock)
      semaphore = true;  
      return c;
    }

    inline int peek(void) {
      if (!available()) return -1;

      // Take semaphore (lock)
      while (!semaphore);  
      semaphore = false;  

      char c = recv_buffer[recv_head];

      // Give semaphore (unlock)
      semaphore = true;  
      return c;
    }

    inline void flush(void) {
      while (receiveAvailable()) { yield(); }
    }
  
    inline size_t write(uint8_t data) {
      // Take semaphore (lock)
      while (!semaphore);  
      semaphore = false;  

      xmit_buffer[xmit_tail] = data;
      xmit_tail++;
      xmit_tail &= 0b1111111;
      xmit_buffer[xmit_tail] = 0;

      // Give semaphore (unlock)
      semaphore = true;  
      return 1;
    }

    size_t write(const uint8_t* data, size_t count) {
      for (int i = 0; i < (int)count; i++) { 
          if (!write(data[i])) return i; 
      }
      return count;
    }

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }

    using Print::write;
    volatile bool semaphore = true;  // Simple semaphore for thread safety

  private:
    char recv_buffer[256] = "";
    uint8_t recv_head = 0;
    uint8_t recv_tail = 0;

    char xmit_buffer[128] = "";
    char xmit_result[128] = "";
    uint8_t xmit_index = 0;
    uint8_t xmit_tail = 0;

    
};

extern SerialLocal serialLocal;

#define SERIAL_LOCAL serialLocal

#endif
