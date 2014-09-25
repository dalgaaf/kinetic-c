PROJECT = kinetic-c-client
UTILITY = kinetic-c-util

OUT_DIR = ./obj
BIN_DIR = ./bin
PUB_INC = ./include
API_NAME = kinetic_client
LIB_DIR = ./src/lib
UTIL_DIR = ./src/utility
UTIL_EX = $(UTIL_DIR)/examples
PBC_INC = ./vendor/protobuf-c
SOCKET99 = ./vendor/socket99
VND_INC = ./vendor
BIN = $(BIN_DIR)/kinetic_client
LDFLAGS += -lm -l crypto -l ssl

PREFIX ?= /usr/local
INSTALL ?= install
RM ?= rm

KINETIC_LIB = $(BIN_DIR)/lib${PROJECT}.a
VERSION = $(shell head -n1 ./VERSION)
KINETIC_SO = $(BIN_DIR)/lib${PROJECT}.${VERSION}.so
UTIL_EXEC_NAME = $(UTILITY).$(VERSION)
UTIL_EXEC = $(BIN_DIR)/$(UTIL_EXEC_NAME)
UTIL_EXEC_DYN_NAME = $(UTILITY)
UTIL_EXEC_DYN = $(BIN_DIR)/$(UTIL_EXEC_DYN_NAME)

CC = gcc
OPTIMIZE = -O3
WARN = -Wall -Wextra -pedantic
# This is necessary because the library depends on
# both C99 _and_ POSIX (for the BSD sockets API).
CDEFS += -D_POSIX_C_SOURCE=1
CFLAGS += -std=c99 -fPIC -g ${WARN} ${CDEFS} ${OPTIMIZE}

LIB_INCS = -I$(LIB_DIR) -I$(PUB_INC) -I$(PBC_INC) -I$(VND_INC)
LIB_DEPS = $(PUB_INC)/kinetic_client.h $(PUB_INC)/kinetic_types.h $(LIB_DIR)/kinetic_connection.h $(LIB_DIR)/kinetic_hmac.h $(LIB_DIR)/kinetic_logger.h $(LIB_DIR)/kinetic_message.h $(LIB_DIR)/kinetic_nbo.h $(LIB_DIR)/kinetic_operation.h $(LIB_DIR)/kinetic_pdu.h $(LIB_DIR)/kinetic_proto.h $(LIB_DIR)/kinetic_socket.h
# LIB_OBJ = $(patsubst %,$(OUT_DIR)/%,$(LIB_OBJS))
LIB_OBJS = $(OUT_DIR)/kinetic_nbo.o $(OUT_DIR)/kinetic_operation.o $(OUT_DIR)/kinetic_pdu.o $(OUT_DIR)/kinetic_proto.o $(OUT_DIR)/kinetic_socket.o $(OUT_DIR)/kinetic_message.o $(OUT_DIR)/kinetic_logger.o $(OUT_DIR)/kinetic_hmac.o $(OUT_DIR)/kinetic_connection.o $(OUT_DIR)/kinetic_types.o $(OUT_DIR)/kinetic_client.o $(OUT_DIR)/socket99.o $(OUT_DIR)/protobuf-c.o

default: $(KINETIC_SO)

test: Rakefile $(LIB_OBJS)
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Testing $(PROJECT)
	@echo --------------------------------------------------------------------------------
	bundle install
	bundle exec rake ci

