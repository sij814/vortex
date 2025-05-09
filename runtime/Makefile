ROOT_DIR := $(realpath ..)
include $(ROOT_DIR)/config.mk

all: stub rtlsim simx opae opaesimx xrt

vm: stub simx

stub:
	$(MAKE) -C stub

simx:
	$(MAKE) -C simx

rtlsim:
	$(MAKE) -C rtlsim

opae:
	$(MAKE) -C opae

opaesimx:
	$(MAKE) -C opaesimx

xrt:
	$(MAKE) -C xrt

clean:
	$(MAKE) -C stub clean
	$(MAKE) -C simx clean
	$(MAKE) -C rtlsim clean
	$(MAKE) -C opae clean
	$(MAKE) -C opaesimx clean
	$(MAKE) -C xrt clean

.PHONY: all stub simx rtlsim opae opaesimx xrt clean