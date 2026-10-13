# NEORV32 application makefile wrapper.
# Point NEORV32_HOME at your local NEORV32 checkout.
NEORV32_HOME ?= ../../../neorv32
APP_SRC = main.c
EFFORT  = -O2
include $(NEORV32_HOME)/sw/common/common.mk

# Typical flow:
#   make clean_all install   # builds and writes neorv32_imem_image.vhd
# then re-compile the Quartus project so the IMEM boots this program.
