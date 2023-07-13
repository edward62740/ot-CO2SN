.PHONY = all clean


ifeq ($(OS),Windows_NT)
	POSIX_TOOL_PATH := C:/SiliconLabs/SimplicityStudio/v5/support/common/build/msys/1.0/bin/
endif
TOOLCHAIN_DIR := C:/SiliconLabs/SimplicityStudio/v5/developer/toolchains/gnu_arm/10.2_2020q4
SDK_PATH := C:/Users/Workstation/SimplicityStudio/SDKs/gecko_sdk_6

ARM_GCC_DIR_WIN = C:/SiliconLabs/SimplicityStudio/v5/developer/toolchains/gnu_arm/10.2_2020q4
ARM_GCC_DIR_OSX = 
ARM_GCC_DIR_LINUX = 
POST_BUILD_EXE_WIN = C:/SiliconLabs/SimplicityStudio/v5/developer/adapter_packs/commander/commander.exe
POST_BUILD_EXE_OSX = 
POST_BUILD_EXE_LINUX = 

# Pre-defined definitions in this file
ifeq ($(OS),Windows_NT)
	ARM_GCC_DIR ?= $(ARM_GCC_DIR_WIN)
	POST_BUILD_EXE ?= $(POST_BUILD_EXE_WIN)
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		ARM_GCC_DIR ?= $(ARM_GCC_DIR_OSX)
		POST_BUILD_EXE ?= $(POST_BUILD_EXE_OSX)
	else
		ARM_GCC_DIR ?= $(ARM_GCC_DIR_LINUX)
		POST_BUILD_EXE ?= $(POST_BUILD_EXE_LINUX)
	endif
endif

AR      = "$(ARM_GCC_DIR)/bin/arm-none-eabi-gcc-ar"
CC      = "$(ARM_GCC_DIR)/bin/arm-none-eabi-gcc"
CXX     = "$(ARM_GCC_DIR)/bin/arm-none-eabi-g++"
OBJCOPY = "$(ARM_GCC_DIR)/bin/arm-none-eabi-objcopy"
LD      = "$(ARM_GCC_DIR)/bin/arm-none-eabi-gcc"

# Command output is hidden by default, it can be enabled by
# setting VERBOSE=true on the commandline.
ifeq ($(VERBOSE),)
  ECHO = @
endif

PROJECTNAME := co2sn
SRC := .
OUTPUT_DIR := vscode-build
OBJ_DIR := $(OUTPUT_DIR)/obj

ASM_FLAGS := 

C_FLAGS := 

CXX_FLAGS := 
  
LD_FLAGS := 


-include co2sn.vscode.project.mak


