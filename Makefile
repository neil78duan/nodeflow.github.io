
topdir=(shell pwd)
VPATH = .

# include ./Rules.make

SUBDIRS =  src


all: debug


debug:
	for n in $(SUBDIRS); do $(MAKE) -C $$n debug || exit 1; done

release:
	for n in $(SUBDIRS); do $(MAKE) -C $$n release || exit 1; done


dll : dll_release

dll_debug:
	cd src/parser;  make dll DEBUG="y" PROFILE="y" BUILD_DLL="y" || exit 1 ;

dll_release:
	cd src/parser;  make dll DEBUG="n" PROFILE="n" BUILD_DLL="y" || exit 1 ;

clean_dll:
	cd src/parser; make clean_dll

clean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean DEBUG="n" PROFILE="n"; done
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean DEBUG="y" PROFILE="y" || exit 1; done

config:
	chmod u+x ./config.sh ; ./config.sh
