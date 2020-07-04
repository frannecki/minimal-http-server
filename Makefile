CC = gcc
SLIBDIR = bin
CGIDIR = cgi-bin
EXEC = $(SLIBDIR)/server
COMMON = -Iinclude/ -L$(SLIBDIR)/
CGIEXEC = addnum hostinfo
CGIEXECS = $(addprefix $(CGIDIR)/, $(CGIEXEC))
SLIB = libtiny.so
SLIBS = $(addprefix $(SLIBDIR)/, $(SLIB))

all: $(SLIBDIR) $(CGIDIR) $(SLIBS) $(EXEC) $(CGIEXECS)

$(EXEC): tiny_server.c $(SLIBS)
	@$(CC) -o $@ ${COMMON} $^

$(SLIBDIR):
	@mkdir $(SLIBDIR)

$(SLIBS): src/*.c
	@$(CC) -shared -fPIC -o $@ $(COMMON) $^ -lpthread

$(CGIDIR):
	@mkdir $(CGIDIR)

$(CGIDIR)/%: cgi-src/%.c $(wildcard cgi-src/*.c) 
	@$(CC) -o $@ $<

clean:
	@rm -r $(SLIBDIR)
	@rm -r $(CGIDIR)