ASM_INCLUDES := -I"config" -I"autogen" -I"." -I"$(COPIED_SDK_PATH)/platform/Device/SiliconLabs/MGM24/Include" -I"$(COPIED_SDK_PATH)/platform/common/inc" -I"$(COPIED_SDK_PATH)/platform/CMSIS/Core/Include" -I"$(COPIED_SDK_PATH)/platform/service/device_init/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/dmadrv/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/common/inc" -I"$(COPIED_SDK_PATH)/platform/emlib/inc" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/fem_util" -I"$(COPIED_SDK_PATH)/platform/emdrv/gpiointerrupt/inc" -I"$(COPIED_SDK_PATH)/platform/service/hfxo_manager/inc" -I"$(COPIED_SDK_PATH)/platform/driver/i2cspm/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc" -I"$(COPIED_SDK_PATH)/util/third_party/mbedtls/include" -I"$(COPIED_SDK_PATH)/util/third_party/mbedtls/library" -I"$(COPIED_SDK_PATH)/platform/service/mpu/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/nvm3/inc" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src/cli" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/include" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/include/openthread" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src/core" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/third_party/tcplp" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/examples/platforms" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/examples/platforms/utils" -I"$(COPIED_SDK_PATH)/protocol/openthread/platform-abstraction/efr32" -I"$(COPIED_SDK_PATH)/protocol/openthread/platform-abstraction/include" -I"$(COPIED_SDK_PATH)/platform/peripheral/inc" -I"$(COPIED_SDK_PATH)/platform/service/power_manager/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/common" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/ble" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/ieee802154" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/wmbus" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/zwave" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/chip/efr32/efr32xg2x" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/efr32xg24" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_power_manager_init" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_rssi" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/se_manager/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/se_manager/src" -I"$(COPIED_SDK_PATH)/util/plugin/security_manager" -I"$(COPIED_SDK_PATH)/util/third_party/segger/systemview/SEGGER" -I"$(COPIED_SDK_PATH)/util/silicon_labs/silabs_core/memory_manager" -I"$(COPIED_SDK_PATH)/platform/common/toolchain/inc" -I"$(COPIED_SDK_PATH)/platform/service/system/inc" -I"$(COPIED_SDK_PATH)/platform/service/sleeptimer/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_protocol_crypto/src" -I"$(COPIED_SDK_PATH)/platform/emdrv/uartdrv/inc" -I"$(COPIED_SDK_PATH)/platform/service/udelay/inc" 
C_INCLUDES := -I"config" -I"autogen" -I"." -I"$(COPIED_SDK_PATH)/platform/Device/SiliconLabs/MGM24/Include" -I"$(COPIED_SDK_PATH)/platform/common/inc" -I"$(COPIED_SDK_PATH)/platform/CMSIS/Core/Include" -I"$(COPIED_SDK_PATH)/platform/service/device_init/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/dmadrv/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/common/inc" -I"$(COPIED_SDK_PATH)/platform/emlib/inc" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/fem_util" -I"$(COPIED_SDK_PATH)/platform/emdrv/gpiointerrupt/inc" -I"$(COPIED_SDK_PATH)/platform/service/hfxo_manager/inc" -I"$(COPIED_SDK_PATH)/platform/driver/i2cspm/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc" -I"$(COPIED_SDK_PATH)/util/third_party/mbedtls/include" -I"$(COPIED_SDK_PATH)/util/third_party/mbedtls/library" -I"$(COPIED_SDK_PATH)/platform/service/mpu/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/nvm3/inc" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src/cli" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/include" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/include/openthread" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src/core" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/third_party/tcplp" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/examples/platforms" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/examples/platforms/utils" -I"$(COPIED_SDK_PATH)/protocol/openthread/platform-abstraction/efr32" -I"$(COPIED_SDK_PATH)/protocol/openthread/platform-abstraction/include" -I"$(COPIED_SDK_PATH)/platform/peripheral/inc" -I"$(COPIED_SDK_PATH)/platform/service/power_manager/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/common" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/ble" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/ieee802154" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/wmbus" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/zwave" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/chip/efr32/efr32xg2x" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/efr32xg24" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_power_manager_init" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_rssi" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/se_manager/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/se_manager/src" -I"$(COPIED_SDK_PATH)/util/plugin/security_manager" -I"$(COPIED_SDK_PATH)/util/third_party/segger/systemview/SEGGER" -I"$(COPIED_SDK_PATH)/util/silicon_labs/silabs_core/memory_manager" -I"$(COPIED_SDK_PATH)/platform/common/toolchain/inc" -I"$(COPIED_SDK_PATH)/platform/service/system/inc" -I"$(COPIED_SDK_PATH)/platform/service/sleeptimer/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_protocol_crypto/src" -I"$(COPIED_SDK_PATH)/platform/emdrv/uartdrv/inc" -I"$(COPIED_SDK_PATH)/platform/service/udelay/inc" 
CXX_INCLUDES := -I"config" -I"autogen" -I"." -I"$(COPIED_SDK_PATH)/platform/Device/SiliconLabs/MGM24/Include" -I"$(COPIED_SDK_PATH)/platform/common/inc" -I"$(COPIED_SDK_PATH)/platform/CMSIS/Core/Include" -I"$(COPIED_SDK_PATH)/platform/service/device_init/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/dmadrv/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/common/inc" -I"$(COPIED_SDK_PATH)/platform/emlib/inc" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/fem_util" -I"$(COPIED_SDK_PATH)/platform/emdrv/gpiointerrupt/inc" -I"$(COPIED_SDK_PATH)/platform/service/hfxo_manager/inc" -I"$(COPIED_SDK_PATH)/platform/driver/i2cspm/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/config/preset" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_mbedtls_support/inc" -I"$(COPIED_SDK_PATH)/util/third_party/mbedtls/include" -I"$(COPIED_SDK_PATH)/util/third_party/mbedtls/library" -I"$(COPIED_SDK_PATH)/platform/service/mpu/inc" -I"$(COPIED_SDK_PATH)/platform/emdrv/nvm3/inc" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src/cli" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/include" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/include/openthread" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/src/core" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/third_party/tcplp" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/examples/platforms" -I"$(COPIED_SDK_PATH)/util/third_party/openthread/examples/platforms/utils" -I"$(COPIED_SDK_PATH)/protocol/openthread/platform-abstraction/efr32" -I"$(COPIED_SDK_PATH)/protocol/openthread/platform-abstraction/include" -I"$(COPIED_SDK_PATH)/platform/peripheral/inc" -I"$(COPIED_SDK_PATH)/platform/service/power_manager/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_psa_driver/inc" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/common" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/ble" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/ieee802154" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/wmbus" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/protocol/zwave" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/chip/efr32/efr32xg2x" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/pa-conversions/efr32xg24" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_power_manager_init" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_pti" -I"$(COPIED_SDK_PATH)/platform/radio/rail_lib/plugin/rail_util_rssi" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/se_manager/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/se_manager/src" -I"$(COPIED_SDK_PATH)/util/plugin/security_manager" -I"$(COPIED_SDK_PATH)/util/third_party/segger/systemview/SEGGER" -I"$(COPIED_SDK_PATH)/util/silicon_labs/silabs_core/memory_manager" -I"$(COPIED_SDK_PATH)/platform/common/toolchain/inc" -I"$(COPIED_SDK_PATH)/platform/service/system/inc" -I"$(COPIED_SDK_PATH)/platform/service/sleeptimer/inc" -I"$(COPIED_SDK_PATH)/platform/security/sl_component/sl_protocol_crypto/src" -I"$(COPIED_SDK_PATH)/platform/emdrv/uartdrv/inc" -I"$(COPIED_SDK_PATH)/platform/service/udelay/inc" 

