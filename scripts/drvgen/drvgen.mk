ifdef MTK_PLATFORM

PRIVATE_CUSTOM_KERNEL_DCT := $(if $(CUSTOM_KERNEL_DCT),$(CUSTOM_KERNEL_DCT),dct)
ifneq ($(wildcard $(PWD)/arch/arm/mach-$(MTK_PLATFORM)/$(MTK_PROJECT)/dct/$(PRIVATE_CUSTOM_KERNEL_DCT)/codegen.dws),)
  DRVGEN_PATH := arch/arm/mach-$(MTK_PLATFORM)/$(MTK_PROJECT)/dct/$(PRIVATE_CUSTOM_KERNEL_DCT)
else
  DRVGEN_PATH := drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(MTK_PROJECT)/dct/$(PRIVATE_CUSTOM_KERNEL_DCT)
endif
ifndef DRVGEN_OUT
DRVGEN_OUT := $(objtree)/$(DRVGEN_PATH)
endif
export DRVGEN_OUT

DRVGEN_OUT_PATH := $(DRVGEN_OUT)/inc

ALL_DRVGEN_FILE :=

ifeq ($(filter mt2601,$(MTK_PLATFORM)),)
  ALL_DRVGEN_FILE += inc/cust_gpio_boot.h
  ALL_DRVGEN_FILE += inc/cust_gpio_usage.h
  ifdef CONFIG_MTK_LEGACY
    ALL_DRVGEN_FILE += inc/cust_kpd.h
    ALL_DRVGEN_FILE += inc/cust_eint.h
    ALL_DRVGEN_FILE += inc/cust_adc.h
    ALL_DRVGEN_FILE += inc/pmic_drv.h
    ALL_DRVGEN_FILE += pmic_drv.c
  endif
endif

ifeq ($(filter mt2601 mt8127 mt8163,$(MTK_PLATFORM)),)
  ifdef CONFIG_MTK_LEGACY
    ALL_DRVGEN_FILE += inc/cust_eint_md1.h
  endif
endif

ifeq ($(filter mt2601 mt6572 mt6582 mt6592 mt8127,$(MTK_PLATFORM)),)
  ALL_DRVGEN_FILE += cust_eint.dtsi
endif

ifeq ($(filter mt2601 mt6580,$(MTK_PLATFORM)),)
  ifdef CONFIG_MTK_LEGACY
    ALL_DRVGEN_FILE += inc/cust_power.h
  endif
endif

ifeq ($(filter mt2601 mt6572 mt6582 mt6592 mt8127 mt8163,$(MTK_PLATFORM)),)
  ifdef CONFIG_MTK_LEGACY
    ALL_DRVGEN_FILE += inc/cust_clk_buf.h
  endif
endif

ifeq ($(filter mt2601 mt6572 mt6582 mt6592 mt8127 mt8163,$(MTK_PLATFORM)),)
  ifdef CONFIG_MTK_LEGACY
    ALL_DRVGEN_FILE += inc/cust_i2c.h
  endif
endif

ifeq ($(MTK_PLATFORM),mt2601)
  ALL_DRVGEN_FILE += cust_kpd.h
  ALL_DRVGEN_FILE += cust_eint.h
  ALL_DRVGEN_FILE += cust_gpio_boot.h
  ALL_DRVGEN_FILE += cust_gpio_usage.h
  ALL_DRVGEN_FILE += cust_power.h
  ALL_DRVGEN_FILE += cust_adc.h
  ALL_DRVGEN_FILE += cust_eint_md1.h
  ALL_DRVGEN_FILE += pmic_drv.h
  ALL_DRVGEN_FILE += pmic_drv.c
endif

ifeq ($(MTK_PLATFORM),mt6752)
  ALL_DRVGEN_FILE += inc/cust_eint_md2.h
endif

ifeq ($(MTK_PLATFORM),mt6595)
  ALL_DRVGEN_FILE += inc/cust_gpio_suspend.h
endif

ifeq ($(MTK_PLATFORM),mt6580)
  ALL_DRVGEN_FILE += cust_i2c.dtsi
endif

ifeq ($(MTK_PLATFORM),mt8127)
  ALL_DRVGEN_FILE += inc/cust_eint_ext.h
endif

ifeq ($(MTK_PLATFORM),mt6735)
  ALL_DRVGEN_FILE += cust_adc.dtsi
  ALL_DRVGEN_FILE += cust_i2c.dtsi
  ALL_DRVGEN_FILE += cust_md1_eint.dtsi
  ALL_DRVGEN_FILE += cust_kpd.dtsi
  ALL_DRVGEN_FILE += cust_clk_buf.dtsi
  ALL_DRVGEN_FILE += cust_gpio.dtsi
  ALL_DRVGEN_FILE += cust_adc.dtsi
  ALL_DRVGEN_FILE += cust_pmic.dtsi
endif

