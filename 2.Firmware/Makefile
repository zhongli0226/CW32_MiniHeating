# 项目编译目标名
TARGET = MiniHeating

# 调试信息
DEBUG = 1
# 优化等级
OPT = -Og
# 链接时优化
LTO = -flto


# 编译临时文件目录
BUILD_DIR = build
EXEC_DIR = build_exec

# 模块导入

Core_DIR = Core
include Core/Core.mk

Libraries_DIR = Libraries
include Libraries/Libraries.mk

User_DIR = User
include User/User.mk


#######################################
# 编译器指定
#######################################
PREFIX = arm-none-eabi-
# 启用下一项以指定GCC目录
# GCC_PATH = 
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
DUMP = $(GCC_PATH)/$(PREFIX)objdump
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
DUMP = $(PREFIX)objdump
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# 目标单片机配置信息
#######################################
# cpu
CPU = -mcpu=cortex-m0plus

# fpu
FPU = #none

# float-abi
FLOAT-ABI = #none

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections 

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################

# C源文件宏定义
# C_DEFS += -D

# C头文件目录
C_INCLUDES +=

# C源文件
C_SOURCES +=

# 向 LIBDIR 中添加静态库文件路径
# LIBDIR += -L$(Libraries_DIR)/Lib
# 向 LIBS 中添加需要链接的静态库
# LIBS += -lxxxx

# link script
LDSCRIPT = CW32F030x_FLASH.ld

# 汇编文件宏定义
AS_DEFS += 

# 汇编头文件目录
AS_INCLUDES += 

# 汇编源文件（starup）
ASM_SOURCES += startup_cw32f030_gcc.s

# 链接库
LIBS += -lc -lm -lnosys 
# 库文件路径
LIBDIR += 

# libraries
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
-Wl,--gc-sections -ffunction-sections --specs=nano.specs --specs=nosys.specs $(LTO) 

# # printf float 打印浮点数
# LDFLAGS += -lc -lrdimon -u _printf_float

# cStandard
CMODE = c99
# cppStandard
CPPMODE = c++11

# default action: build all
all: $(EXEC_DIR)/$(TARGET).elf $(EXEC_DIR)/$(TARGET).hex $(EXEC_DIR)/$(TARGET).bin POST_BUILD

#######################################
# build the application
#######################################
# list of objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo "[CC]    $< -> $@"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@ -std=$(CMODE)

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "[AS]    $< -> $@"
	@$(AS) -c $(CFLAGS) $< -o $@

$(EXEC_DIR)/$(TARGET).elf: $(OBJECTS) Makefile | $(EXEC_DIR)
	@echo "[LD]    $(OBJECTS) -> $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(EXEC_DIR)/%.hex: $(EXEC_DIR)/%.elf | $(EXEC_DIR)
	@echo "[HEX]   $< -> $@"
	@$(HEX) $< $@
	
$(EXEC_DIR)/%.bin: $(EXEC_DIR)/%.elf | $(EXEC_DIR)
	@echo "[BIN]   $< -> $@"
	@$(BIN) $< $@
	
$(BUILD_DIR):
	@mkdir $@

$(EXEC_DIR):
	@mkdir $@

.PHONY: POST_BUILD
POST_BUILD: $(EXEC_DIR)/$(TARGET).elf
ifeq ($(DEBUG), 1)
	@echo "[DUMP]  $< -> $(EXEC_DIR)/$(TARGET).s"
	@$(DUMP) -d $< > $(EXEC_DIR)/$(TARGET).s
endif
	@echo "[SIZE]  $<"
	@$(SZ) $<
	@echo -e "------Build Finish------"

#######################################
# 清除临时文件
#######################################
.PHONY: clean
clean:
	-del $(BUILD_DIR)
	@echo -e "------Clean Build Finish------"

.PHONY: cleanall
cleanall: clean
	-del $(EXEC_DIR)
	@echo -e "------Clean Exec Finish------"

# #######################################
# # 烧录程序
# #######################################
# .PHONY: flash
# flash: $(EXEC_DIR)/$(TARGET).elf
# 	@echo -e "$(OK_COLOR)Start pyOCD$(NO_COLOR)"
# 	@pyocd flash $<

# #######################################
# # 构建并烧录程序
# #######################################
# .PHONY: run
# run:
# 	@make -j
# 	@make flash
  
#######################################
# 依赖文件
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***