-include $(CDEPS)
-include $(CXXDEPS)
-include $(ASMDEPS_s)
-include $(ASMDEPS_S)

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ASM_SOURCES := $(call rwildcard, $(SRC), *.s) $(call rwildcard, $(SRC), *.S)
C_SOURCES := $(call rwildcard, $(SRC), *.c)
CXX_SOURCES := $(call rwildcard, $(SRC), *.cpp) $(call rwildcard, $(SRC), *.cc)

ASM_OBJECTS := $(patsubst $(SRC)/%.s, $(OBJ_DIR)/%.o, $(filter %s, $(ASM_SOURCES)))
ASM_OBJECTS += $(patsubst $(SRC)/%.S, $(OBJ_DIR)/%.o, $(filter %S, $(ASM_SOURCES)))
C_OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ_DIR)/%.o, $(filter %c, $(C_SOURCES)))
CXX_OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ_DIR)/%.o, $(filter %cpp, $(CXX_SOURCES)))
CXX_OBJECTS += $(patsubst $(SRC)/%.cc, $(OBJ_DIR)/%.o, $(filter %cc, $(CXX_SOURCES)))
PROJ_OBJECTS := $(ASM_OBJECTS)
PROJ_OBJECTS += $(C_OBJECTS)
PROJ_OBJECTS += $(CXX_OBJECTS)

ASM_FLAGS += -MP -MD
C_FLAGS += -MP -MD
CXX_FLAGS += -MP -MD

override CFLAGS = $(C_FLAGS) $(C_DEFS) $(INCLUDES) $(DEPFLAGS)
override CXXFLAGS = $(CXX_FLAGS) $(C_DEFS) $(INCLUDES) $(DEPFLAGS)
override ASMFLAGS = $(ASM_FLAGS) $(ASM_DEFS) $(INCLUDES) $(DEPFLAGS)

DEPS := $(C_OBJECTS:%.o=%.d)

all: pre-build $(OUTPUT_DIR)/$(PROJECTNAME).out post-build
	$(ECHO)$(OBJCOPY) $(OUTPUT_DIR)/$(PROJECTNAME).out -O binary $(OUTPUT_DIR)/$(PROJECTNAME).bin
	$(ECHO)$(OBJCOPY) $(OUTPUT_DIR)/$(PROJECTNAME).out -O ihex $(OUTPUT_DIR)/$(PROJECTNAME).hex
	$(ECHO)$(OBJCOPY) $(OUTPUT_DIR)/$(PROJECTNAME).out -O srec $(OUTPUT_DIR)/$(PROJECTNAME).s37

clean:
	$(POSIX_TOOL_PATH)rm -rf $(OUTPUT_DIR)

$(OUTPUT_DIR)/$(PROJECTNAME).out: $(OBJS) $(PROJ_OBJECTS) $(LIB_FILES)
	@$(POSIX_TOOL_PATH)echo 'Linking $(OUTPUT_DIR)/$(PROJECTNAME).out'
	@$(POSIX_TOOL_PATH)echo $(OBJS) > $(OUTPUT_DIR)/objs
	@$(POSIX_TOOL_PATH)echo $(PROJ_OBJECTS) > $(OUTPUT_DIR)/proj_objs
	$(ECHO)$(LD) @$(OUTPUT_DIR)/objs @$(OUTPUT_DIR)/proj_objs $(LIBS) $(LD_FLAGS) -o $(OUTPUT_DIR)/$(PROJECTNAME).out

$(OBJ_DIR)/%.o: $(SRC)/%.s
	@$(POSIX_TOOL_PATH)echo 'Building $<'
	$(ECHO)$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(ASMFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/%.o: $(SRC)/%.S
	@$(POSIX_TOOL_PATH)echo 'Building $<'
	$(ECHO)$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(ASMFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/%.o: $(SRC)/%.c
	@$(POSIX_TOOL_PATH)echo 'Building $<'
	$(ECHO)$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/%.o: $(SRC)/%.cpp
	@$(POSIX_TOOL_PATH)echo 'Building $<'
	$(ECHO)$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CXXFLAGS) -c "$<" -o "$@"

$(OBJ_DIR)/%.o: $(SRC)/%.cc
	@$(POSIX_TOOL_PATH)echo 'Building $<'
	$(ECHO)$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CXXFLAGS) -c "$<" -o "$@"

-include $(DEPS)