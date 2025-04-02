DPDK_VERSION = 24.11.1
DPDK_DIR = dpdk-$(DPDK_VERSION)
DPDK_TAR = $(DPDK_DIR).tar.gz
DPDK_URL = https://fast.dpdk.org/rel/$(DPDK_TAR)


CFLAGS = -Wall -Wextra -Wstrict-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Werror 
DPDK_INCLUDE = -I/usr/local/include/dpdk
DPDK_LIBS = -L/usr/local/lib -ldl -lnuma -lm -lpthread -lrte_eal -lrte_hash -lrte_mbuf -lrte_net


all: download-dpdk build-dpdk build-test

download-dpdk:
	wget -P test $(DPDK_URL)

build-dpdk:
	tar -xvf test/$(DPDK_TAR) 
	mv dpdk-stable-$(DPDK_VERSION) test/$(DPDK_DIR)
	rm test/$(DPDK_TAR)
	cd test/$(DPDK_DIR) && meson build           
	cd test/$(DPDK_DIR)/build && ninja
	sudo ninja -C test/$(DPDK_DIR)/build install
	sudo ldconfig

build-test: build-dpdk
	gcc $(CFLAGS) $(DPDK_INCLUDE) test/test.c $(DPDK_LIBS) -o test/test

setup-hugepages:
	sudo bash -c "echo 8192 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages"
	@echo "Hugepages configured"

cleanup-hugepages:
	sudo bash -c "echo 0 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages"
	@echo "Hugepages cleared"

run: setup-hugepages
	sudo ./test/test

clean:
	rm -rf test/$(DPDK_DIR) 
	rm -rf $(DPDK_TAR)
