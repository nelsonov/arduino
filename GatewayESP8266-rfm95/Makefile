####### Serial up0load Makefile
# Project Structure:
#   /PROJECTBASE
#   |- .git
#   |- .gitignore
#   |
#   |- PROJECT
#   |  |- PROJECT
#   |  |  |- *.ino *.h *.c *.cpp
#   |  |- Makefile
#   |  |- build
#   |  |  |- PROJECT.ino.hex
#
# Per Project settings:
PORT = /dev/ttyUSB0
PROJECT = GatewayESP8266-rfm95
PROJECTBASE = /srv/src/Arduino/git
FBQN = esp8266:esp8266:nodemcuv2
UPLOADSPEED=57600
#
# The following shouldn't need changes
SRCBASE = $(PROJECTBASE)/$(PROJECT)/$(PROJECT)
BUILD = $(PROJECTBASE)/$(PROJECT)/build
HEX = $(PROJECT).ino.hex

CMD = arduino-cli
COMPILE = $(CMD) compile
UPLOAD = $(CMD) upload

build : $(HEX)

$(HEX) : $(SRCBASE)/*.ino
	$(COMPILE) --verbose --build-path $(BUILD) \
	--fqbn $(FBQN) $(PROJECT)

install : $(HEX)
	$(UPLOAD) --verbose --verify \
	--fqbn $(FBQN) -p $(PORT) $(PROJECT)

.PHONY : build
