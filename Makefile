FLAGS= -lm -fopenmp -D_GNU_SOURCE -O3 -Wall -g
SRC_DIRS= src/scattershot src

SRCS= $(foreach d,$(SRC_DIRS),$(wildcard $(addprefix $(d)/*,*.c)))

all:
	gcc -I include $(SRCS) -o main $(FLAGS)
