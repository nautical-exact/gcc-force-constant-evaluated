PLUGIN_NAME := gcc-force-constant-evaluated

ifdef COMMON_GCC

ifdef HOST_GCC
$(error Error: Defined both COMMON_GCC and HOST_GCC)
endif

ifdef TARGET_GCC
$(error Error: Defined both COMMON_GCC and TARGET_GCC)
endif

HOST_GCC = $(COMMON_GCC)
TARGET_GCC = $(COMMON_GCC)

else # COMMON_GCC is not defined

ifndef HOST_GCC
$(warning Warning: HOST_GCC not defined, using default (g++))
HOST_GCC := g++
endif

ifndef TARGET_GCC
$(warning Warning: TARGET_GCC not defined, using default (g++))
TARGET_GCC := g++
endif

endif # COMMON_GCC

GCC_PLUGINS_DIR = $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS += -I"$(GCC_PLUGINS_DIR)/include" -fPIC -fno-rtti -std=c++11
CXXFLAGS += -Wall -Wextra -Werror

ROOT_DIR := $(shell dirname "$(realpath "$(firstword $(MAKEFILE_LIST))")")

.PHONY: test

$(PLUGIN_NAME).so: plugin.cpp
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

test: $(PLUGIN_NAME).so
	$(TARGET_GCC) test/*.cpp -fplugin="$(ROOT_DIR)/$(PLUGIN_NAME).so" -o test.out
