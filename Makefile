CC	= gcc
LD	= gcc

CFLAGS	= -g -Wall -I. 
LDFLAGS	= 

OBJS	= 

LIBS	= 

SRCS	= $(patsubst %.o,%.c,$(OBJS))

PRGS	=  sts test

all: $(PRGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $*.c $(INCLUDE) -o $@

$(PRGS): $(OBJS)
$(PRGS): $(LIBS)
$(PRGS): % : %.o
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LDFLAGS) $(LIBS)

setup:
	@rm -rf ./test
	@mkdir -p test/direct
	@mkdir -p test/emptydir
	@echo Apples\\nGrapes\\nBananas\\nKiwis\\nBlueberries > test/random.txt
	@echo Hello world > test/direct/helloworld.txt
	@touch -A -120000 test/emptydir
	@touch -A -040000 test/random.txt
	@touch -A -020000 test/direct/helloworld.txt
	@touch -A -080000 test/direct
	@echo Setup complete.

clean:
	-rm -f *.o  *~ $(PRGS)
	-rm -rf ./test