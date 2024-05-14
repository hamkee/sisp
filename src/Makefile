srcdir = .


DESTDIR =
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin

INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL = /usr/bin/install -c

CC = cc
CFLAGS = -g -Wall -pedantic -Wextra
LDFLAGS = -lpthread -lm
LIBS =   


PROG = sisp
OBJS = process.o main.o  vmm.o lexer.o parser.o funcs-set.o funcs-math.o funcs.o object.o misc.o eval.o 

.c.o:
	$(CC) -c -fcommon -DGCTHREADS -I$(srcdir) $(CFLAGS) $<

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -DGCTHREADS -o $(PROG) $(OBJS) $(LIBS)

.PHONY: install uninstall clean realclean distclean mostlyclean

install: $(PROG)
	$(srcdir)/../mkinsdir.sh $(DESTDIR)$(bindir) 
	$(INSTALL_PROGRAM) $(PROG) $(DESTDIR)$(bindir)
	$(INSTALL_DATA) $(srcdir)/$(PROG).1 

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(PROG)

clean: 
	-rm -f $(PROG) *.*~ $(OBJS)


distclean: clean 
	-rm -f Makefile

realclean: distclean

mostlyclean: clean

$(OBJS): sisp.h libsisp.h extern.h 
