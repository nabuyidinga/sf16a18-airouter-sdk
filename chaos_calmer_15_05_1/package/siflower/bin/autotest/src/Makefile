EXEC=autotest
EXEB=random_name
EXED=flash_speed_test
SRC=socket_server.c
SRB=random_name.c
SRD=flash_speed_test.c

all: $(EXEC)  $(EXEB) $(EXED)
autotest: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

random_name: $(SRB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRB) $(LDLIBS)

flash_speed_test: $(SRD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRD) $(LDLIBS)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
	-rm -f $(EXEB) *.elf *.gdb *.o
