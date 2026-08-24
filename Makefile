#---------------------------------------------------------------------------------
# Makefile para Nintendo Switch Homebrew (libnx)
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Por favor configura DEVKITPRO en tu entorno. Exporta DEVKITPRO=<path a devkitpro>")
endif

TOPDIR ?= $(CURDIR)

export DEVKITPRO ?= /opt/devkitpro
export DEVKITA64 := $(DEVKITPRO)/devkitA64
export LIBNX     := $(DEVKITPRO)/libnx
export PORTLIBS  := $(DEVKITPRO)/portlibs/switch
export PREFIX    := aarch64-none-elf-
export CC        := $(PREFIX)gcc
export CXX       := $(PREFIX)g++
export LD        := $(PREFIX)gcc
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

export CFLAGS   := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
export CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
export ASFLAGS  := -g $(ARCH)

export LDFLAGS  := -specs=$(LIBNX)/switch.specs -g $(ARCH) -Wl,-Map,$(TARGET).map

LIBS        := -lnx

export OUTPUT   := $(CURDIR)/$(TARGET)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

# Definir CFILES y OFILES para ambos entornos (top-level y sub-make)
CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.s)))
BINFILES    := $(foreach dir,$(DATA),$(notdir $(wildcard $(TOPDIR)/$(dir)/*)))

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(TOPDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export CPPFLAGS += $(INCLUDE)
export CFLAGS   += $(INCLUDE)
export CXXFLAGS += $(INCLUDE)

ifneq ($(BUILD),$(notdir $(CURDIR)))

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
	@echo "Enlazando $(notdir $@)..."
	$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

-include $(DEPENDS)

endif
