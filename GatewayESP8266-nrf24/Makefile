PORT = /dev/ttyUSB0
PROJECT = GatewayESP8266-nrf24
FBQN = arduino:avr:pro
UPLOADSPEED=115200

CMD = arduino-cli
COMPILE = $(CMD) compile
UPLOAD = $(CMD) upload

PROJECTBASE = /srv/src/Arduino/git/GatewayESP8266-nrf24
LIBRARIES = $(PROJECTBASE)/libraries
SRCBASE = $(PROJECTBASE)/$(PROJECT)
BUILD = $(PROJECTBASE)/build

HEX = build/$(PROJECT).ino.hex

build : $(HEX)

$(HEX) : $(SRCBASE)/*.ino $(SRCBASE)/*.h
	$(COMPILE) --verbose --build-path $(BUILD) \
	--libraries $(LIBRARIES) \
	--fqbn $(FBQN) $(PROJECT)

install : $(HEX)
	$(UPLOAD) --verbose --verify \
	--fqbn $(FBQN) -p $(PORT) $(PROJECT)

.PHONY: build