clean:
	rm -rf $(BIN_DIR)/* $(OUT_DIR)/*.o *.core

.PHONY: clean

# $(OUT_DIR)/%.o: %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

$(OUT_DIR)/kinetic_nbo.o: $(LIB_DIR)/kinetic_nbo.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_pdu.o: $(LIB_DIR)/kinetic_pdu.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_proto.o: $(LIB_DIR)/kinetic_proto.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_socket.o: $(LIB_DIR)/kinetic_socket.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_message.o: $(LIB_DIR)/kinetic_message.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_logger.o: $(LIB_DIR)/kinetic_logger.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_hmac.o: $(LIB_DIR)/kinetic_hmac.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_connection.o: $(LIB_DIR)/kinetic_connection.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_operation.o: $(LIB_DIR)/kinetic_operation.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/socket99.o: $(SOCKET99)/socket99.c $(SOCKET99)/socket99.h
	$(CC) -c -o $@ $< $(CFLAGS) -I$(SOCKET99)
$(OUT_DIR)/protobuf-c.o: $(PBC_INC)/protobuf-c/protobuf-c.c $(PBC_INC)/protobuf-c/protobuf-c.h
	$(CC) -c -o $@ $< -std=c99 -fPIC -g -Wall ${OPTIMIZE} -I$(PBC_INC)
$(OUT_DIR)/kinetic_types.o: $(LIB_DIR)/kinetic_types.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)
$(OUT_DIR)/kinetic_client.o: $(LIB_DIR)/kinetic_client.c $(LIB_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIB_INCS)

$(KINETIC_LIB): $(LIB_OBJS)
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Building $(KINETIC_LIB) static library
	@echo --------------------------------------------------------------------------------
	ar -rcs $@ $(LIB_OBJS)
	ar -t $@

$(KINETIC_SO): $(KINETIC_LIB)
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Building $(KINETIC_SO) dynamic library
	@echo --------------------------------------------------------------------------------
	$(CC) $(LIB_OBJS) -shared ${LDFLAGS} -o ${KINETIC_SO}

libso: $(KINETIC_SO)

UTIL_OBJS = $(OUT_DIR)/noop.o $(OUT_DIR)/put.o $(OUT_DIR)/get.o $(OUT_DIR)/delete.o
UTIL_INCS = -I/usr/local/include -I$(UTIL_DIR)

$(OUT_DIR)/noop.o: $(UTIL_EX)/noop.c
	$(CC) -c -o $@ $< $(CFLAGS) $(UTIL_INCS)
$(OUT_DIR)/put.o: $(UTIL_EX)/put.c
	$(CC) -c -o $@ $< $(CFLAGS) $(UTIL_INCS)
$(OUT_DIR)/get.o: $(UTIL_EX)/get.c
	$(CC) -c -o $@ $< $(CFLAGS) $(UTIL_INCS)
$(OUT_DIR)/delete.o: $(UTIL_EX)/delete.c
	$(CC) -c -o $@ $< $(CFLAGS) $(UTIL_INCS)
$(UTIL_EXEC): $(UTIL_DIR)/main.c $(UTIL_OBJS)
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Building $(UTIL_EXEC) $(PROJECT) test utility \(statically linked\)
	@echo --------------------------------------------------------------------------------
	${CC} -o $@ $< $(UTIL_OBJS) $(UTIL_INCS) ${CFLAGS} -l $(PROJECT) ${LDFLAGS}
$(UTIL_EXEC_DYN): $(UTIL_DIR)/main.c $(UTIL_OBJS)
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Building $(UTIL_EXEC) $(PROJECT) test utility \(dynamically linked\)
	@echo --------------------------------------------------------------------------------
	${CC} -o $@ -L$(PREFIX) -l $(PROJECT).$(VERSION) $< $(UTIL_OBJS) $(UTIL_INCS) ${CFLAGS} ${LDFLAGS}

# utility: ${UTIL_EXEC} ${UTIL_EXEC_DYN}
utility: ${UTIL_EXEC}

# Configure to launch java simulator
# JAVA=${JAVA_HOME}/bin/java
SIM_JARS_PREFIX = vendor/kinetic-java/kinetic-simulator-0.7.0.2-kinetic-proto-2.0.6-SNAPSHOT
CLASSPATH = ${JAVA_HOME}/lib/tools.jar:$(SIM_JARS_PREFIX)-jar-with-dependencies.jar:$(SIM_JARS_PREFIX)-sources.jar:$(SIM_JARS_PREFIX).jar
SIM_RUNNER = com.seagate.kinetic.simulator.internal.SimulatorRunner
SIM_ADMIN = com.seagate.kinetic.admin.cli.KineticAdminCLI

run: ${UTIL_EXEC}
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Running $(UTIL_EXEC) $(PROJECT) test utility \(statically linked\)
	@echo --------------------------------------------------------------------------------
	@sleep 2
	exec java -classpath "${CLASSPATH}" ${SIM_RUNNER} "$@" &
	@sleep 5
	exec java -classpath "${CLASSPATH}" ${SIM_ADMIN} -setup -erase true
	${UTIL_EXEC} noop
	${UTIL_EXEC} put
	${UTIL_EXEC} get
	${UTIL_EXEC} delete
	exec pkill -f 'java.*kinetic-simulator'

rund: ${UTIL_EXEC_DYN}
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Running $(UTIL_EXEC) $(PROJECT) test utility \(dynamically linked\)
	@echo --------------------------------------------------------------------------------
	@sleep 2
	exec java -classpath "${CLASSPATH}" ${SIM_RUNNER} "$@" &
	@sleep 5
	exec java -classpath "${CLASSPATH}" ${SIM_ADMIN} -setup -erase true
	${UTIL_EXEC_DYN} noop
	${UTIL_EXEC_DYN} put
	${UTIL_EXEC_DYN} get
	${UTIL_EXEC_DYN} delete
	exec pkill -f 'java.*kinetic-simulator'

# Installation
install: ${KINETIC_LIB} ${KINETIC_SO} VERSION
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Installing $(PROJECT) v$(VERSION) into $(PREFIX)
	@echo --------------------------------------------------------------------------------
	@echo
	@echo You may be prompted for your password in order to proceed.
	@echo
	${INSTALL} -d ${PREFIX}/lib/
	${INSTALL} -c ${KINETIC_LIB} ${PREFIX}/lib/
	# ${INSTALL} -c ${KINETIC_SO} ${PREFIX}/lib/
	${INSTALL} -d ${PREFIX}/include/
	${INSTALL} -c ./include/${API_NAME}.h ${PREFIX}/include/
	${INSTALL} -c ./include/kinetic_types.h ${PREFIX}/include/
	${INSTALL} -c ./src/lib/kinetic_proto.h ${PREFIX}/include/
	${INSTALL} -d ${PREFIX}/include/protobuf-c
	${INSTALL} -c ./vendor/protobuf-c/protobuf-c/protobuf-c.h ${PREFIX}/include/protobuf-c/

uninstall:
	@echo
	@echo --------------------------------------------------------------------------------
	@echo Uninstalling $(PROJECT) from $(PREFIX)
	@echo --------------------------------------------------------------------------------
	@echo
	@echo You may be prompted for your password in order to proceed.
	@echo
	${RM} -f ${PREFIX}/lib/lib${PROJECT}*.a
	${RM} -f ${PREFIX}/lib/lib${PROJECT}*.so
	${RM} -f ${PREFIX}/include/${PUBLIC_API}.h
	${RM} -f ${PREFIX}/include/kinetic_types.h
	${RM} -f ${PREFIX}/include/kinetic_proto.h
	${RM} -f ${PREFIX}/include/protobuf-c/protobuf-c.h
	${RM} -f ${PREFIX}/include/protobuf-c.h

eclipse: clean default

all:

# all: uninstall clean test default install run rund
#all: uninstall clean test default install run
all:

	@echo
	@echo --------------------------------------------------------------------------------
	@echo $(PROJECT) build completed successfully!
	@echo --------------------------------------------------------------------------------
	@echo $(PROJECT) v$(VERSION) is in working order
	@echo

# Other dependencies
$(BIN_DIR)/lib${PROJECT}.a: Makefile Rakefile VERSION
# kinetic-lib.o: kinetic_client.h kinetic_connection.h kinetic_hmac.h kinetic_logger.h kinetic_message.h kinetic_nbo.h kinetic_operation.h kinetic_pdu.h kinetic_proto.h kinetic_socket.h protobuf-c.h socket99.h
