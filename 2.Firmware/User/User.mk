# 模块名_DIR 是上一层传递下来的参数，
# 是从工程根目录到该模块文件夹的路径

# 向 C_SOURCES 中添加需要编译的源文件
C_SOURCES += $(wildcard $(User_DIR)/easylogger/src/*.c)
C_SOURCES += $(wildcard $(User_DIR)/easylogger/port/*.c)

C_SOURCES += $(wildcard $(User_DIR)/oled/*.c)

C_SOURCES += $(wildcard $(User_DIR)/GUI/*.c)

C_SOURCES += $(wildcard $(User_DIR)/EC11/*.c)
# 向 C_INCLUDES 中添加头文件路径
C_INCLUDES += -I$(User_DIR)/easylogger/inc
C_INCLUDES += -I$(User_DIR)/OLED
C_INCLUDES += -I$(User_DIR)/GUI
C_INCLUDES += -I$(User_DIR)/EC11