PORT = /dev/ttyUSB0
#IP = 192.168.42.140
PROJECT = EEProm
FBQN = arduino:avr:pro
#URL = http://$(IP)/console/baud
UPLOADSPEED=115200

CMD = arduino-cli
COMPILE = $(CMD) compile
UPLOAD = $(CMD) upload

PROJECTBASE = /srv/src/Arduino/git/mysensors-eeprom
LIBRARIES = $(PROJECTBASE)/libraries
SRCBASE = $(PROJECTBASE)/$(PROJECT)
BUILD = $(PROJECTBASE)/build

HEX = build/$(PROJECT).ino.hex

build : $(HEX)

$(HEX) : $(SRCBASE)/*.ino
	$(COMPILE) --verbose --build-path $(BUILD) \
	--libraries $(LIBRARIES) \
	--fqbn $(FBQN) $(PROJECT)

install : $(HEX)
	$(UPLOAD) --verbose --verify \
	--fqbn $(FBQN) -p $(PORT) $(PROJECT)

.PHONY: build