DRVGEN_FILE_LIST := $(addprefix $(DRVGEN_OUT)/,$(ALL_DRVGEN_FILE))
DRVGEN_TOOL := $(PWD)/tools/dct/DrvGen
DWS_FILE := $(PWD)/$(DRVGEN_PATH)/codegen.dws
DRVGEN_PREBUILT_PATH := $(PWD)/$(DRVGEN_PATH)
DRVGEN_PREBUILT_CHECK := $(filter-out $(wildcard $(addprefix $(DRVGEN_PREBUILT_PATH)/,$(ALL_DRVGEN_FILE))),$(addprefix $(DRVGEN_PREBUILT_PATH)/,$(ALL_DRVGEN_FILE)))
#wangyuchao custom codegen dws
HQ_PROJECT_DEFALUT_PATH := $(PWD)/../device/huaqin/$(HQ_PROJECT)/default
HQ_PROJECT_CLIENT_PATH  := $(PWD)/../device/huaqin/$(HQ_PROJECT)/$(HQ_CLIENT)
HQ_CUST_DWS_FILE := $(PWD)/../vendor/mediatek/proprietary/custom/$(MTK_PROJECT)/kernel/dct/dct/codegen.dws
# DCT Path
DCT_PATH := $(HQ_PROJECT_CLIENT_PATH)/drv/codegen.dws
ifneq ($(strip $(wildcard $(DCT_PATH))), )
LOCAL_DCT_PATH := $(DCT_PATH)
else
LOCAL_DCT_PATH := $(HQ_PROJECT_DEFALUT_PATH)/drv/codegen.dws
endif
.PHONY: hqcustgen
hqcustgen:
	@echo www gen kernel codegen dws
ifneq ($(strip $(wildcard $(LOCAL_DCT_PATH))),)
	$(hide) cp $(LOCAL_DCT_PATH) $(DWS_FILE) -f
	$(hide) cp $(LOCAL_DCT_PATH) $(HQ_CUST_DWS_FILE) -f
endif
$(info DWS_FILE=$(PWD))
$(info DWS_FILE22= $(HQ_CUST_DWS_FILE))
.PHONY: drvgen
drvgen: $(DRVGEN_FILE_LIST)
ifneq ($(DRVGEN_PREBUILT_CHECK),)

$(DRVGEN_OUT)/inc/cust_kpd.h: $(DRVGEN_TOOL) $(DWS_FILE) hqcustgen
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) kpd_h

$(DRVGEN_OUT)/inc/cust_eint.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) eint_h

$(DRVGEN_OUT)/inc/cust_gpio_boot.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) gpio_boot_h

$(DRVGEN_OUT)/inc/cust_gpio_usage.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) gpio_usage_h

$(DRVGEN_OUT)/inc/cust_adc.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) adc_h

$(DRVGEN_OUT)/inc/cust_eint_md1.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) md1_eint_h

$(DRVGEN_OUT)/inc/cust_power.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) power_h

$(DRVGEN_OUT)/inc/pmic_drv.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) pmic_h

$(DRVGEN_OUT)/pmic_drv.c: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) pmic_c

$(DRVGEN_OUT)/inc/cust_i2c.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) i2c_h

$(DRVGEN_OUT)/inc/cust_clk_buf.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) clk_buf_h

$(DRVGEN_OUT)/inc/cust_eint_md2.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) md2_eint_h

$(DRVGEN_OUT)/inc/cust_gpio_suspend.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) suspend_h

$(DRVGEN_OUT)/inc/cust_eint_ext.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) eint_ext_h

$(DRVGEN_OUT)/cust_eint.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) eint_dtsi

$(DRVGEN_OUT)/inc/pmic_drv.c: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) pmic_c

$(DRVGEN_OUT)/cust_i2c.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) i2c_dtsi

$(DRVGEN_OUT)/cust_adc.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) adc_dtsi

$(DRVGEN_OUT)/cust_md1_eint.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) md1_eint_dtsi

$(DRVGEN_OUT)/cust_kpd.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) kpd_dtsi

$(DRVGEN_OUT)/cust_clk_buf.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) clk_buf_dtsi

$(DRVGEN_OUT)/cust_gpio.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) gpio_dtsi

$(DRVGEN_OUT)/cust_adc.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) adc_dtsi

$(DRVGEN_OUT)/cust_pmic.dtsi: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT) $(DRVGEN_OUT_PATH) pmic_dtsi

$(DRVGEN_OUT)/inc/mt6735-pinfunc.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) mt6735_pinfunc_h

$(DRVGEN_OUT)/inc/pinctrl-mtk-mt6735.h: $(DRVGEN_TOOL) $(DWS_FILE)
	@mkdir -p $(dir $@)
	@$(DRVGEN_TOOL) $(DWS_FILE) $(DRVGEN_OUT_PATH) $(DRVGEN_OUT_PATH) pinctrl_mtk_mt6735_h

else
$(DRVGEN_FILE_LIST): $(DRVGEN_OUT)/% : $(DRVGEN_PREBUILT_PATH)/%  hqcustgen
	@mkdir -p $(dir $@)
	cp -f $< $@
endif

endif#MTK_PLATFORM
