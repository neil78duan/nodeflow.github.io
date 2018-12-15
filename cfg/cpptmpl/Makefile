# c test  Makefile 
# 2005-11-6 
# neil duan 

include ./Rules.make

PROJ2  = ../../bin/cppTemplate

TOOLOBJS :=  apo_script_data.o 

LFLAGS += -L../../lib/$(AIM_NAME)

ifeq ($(DEBUG),y)
	LFLAGS +=  -llogic_parser_d -lnd_vm_dbg -lndclient_$(AIM_NAME)_d
else
	LFLAGS +=  -llogic_parser  -lnd_vm -lndclient_$(AIM_NAME)
endif


VPATH := .

all:  $(PROJ2)
			

$(PROJ2): $(TOOLOBJS) $(MYLIBS)
	$(CPP) -o  $(PROJ2) $(TOOLOBJS) $(LFLAGS)


.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
	
clean:
	rm -f *.o  $(PROJS) $(OBJS:.o=.d)
	
