
ifeq ($(SUBDIRS),)
SUBDIRS	=	c_src src
endif

all clean version docs install_drip install_dripdb:
	@set -e ; \
	  for d in $(SUBDIRS) ; do \
	    if [ -f $$d/Makefile ]; then \
            (cd $$d && $(MAKE) $@) || exit 1 ; \
        elif [ -f $$d/src/Makefile ]; then \
            (cd $$d/src && $(MAKE) $@) || exit 1; \
	    fi \
	  done

