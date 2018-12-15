#make file 

SRCDIR=.
OBJDIR = ./.objs

include ../../Rules.make


##############set flag


CFLAGS += -I$(TOPDIR)/include -I$(TOPDIR)/include/logic_parser 
##########make 
##############################################################

LFLAGS += -l$(COMMON_DLL)

ifeq ($(DEBUG),y)
    PROJS = $(LIBDIR)/libnfParser_s_d.a
	DLLPROJS = $(LIBDIR)/libnfParser_d.$(DLL_EXT_NAME)
else 
    PROJS = $(LIBDIR)/libnfParser_s.a
	DLLPROJS = $(LIBDIR)/libnfParser.$(DLL_EXT_NAME)
endif

ifeq ($(BUILD_DLL),y)
   PROJS = $(DLLPROJS)
   LOCAL_CFLAG =  $(CFLAGS) -fPIC
else 
   LOCAL_CFLAG =  $(CFLAGS) 
endif

ifeq ($(BUILD_DLL),y)
   PROJS = $(DLLPROJS)
   LOCAL_CFLAG =  $(CFLAGS) -fPIC
else 
   LOCAL_CFLAG =  $(CFLAGS) 
endif



all: $(PROJS)

$(PROJS): $(OBJS)
	$(AR) $(PROJS)  $(PathOBJS)  

dll: $(DLLPROJS)

$(DLLPROJS): $(OBJS)
	$(CPP_DLL) -fPIC -o $(DLLPROJS)  $(PathOBJS) $(LFLAGS)

.c.o:
	$(CC) $(INCLUDE) $(LOCAL_CFLAG) -o $(OBJDIR)/$*.o $<

.cpp.o:
	$(CPP) $(INCLUDE) $(CFLAGS) -o $(OBJDIR)/$*.o $<
	
clean_dll:
	rm -f $(DLLPROJS) $(PathOBJS)  $(PathOBJS:.o=.d)

clean:
	rm -f $(PROJS) $(PathOBJS)  $(PathOBJS:.o=.d)

