KERNEL_DIR := /home/collin/code/lubancat_sdk/kernel
ARCH       := arm64
CROSS_COMPILE := aarch64-linux-gnu-
export ARCH CROSS_COMPILE

MODULE_DIRS := $(CURDIR)/char_dev_exam $(CURDIR)/led_driver $(CURDIR)/bus_driver $(CURDIR)/platform_driver $(CURDIR)/devicetree \
		       $(CURDIR)/devicetree_led	

BUILD_DIR := /home/collin/shared

all: $(BUILD_DIR)
	@for dir in $(MODULE_DIRS); do \
		echo "Building module in $$dir"; \
		$(MAKE) -C $(KERNEL_DIR) M=$$dir modules; \
		\
			echo "Moving files to $(BUILD_DIR)"; \
		\
		for file in $$dir/*.ko; do \
			[ -e "$$file" ] && mv -f "$$file" $(BUILD_DIR)/; \
		done; \
		\
		for file in $$dir/*.mod.o; do \
			[ -e "$$file" ] && mv -f "$$file" $(BUILD_DIR)/; \
		done; \
		\
		for file in $$dir/*.o; do \
			[ -e "$$file" ] && mv -f "$$file" $(BUILD_DIR)/; \
		done; \
		\
		if [ -f "$$dir/Module.symvers" ]; then \
			mv -f "$$dir/Module.symvers" $(BUILD_DIR)/; \
		fi; \
		\
		if [ -f "$$dir/modules.order" ]; then \
			mv -f "$$dir/modules.order" $(BUILD_DIR)/; \
		fi; \
	done

$(BUILD_DIR):
	mkdir -p $@

PHONY: clean
clean:
	@for dir in $(MODULE_DIRS); do \
		echo "Cleaning $$dir"; \
		$(MAKE) -C $(KERNEL_DIR) M=$$dir clean; \
		rm -f $$dir/*.ko $$dir/*.o $$dir/*.mod.o; \
	done
	rm -f $(BUILD_DIR)/*