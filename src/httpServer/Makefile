#make file 


include ../../Rules.make

OBJS := db_mysql_api.o httpInstant.o httpScriptApi.o mydatabase.o 

LFLAGS += -l$(SRV_LIB) -liconv

PROJECT_NAME := nfhttp


CFLAGS += -I$(TOPDIR)/include -I./ -I$(TOPDIR)/include/mysql -I$(TOPDIR)/include/httpServer

ifeq ($(DEBUG),y)
	PROJ = $(WORKDIR)/$(PROJECT_NAME)_d
	MYLIBS := $(LIBDIR)/libhttpExt_s_d.a
	LFLAGS +=  -lnfParser_s_d -lhttpExt_s_d
else

	PROJ = $(WORKDIR)/$(PROJECT_NAME)
	MYLIBS := $(LIBDIR)/libhttpExt_s.a
	LFLAGS +=  -lnfParser_s -lhttpExt_s

endif



all: $(MYLIBS) $(PROJ)
	
$(MYLIBS): $(OBJS)
	$(AR) $(MYLIBS)  $(OBJS)  
	
$(PROJ): main.o $(MYLIBS)
	$(CPP) -o  $(PROJ) main.o   $(LFLAGS)
	
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -o $*.o $<
.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $*.o $<

clean:
	rm -f *.o  $(PROJ) $(MYLIBS) $(PROJ3) $(LIBOBJS:.o=.d)  main.d
	

	

