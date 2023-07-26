# 模块名_DIR 是上一层传递下来的参数，
# 是从工程根目录到该模块文件夹的路径

# 向 C_SOURCES 中添加需要编译的源文件
C_SOURCES += $(wildcard $(Libraries_DIR)/CW32F030x_StdLib/src/*.c)

# 向 C_INCLUDES 中添加头文件路径
C_INCLUDES += -I$(Libraries_DIR)/CMSIS/Include
C_INCLUDES += -I$(Libraries_DIR)/CW32F030x_StdLib/inc

