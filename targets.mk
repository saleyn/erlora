.SUFFIXES: .erl .$(EMULATOR) .mib .bin .script .boot .html .edoc .app .rel
.PHONY   : all clean info version boot install install_app

all: $(ALL_TARGET)

clean:
	@-rm -f $(ERL_OBJECTS) $(CLEAN_TARGET)
	@-rm -f *.dump *.$(EMULATOR) core *.core ktrace.out*
	@-rm -f $(EBIN_DIR)/$(APPNAME).boot $(EBIN_DIR)/$(APPNAME).script $(ERL_MIBS_BIN)
	@-rm -f $(PRIV_DIR)/*.so *.log $(PRIV_DIR)/*.log
	@-rm -fr Mnesia.*
	@-rm -f $(APPNAME).tar.gz
	@-rm -f $(APPNAME).boot
	@-rm -f $(APPNAME).script

# This target is moved to ../releases/$(VSN)/Makefile
#	erl $(ALL_EBIN_DIRS) \
#	  -eval "systools:make_script(\"$(APPNAME)\")." \
#	  -noshell -s init stop | grep -v '\*WARNING\* snmp\: Source code not found' | cat

info:
	@echo "===>Application: $(APPNAME)"
	@echo "===>Version: $(VSN)"
	@echo "===>Sources:"
	@echo $(ERL_SOURCES)
	@echo "===>Documents:"
	@echo $(ERL_DOCUMENTS)
	@echo "===>Targets (all):"
	@echo $(ALL_TARGET)
	@echo "===>ERL Compile Flags:"
	@echo $(ERLC_FLAGS)
	@echo "===>ERL ebin dirs"
	@echo $(ALL_EBIN_DIRS)
	@echo "===>ERL MIB bin files"
	@echo $(ERL_MIBS_BIN)

version:
	@echo "$(VSN)"

boot: $(EBIN_DIR)/$(APPNAME).boot

# Compile *.erl -> *.beam
$(EBIN_DIR)/%.$(EMULATOR): %.erl
	$(ERLC) $(ERLC_FLAGS) -o $(EBIN_DIR) $<

# Generation of *.boot and *.script files
$(EBIN_DIR)/$(APPNAME).boot $(EBIN_DIR)/$(APPNAME).script: $(EBIN_DIR)/$(APPNAME).rel $(ERL_RELEASE) $(ERL_OBJECTS)
	$(ERLC) -pa $(EBIN_DIR) -o $(EBIN_DIR) $(ERL_RELEASE) $(ERLC_BOOT_FLAGS)

# Generation of *.app file from *.app.src
$(EBIN_DIR)/$(APPNAME).app: $(APPNAME).app.src ../vsn.mk
	@echo "Creating $@ from $<"
	@perl -e $(APPSCRIPT) "$(VSN)" $(ERL_SOURCES:%.erl=%) < $< > $@

# generate documentation with edoc:
# this is still not the proper way to do it, but it works
# (see the wumpus application for an example)

# Compile MIB files: *.mib -> *.bin
$(MIBS_BIN_DIR)/%.bin: $(MIBS_SRC_DIR)/%.mib $(MIBS_BIN_DIR)
	$(ERLC) $(MIBS_EXTRA_FLAGS) -I $(MIBS_BIN_DIR) -o $(MIBS_BIN_DIR) $<

$(MIBS_BIN_DIR):
	mkdir -p $(MIBS_BIN_DIR)

# ----------------------------------------------------
# EDOC stuff
# ----------------------------------------------------

EXTRA_FILES = \
	$(DOC_DIR)/overview.edoc $(DOC_DIR)/stylesheet.css

EXTRA_HTML_FILES = \
	$(DOC_DIR)/modules-frame.html $(DOC_DIR)/overview-summary.html $(DOC_DIR)/packages-frame.html

# ----------------------------------------------------

$(ERL_DOCUMENTS): $(DOC_DIR)/%.html: ./%.erl

DOC_OPTS={def,{vsn,"$(VSN)"}},{app_default,"http://www.erlang.org/doc/doc-5.5"}

SYNTAX_TOOLS_DIR=$(ERL_TOP)/lib/syntax_tools
XMERL_DIR=$(ERL_TOP)/lib/xmerl
INCDIR=$(XMERL_DIR)/include

# Note: APPNAME must be provided in the Makefile

docs: $(ERL_DOCUMENTS) # $(EXTRA_HTML_FILES) $(EXTRA_FILES)
	$(ERL) -noshell -pa $(EBIN_DIR) -pa $(SYNTAX_TOOLS_DIR)/ebin \
		-pa $(XMERL_DIR)/ebin -run edoc_run application \
		"'$(APPNAME)'" '"."' '[$(DOC_OPTS), {dir,"$(DOC_DIR)/html"}]'
	$(ERL) -noshell -pa $(EBIN_DIR) -pa $(SYNTAX_TOOLS_DIR)/ebin \
        -pa $(XMERL_DIR)/ebin -run edoc_run application \
	    "'$(APPNAME)'" '"."' \
        '[$(DOC_OPTS), {dir,"$(DOC_DIR)/xml"}, {layout,forrest_layout}, {file_suffix,".xml"}, {edoc_info, false},{stylesheet,false}]'

