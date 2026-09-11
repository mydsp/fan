cmake_minimum_required(VERSION 3.22)
set(FAN_MCU "f401" CACHE STRING "f401 or f103")
string(TOLOWER "${FAN_MCU}" FAN_MCU_LOWER)
if(NOT FAN_MCU_LOWER STREQUAL "f401" AND NOT FAN_MCU_LOWER STREQUAL "f103")
  message(FATAL_ERROR "FAN_MCU must be f401 or f103, got ${FAN_MCU}")
endif()

if(FAN_MCU_LOWER STREQUAL "f103")
  set(FW_DIR "E:/Stm32cube/repository/STM32Cube_FW_F1_V1.8.5")
else()
  set(FW_DIR "C:/Users/L/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0")
endif()
project(fan C CXX ASM)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(FAN_MCU_LOWER STREQUAL "f103")
  include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
    ${FW_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${FW_DIR}/Drivers/CMSIS/Include
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3
  )
  add_definitions(-DUSE_HAL_DRIVER -DSTM32F103xB -DSYSCLK_FREQ_72MHz)
else()
  include_directories(
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${FW_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${FW_DIR}/Drivers/CMSIS/Include
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
  )
  add_definitions(-DUSE_HAL_DRIVER -DSTM32F401xE)
endif()

file(GLOB APP_SOURCES "${CMAKE_SOURCE_DIR}/Core/Src/*.c")
if(FAN_MCU_LOWER STREQUAL "f103")
  list(FILTER APP_SOURCES EXCLUDE REGEX "stm32f4xx_hal_msp\.c|stm32f4xx_it\.c|system_stm32f4xx\.c")
  set(STARTUP_SOURCES
    ${FW_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s
    ${FW_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
  )
  set(HAL_SOURCES
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
    ${FW_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
  )
  set(FREERTOS_SOURCES
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/list.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c
  )
  set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld)
else()
  list(FILTER APP_SOURCES EXCLUDE REGEX "stm32f1xx_hal_msp\.c|stm32f1xx_it\.c")
  set(STARTUP_SOURCES ${CMAKE_SOURCE_DIR}/Core/Src/startup_stm32f401xe.s)
  set(HAL_SOURCES
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
    ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
  )
  set(FREERTOS_SOURCES
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/timers.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/list.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ${FW_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
  )
  set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F401RETx_FLASH.ld)
endif()

add_executable(${CMAKE_PROJECT_NAME}
  ${APP_SOURCES}
  ${HAL_SOURCES}
  ${FREERTOS_SOURCES}
  ${STARTUP_SOURCES}
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
  -T${LINKER_SCRIPT}
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_PROJECT_NAME}.map
  --specs=nano.specs --specs=nosys.specs
  -Wl,-u,__errno
  -Wl,--print-memory-usage
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex  $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
  COMMENT "Generating hex/bin and size"
)
