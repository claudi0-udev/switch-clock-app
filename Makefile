#---------------------------------------------------------------------------------
# Makefile para Nintendo Switch Homebrew (libnx)
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Por favor configura DEVKITPRO en tu entorno. Exporta DEVKITPRO=<path a devkitpro>")
endif

TOPDIR ?= $(CURDIR)

export DEVKITPRO
export LIBNX     := $(DEVKITPRO)/libnx
export PORTLIBS  := $(DEVKITPRO)/portlibs/switch
export LIBDIRS   := $(PORTLIBS) $(LIBNX)

include $(LIBNX)/switch_rules

# Metadatos de la Aplicacion
TARGET      := SwitchClock
BUILD       := build
SOURCES     := src
DATA        := data
INCLUDES    := include
EXEFS_SRC   := exefs_src

APP_TITLE   := Switch Clock Suite
APP_AUTHOR  := Antigravity Homebrew
APP_VERSION := 1.0.0

# Banderas de compilacion
ARCH        := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS      := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS     := -g $(ARCH)

LDFLAGS     = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(ver_dir)/$(TARGET).map

LIBS        := -lnx

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES    := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*)))

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

# Agregar banderas de include a CFLAGS y CXXFLAGS para la compilacion de objetos
CFLAGS   += $(INCLUDE)
CXXFLAGS += $(INCLUDE)

.PHONY: all clean

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo "Limpiando archivos compilados..."
	@rm -rf $(BUILD) $(TARGET).nro $(TARGET).elf

else

DEPENDS := $(OFILES:.o=.d)

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
