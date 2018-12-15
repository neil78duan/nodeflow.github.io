#make file 


include ../../Rules.make

OBJS := nfmain.o

LFLAGS += -l$(COMMON_LIB) -liconv

PROJECT_NAME := nodeflow
TMPL_NAME := cppTemplate

CFLAGS += -I$(TOPDIR)/include -I./  -I$(NDHOME)/include -I../machoPlug

ifeq ($(DEBUG),y)
	PROJ = $(WORKDIR)/$(PROJECT_NAME)_d
	PROJ_TMPL = $(WORKDIR)/$(TMPL_NAME)_d
	LFLAGS +=  -lnfParser_s_d 
else
	PROJ = $(WORKDIR)/$(PROJECT_NAME)
	PROJ_TMPL = $(WORKDIR)/$(TMPL_NAME)
	LFLAGS +=  -lnfParser_s 
endif


all:  $(PROJ) $(PROJ_TMPL)
	
$(PROJ_TMPL): templCPPmain.o
	$(CPP) -o  $(PROJ_TMPL) templCPPmain.o   $(LFLAGS)

$(PROJ): $(OBJS) 
	$(CPP) -o  $(PROJ) $(OBJS)   $(LFLAGS)
	
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $*.o $<

clean:
	rm -f $(OBJS)  $(PROJ)   *.d
	

	

