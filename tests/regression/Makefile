ROOT_DIR := $(realpath ../..)
include $(ROOT_DIR)/config.mk

all:
	$(MAKE) -C basic
	$(MAKE) -C demo
	$(MAKE) -C dogfood
	$(MAKE) -C mstress
	$(MAKE) -C io_addr
	$(MAKE) -C printf
	$(MAKE) -C diverge
	$(MAKE) -C sort
	$(MAKE) -C fence
	$(MAKE) -C vecaddx
	$(MAKE) -C sgemmx
	$(MAKE) -C conv3x
	$(MAKE) -C vecaddx_banked

run-simx:
	$(MAKE) -C basic run-simx
	$(MAKE) -C demo run-simx
	$(MAKE) -C dogfood run-simx
	$(MAKE) -C mstress run-simx
	$(MAKE) -C io_addr run-simx
	$(MAKE) -C printf run-simx
	$(MAKE) -C diverge run-simx
	$(MAKE) -C sort run-simx
	$(MAKE) -C fence run-simx
	$(MAKE) -C vecaddx run-simx
	$(MAKE) -C sgemmx run-simx
	$(MAKE) -C conv3x run-simx
	$(MAKE) -C vecaddx_banked run-simx

run-rtlsim:
	$(MAKE) -C basic run-rtlsim
	$(MAKE) -C demo run-rtlsim
	$(MAKE) -C dogfood run-rtlsim
	$(MAKE) -C mstress run-rtlsim
	$(MAKE) -C io_addr run-rtlsim
	$(MAKE) -C printf run-rtlsim
	$(MAKE) -C diverge run-rtlsim
	$(MAKE) -C sort run-rtlsim
	$(MAKE) -C fence run-rtlsim
	$(MAKE) -C vecaddx run-rtlsim
	$(MAKE) -C sgemmx run-rtlsim
	$(MAKE) -C conv3x run-rtlsim
	$(MAKE) -C vecaddx_banked run-rtlsim

run-opae:
	$(MAKE) -C basic run-opae
	$(MAKE) -C demo run-opae
	$(MAKE) -C dogfood run-opae
	$(MAKE) -C mstress run-opae
	$(MAKE) -C io_addr run-opae
	$(MAKE) -C printf run-opae
	$(MAKE) -C diverge run-opae
	$(MAKE) -C sort run-opae
	$(MAKE) -C fence run-opae
	$(MAKE) -C vecaddx run-opae
	$(MAKE) -C sgemmx run-opae
	$(MAKE) -C conv3x run-opae
	$(MAKE) -C vecaddx_banked run-opae

clean:
	$(MAKE) -C basic clean
	$(MAKE) -C demo clean
	$(MAKE) -C dogfood clean
	$(MAKE) -C mstress clean
	$(MAKE) -C io_addr clean
	$(MAKE) -C printf clean
	$(MAKE) -C diverge clean
	$(MAKE) -C sort clean
	$(MAKE) -C fence clean
	$(MAKE) -C vecaddx clean
	$(MAKE) -C sgemmx clean
	$(MAKE) -C conv3x clean
	$(MAKE) -C vecaddx_banked clean

clean-all:
	$(MAKE) -C basic clean-all
	$(MAKE) -C demo clean-all
	$(MAKE) -C dogfood clean-all
	$(MAKE) -C mstress clean-all
	$(MAKE) -C io_addr clean-all
	$(MAKE) -C printf clean-all
	$(MAKE) -C diverge clean-all
	$(MAKE) -C sort clean-all
	$(MAKE) -C fence clean-all
	$(MAKE) -C vecaddx clean-all
	$(MAKE) -C sgemmx clean-all
	$(MAKE) -C conv3x clean-all
	$(MAKE) -C vecaddx_banked clean-all
