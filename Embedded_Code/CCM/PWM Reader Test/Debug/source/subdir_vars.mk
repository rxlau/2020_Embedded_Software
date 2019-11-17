################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../source/HL_sys_link.cmd 

ASM_SRCS += \
../source/HL_sys_core.asm \
../source/HL_sys_intvecs.asm \
../source/HL_sys_mpu.asm \
../source/HL_sys_pmu.asm 

C_SRCS += \
../source/HL_crc.c \
../source/HL_ecap.c \
../source/HL_emac.c \
../source/HL_epc.c \
../source/HL_errata.c \
../source/HL_errata_SSWF021_45.c \
../source/HL_esm.c \
../source/HL_etpwm.c \
../source/HL_gio.c \
../source/HL_het.c \
../source/HL_mdio.c \
../source/HL_nmpu.c \
../source/HL_notification.c \
../source/HL_phy_dp83640.c \
../source/HL_phy_tlk111.c \
../source/HL_pinmux.c \
../source/HL_rti.c \
../source/HL_sys_dma.c \
../source/HL_sys_main.c \
../source/HL_sys_pcr.c \
../source/HL_sys_phantom.c \
../source/HL_sys_pmm.c \
../source/HL_sys_startup.c \
../source/HL_sys_vim.c \
../source/HL_system.c 

C_DEPS += \
./source/HL_crc.d \
./source/HL_ecap.d \
./source/HL_emac.d \
./source/HL_epc.d \
./source/HL_errata.d \
./source/HL_errata_SSWF021_45.d \
./source/HL_esm.d \
./source/HL_etpwm.d \
./source/HL_gio.d \
./source/HL_het.d \
./source/HL_mdio.d \
./source/HL_nmpu.d \
./source/HL_notification.d \
./source/HL_phy_dp83640.d \
./source/HL_phy_tlk111.d \
./source/HL_pinmux.d \
./source/HL_rti.d \
./source/HL_sys_dma.d \
./source/HL_sys_main.d \
./source/HL_sys_pcr.d \
./source/HL_sys_phantom.d \
./source/HL_sys_pmm.d \
./source/HL_sys_startup.d \
./source/HL_sys_vim.d \
./source/HL_system.d 

OBJS += \
./source/HL_crc.obj \
./source/HL_ecap.obj \
./source/HL_emac.obj \
./source/HL_epc.obj \
./source/HL_errata.obj \
./source/HL_errata_SSWF021_45.obj \
./source/HL_esm.obj \
./source/HL_etpwm.obj \
./source/HL_gio.obj \
./source/HL_het.obj \
./source/HL_mdio.obj \
./source/HL_nmpu.obj \
./source/HL_notification.obj \
./source/HL_phy_dp83640.obj \
./source/HL_phy_tlk111.obj \
./source/HL_pinmux.obj \
./source/HL_rti.obj \
./source/HL_sys_core.obj \
./source/HL_sys_dma.obj \
./source/HL_sys_intvecs.obj \
./source/HL_sys_main.obj \
./source/HL_sys_mpu.obj \
./source/HL_sys_pcr.obj \
./source/HL_sys_phantom.obj \
./source/HL_sys_pmm.obj \
./source/HL_sys_pmu.obj \
./source/HL_sys_startup.obj \
./source/HL_sys_vim.obj \
./source/HL_system.obj 

ASM_DEPS += \
./source/HL_sys_core.d \
./source/HL_sys_intvecs.d \
./source/HL_sys_mpu.d \
./source/HL_sys_pmu.d 

OBJS__QUOTED += \
"source\HL_crc.obj" \
"source\HL_ecap.obj" \
"source\HL_emac.obj" \
"source\HL_epc.obj" \
"source\HL_errata.obj" \
"source\HL_errata_SSWF021_45.obj" \
"source\HL_esm.obj" \
"source\HL_etpwm.obj" \
"source\HL_gio.obj" \
"source\HL_het.obj" \
"source\HL_mdio.obj" \
"source\HL_nmpu.obj" \
"source\HL_notification.obj" \
"source\HL_phy_dp83640.obj" \
"source\HL_phy_tlk111.obj" \
"source\HL_pinmux.obj" \
"source\HL_rti.obj" \
"source\HL_sys_core.obj" \
"source\HL_sys_dma.obj" \
"source\HL_sys_intvecs.obj" \
"source\HL_sys_main.obj" \
"source\HL_sys_mpu.obj" \
"source\HL_sys_pcr.obj" \
"source\HL_sys_phantom.obj" \
"source\HL_sys_pmm.obj" \
"source\HL_sys_pmu.obj" \
"source\HL_sys_startup.obj" \
"source\HL_sys_vim.obj" \
"source\HL_system.obj" 

C_DEPS__QUOTED += \
"source\HL_crc.d" \
"source\HL_ecap.d" \
"source\HL_emac.d" \
"source\HL_epc.d" \
"source\HL_errata.d" \
"source\HL_errata_SSWF021_45.d" \
"source\HL_esm.d" \
"source\HL_etpwm.d" \
"source\HL_gio.d" \
"source\HL_het.d" \
"source\HL_mdio.d" \
"source\HL_nmpu.d" \
"source\HL_notification.d" \
"source\HL_phy_dp83640.d" \
"source\HL_phy_tlk111.d" \
"source\HL_pinmux.d" \
"source\HL_rti.d" \
"source\HL_sys_dma.d" \
"source\HL_sys_main.d" \
"source\HL_sys_pcr.d" \
"source\HL_sys_phantom.d" \
"source\HL_sys_pmm.d" \
"source\HL_sys_startup.d" \
"source\HL_sys_vim.d" \
"source\HL_system.d" 

ASM_DEPS__QUOTED += \
"source\HL_sys_core.d" \
"source\HL_sys_intvecs.d" \
"source\HL_sys_mpu.d" \
"source\HL_sys_pmu.d" 

C_SRCS__QUOTED += \
"../source/HL_crc.c" \
"../source/HL_ecap.c" \
"../source/HL_emac.c" \
"../source/HL_epc.c" \
"../source/HL_errata.c" \
"../source/HL_errata_SSWF021_45.c" \
"../source/HL_esm.c" \
"../source/HL_etpwm.c" \
"../source/HL_gio.c" \
"../source/HL_het.c" \
"../source/HL_mdio.c" \
"../source/HL_nmpu.c" \
"../source/HL_notification.c" \
"../source/HL_phy_dp83640.c" \
"../source/HL_phy_tlk111.c" \
"../source/HL_pinmux.c" \
"../source/HL_rti.c" \
"../source/HL_sys_dma.c" \
"../source/HL_sys_main.c" \
"../source/HL_sys_pcr.c" \
"../source/HL_sys_phantom.c" \
"../source/HL_sys_pmm.c" \
"../source/HL_sys_startup.c" \
"../source/HL_sys_vim.c" \
"../source/HL_system.c" 

ASM_SRCS__QUOTED += \
"../source/HL_sys_core.asm" \
"../source/HL_sys_intvecs.asm" \
"../source/HL_sys_mpu.asm" \
"../source/HL_sys_pmu.asm" 


