# groups.cmake

# group Application/MDK-ARM
add_library(Group_Application_MDK-ARM OBJECT
  "${SOLUTION_ROOT}/startup_stm32h743xx.s"
)
target_include_directories(Group_Application_MDK-ARM PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Group_Application_MDK-ARM PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
add_library(Group_Application_MDK-ARM_ABSTRACTIONS INTERFACE)
target_link_libraries(Group_Application_MDK-ARM_ABSTRACTIONS INTERFACE
  ${CONTEXT}_ABSTRACTIONS
)
target_compile_options(Group_Application_MDK-ARM PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_Application_MDK-ARM PUBLIC
  Group_Application_MDK-ARM_ABSTRACTIONS
)
set(COMPILE_DEFINITIONS
  __MICROLIB
  STM32H743xx
  _RTE_
)
cbuild_set_defines(AS_ARM COMPILE_DEFINITIONS)
set_source_files_properties("${SOLUTION_ROOT}/startup_stm32h743xx.s" PROPERTIES
  COMPILE_FLAGS "${COMPILE_DEFINITIONS}"
)
set_source_files_properties("${SOLUTION_ROOT}/startup_stm32h743xx.s" PROPERTIES
  COMPILE_OPTIONS "-masm=auto;-x;assembler-with-cpp"
)

# group Application/User/Core
add_library(Group_Application_User_Core OBJECT
  "${SOLUTION_ROOT}/../Core/Src/main.c"
  "${SOLUTION_ROOT}/../Core/Src/gpio.c"
  "${SOLUTION_ROOT}/../Core/Src/adc.c"
  "${SOLUTION_ROOT}/../Core/Src/dma.c"
  "${SOLUTION_ROOT}/../Core/Src/tim.c"
  "${SOLUTION_ROOT}/../Core/Src/usart.c"
  "${SOLUTION_ROOT}/../Core/Src/stm32h7xx_it.c"
  "${SOLUTION_ROOT}/../Core/Src/stm32h7xx_hal_msp.c"
)
target_include_directories(Group_Application_User_Core PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Group_Application_User_Core PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
add_library(Group_Application_User_Core_ABSTRACTIONS INTERFACE)
target_link_libraries(Group_Application_User_Core_ABSTRACTIONS INTERFACE
  ${CONTEXT}_ABSTRACTIONS
)
target_compile_options(Group_Application_User_Core PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_Application_User_Core PUBLIC
  Group_Application_User_Core_ABSTRACTIONS
)

# group Drivers/STM32H7xx_HAL_Driver
add_library(Group_Drivers_STM32H7xx_HAL_Driver OBJECT
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c"
  "${SOLUTION_ROOT}/../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c"
)
target_include_directories(Group_Drivers_STM32H7xx_HAL_Driver PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Group_Drivers_STM32H7xx_HAL_Driver PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
add_library(Group_Drivers_STM32H7xx_HAL_Driver_ABSTRACTIONS INTERFACE)
target_link_libraries(Group_Drivers_STM32H7xx_HAL_Driver_ABSTRACTIONS INTERFACE
  ${CONTEXT}_ABSTRACTIONS
)
target_compile_options(Group_Drivers_STM32H7xx_HAL_Driver PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_Drivers_STM32H7xx_HAL_Driver PUBLIC
  Group_Drivers_STM32H7xx_HAL_Driver_ABSTRACTIONS
)

# group Drivers/CMSIS
add_library(Group_Drivers_CMSIS OBJECT
  "${SOLUTION_ROOT}/../Core/Src/system_stm32h7xx.c"
)
target_include_directories(Group_Drivers_CMSIS PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(Group_Drivers_CMSIS PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
add_library(Group_Drivers_CMSIS_ABSTRACTIONS INTERFACE)
target_link_libraries(Group_Drivers_CMSIS_ABSTRACTIONS INTERFACE
  ${CONTEXT}_ABSTRACTIONS
)
target_compile_options(Group_Drivers_CMSIS PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_Drivers_CMSIS PUBLIC
  Group_Drivers_CMSIS_ABSTRACTIONS
)

# group App
add_library(Group_App OBJECT
  "${SOLUTION_ROOT}/../App/fft/fft_analyzer.c"
  "${SOLUTION_ROOT}/../App/measure/freq_measure.c"
  "${SOLUTION_ROOT}/../App/dds/ad9910.c"
  "${SOLUTION_ROOT}/../App/dds/ad9959.c"
  "${SOLUTION_ROOT}/../App/task/adc_task.c"
  "${SOLUTION_ROOT}/../App/comm/hmi_comm.c"
  "${SOLUTION_ROOT}/../App/utils/delay.c"
)
target_include_directories(Group_App PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  "${SOLUTION_ROOT}/../App"
  "${SOLUTION_ROOT}/../App/fft"
  "${SOLUTION_ROOT}/../App/measure"
  "${SOLUTION_ROOT}/../App/dds"
  "${SOLUTION_ROOT}/../App/task"
  "${SOLUTION_ROOT}/../App/comm"
  "${SOLUTION_ROOT}/../App/utils"
)
target_compile_definitions(Group_App PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
add_library(Group_App_ABSTRACTIONS INTERFACE)
target_link_libraries(Group_App_ABSTRACTIONS INTERFACE
  ${CONTEXT}_ABSTRACTIONS
)
target_compile_options(Group_App PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_App PUBLIC
  Group_App_ABSTRACTIONS
)

# group long_fft
add_library(Group_long_fft OBJECT
  "${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_bitreversal2_extra.c"
  "${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_cfft_f32_extra.c"
  "${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_cfft_init_f32_extra.c"
  "${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_common_tables_extra.c"
  "${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_const_structs_extra.c"
  "${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_rfft_fast_f32_extra.c"
  "${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_rfft_fast_init_f32_extra.c"
)
target_include_directories(Group_long_fft PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_INCLUDE_DIRECTORIES>
  "${SOLUTION_ROOT}/../SignalProcess/long_fft"
)
target_compile_definitions(Group_long_fft PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_DEFINITIONS>
)
add_library(Group_long_fft_ABSTRACTIONS INTERFACE)
target_link_libraries(Group_long_fft_ABSTRACTIONS INTERFACE
  ${CONTEXT}_ABSTRACTIONS
)
target_compile_options(Group_long_fft PUBLIC
  $<TARGET_PROPERTY:${CONTEXT},INTERFACE_COMPILE_OPTIONS>
)
target_link_libraries(Group_long_fft PUBLIC
  Group_long_fft_ABSTRACTIONS
  ${SOLUTION_ROOT}/../SignalProcess/long_fft/arm_cortexM7l_math.lib
)
