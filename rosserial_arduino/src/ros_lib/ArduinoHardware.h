/* 
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2011, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of Willow Garage, Inc. nor the names of its
 *    contributors may be used to endorse or promote prducts derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ROS_ARDUINO_HARDWARE_H_
#define ROS_ARDUINO_HARDWARE_H_

#if ARDUINO>=100
  #include <Arduino.h>  // Arduino 1.0
#else
  #include <WProgram.h>  // Arduino 0022
#endif

#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__MKL26Z64__)
  #if defined(USE_TEENSY_HW_SERIAL)
    #define SERIAL_CLASS HardwareSerial // Teensy HW Serial
  #else
    #include <usb_serial.h>  // Teensy 3.0 and 3.1
    #define SERIAL_CLASS usb_serial_class
  #endif
#elif defined(_SAM3XA_)
  #include <UARTClass.h>  // Arduino Due
  #define SERIAL_CLASS UARTClass
#elif defined(USE_USBCON)
  // Arduino Leonardo USB Serial Port
  #define SERIAL_CLASS Serial_
#elif (defined(__STM32F1__) and !(defined(USE_STM32_HW_SERIAL))) or defined(SPARK) 
  // Stm32duino Maple mini USB Serial Port
  #define SERIAL_CLASS USBSerial
#else 
  #include <HardwareSerial.h>  // Arduino AVR
  #define SERIAL_CLASS HardwareSerial
#endif

#include <util/atomic.h>

class ArduinoHardware {
  public:
    ArduinoHardware(SERIAL_CLASS* io , long baud= 57600){
      iostream = io;
      baud_ = baud;
      micros_offset_ = 0ULL;
      micros_previous_ = 0UL;
    }
    ArduinoHardware()
    {
#if defined(USBCON) and !(defined(USE_USBCON))
      /* Leonardo support */
      iostream = &Serial1;
#elif defined(USE_TEENSY_HW_SERIAL) or defined(USE_STM32_HW_SERIAL)
      iostream = &Serial1;
#else
      iostream = &Serial;
#endif
      baud_ = 250000;
      micros_offset_ = 0ULL;
      micros_previous_ = 0UL;
    }
    ArduinoHardware(ArduinoHardware& h){
      this->iostream = h.iostream;
      this->baud_ = h.baud_;
      micros_offset_ = 0ULL;
      micros_previous_ = 0UL;
    }
  
    void setBaud(long baud){
      this->baud_= baud;
    }
  
    int getBaud(){return baud_;}

    void init(){
#if defined(USE_USBCON)
      // Startup delay as a fail-safe to upload a new sketch
      delay(3000); 
#endif
      iostream->begin(baud_);
    }

    int read(){return iostream->read();};
    void write(uint8_t* data, int length){
      for(int i=0; i<length; i++)
        iostream->write(data[i]);
    }

    unsigned long time(){return millis();}
    uint64_t time_micros() {
      uint32_t micros_current = micros();
      ATOMIC_BLOCK(ATOMIC_FORCEON) {
        static_assert(sizeof(unsigned long long) == sizeof(uint64_t),
            "Size of unsigned long long is not equal to uint64_t.");
        static_assert(sizeof(unsigned long) == sizeof(uint32_t),
            "Size of unsigned long is not equal to uint32_t.");

        // Make sure that the difference actually is the result of an overflow of
        // the micros variable and not something else.
        if (micros_previous_ > micros_current) {
          micros_offset_ += 4294967296;
        }
        micros_previous_ = micros_current;
      }
      return micros_current + micros_offset_;
    }

  protected:
    SERIAL_CLASS* iostream;
    long baud_;
    uint64_t micros_offset_;
    uint32_t micros_previous_;
};

#endif
