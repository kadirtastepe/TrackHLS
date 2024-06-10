#======================
#
# Targets and config
#
#======================

# Binary name
EXE=app_${XCL_EMULATION_MODE}

# Directories to include
HOMEDIR := ${FPGA_PATH2EMU}
BLDDIR := ${FPGA_PATH2EMU_OBJ}
LOGDIR := ${FPGA_PATH2EMU_LOG}
BINDIR := ${FPGA_PATH2EMU_BIN}
ETCDIR := ${FPGA_PATH2EMU_ETC}
SRCDIR := ${FPGA_PATH2SRC}
INCDIR := ${FPGA_PATH2INC}
EXTDIR := ${FPGA_PATH2EXT}

# Make sure directories exist
$(shell mkdir -p $(BLDDIR) $(BINDIR) $(LOGDIR) $(ETCDIR))

#======================
#
# Externals
#
#======================
# Argument parser
ARGPARS     := ${EXTDIR}/CLI11

#======================
#
# Compiler flags
#
#======================

# make phony target to simplify calls
.PHONY: clean 

# Targets
TARGET:= ${BINDIR}/${EXE}

CC = g++
LDFLAGS := -Itemp \
	   -lxrt_coreutil \
	   -L.${INCDIR} \
	   -L${XILINX_HLS}/lib \
	   -L${XILINX_XRT}/lib \
	   $(ROOTLFLAGS)
CFLAGS  := -g -Wno-unknown-pragmas \
	   -pthread \
	   -std=c++17 \
	   -Wall \
	   -O0 \
	   -I$(ARGPARS)/include \
	   -I${INCDIR} \
	   -I${XILINX_HLS}/include \
	   -I${XILINX_XRT}/include $(ROOTCFLAGS) #-g -Wall

# For V++
VV = v++

#======================
#
# Targets
#
#======================

# Compile with G++
TARGET: ${BLDDIR}/host.o ${BLDDIR}/kernels.o ${FPGA_PATH2EMU_OBJ}/kernels.xo ${FPGA_PATH2EMU_BIN}/kernels.xclbin all
	$(CC) $(CFLAGS) ${BLDDIR}/host.o ${BLDDIR}/kernels.o -o ${BINDIR}/${EXE} $(LDFLAGS)

${BLDDIR}/kernels.o: ${SRCDIR}/kernels.cpp
	$(CC) $(CFLAGS) -c ${SRCDIR}/kernels.cpp -o ${BLDDIR}/kernels.o

${BLDDIR}/host.o: ${SRCDIR}/host.cpp
	$(CC) $(CFLAGS) -c ${SRCDIR}/host.cpp -o ${BLDDIR}/host.o

# Compile with V++
${FPGA_PATH2EMU_BIN}/kernels.xclbin: ${FPGA_PATH2EMU_OBJ}/kernels.xo
	$(VV) -g -l -t ${XCL_EMULATION_MODE} \
	       	--platform ${FPGA_PLATFORM} \
	       	--config ${FPGA_PATH2ETC}/u280.cfg \
	       	${FPGA_PATH2EMU_OBJ}/kernels.xo -o ${FPGA_PATH2EMU_BIN}/kernels.xclbin

${FPGA_PATH2EMU_OBJ}/kernels.xo: ${FPGA_PATH2SRC}/kernels.cpp
	$(VV) -g -c -t ${XCL_EMULATION_MODE} \
	       	--platform ${FPGA_PLATFORM} \
	       	--config ${FPGA_PATH2ETC}/u280.cfg \
	       	-k fit_local \
	       	-I ${FPGA_PATH2INC} ${FPGA_PATH2SRC}/kernels.cpp -o ${FPGA_PATH2EMU_OBJ}/kernels.xo

# TODO GET RID OF LOG FILES
all: 
#	@mv *.log $(LOGDIR)/.
#	@mv *.json $(ETCDIR)/.
#	@mv -f _x $(HOMEDIR)/vpp_out

clean:
	@rm -rf *.log out/* _x xilinx* *.csv xrt.run_summary *.str *.jou
