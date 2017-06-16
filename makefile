# makefile

# 文件路径
COM=com
HEAD=head
SRC=src
BIN=bin
LIB_SLAB=slab.o
LIB_POOL=pool.o

# 文件名称
POOL=pool.c
SLAB=slab.c
TEST_POOL=test_pool.c
TEST_SLAB=test_slab.c
RES_POOL=test_pool
RES_SLAB=test_slab
# 编译器
CC=gcc

all : build

build:
	${CC} -c ${SRC}/${POOL} -o ${COM}/${LIB_POOL} -I ${HEAD}
	${CC} -c ${SRC}/${SLAB} -o ${COM}/${LIB_SLAB} -I ${HEAD}

install:

	${CC} ${COM}/${LIB_POOL} ${SRC}/${TEST_POOL} -o ${BIN}/${RES_POOL} -I ${HEAD}
	${CC} ${COM}/${LIB_SLAB} ${COM}/${LIB_POOL} ${SRC}/${TEST_SLAB} -o ${BIN}/${RES_SLAB} -I ${HEAD}
.PHONY:clean
clean:
	rm ${COM}/${LIB_SLAB}
	rm ${COM}/${LIB_POOL}
	rm ${BIN}/${RES_POOL}
	rm ${BIN}/${RES_SLAB}
