
OBJS = $(SOURCES:.cpp=.o)

ifeq ($(CFG),)
CFG=debug
endif

# for debugger: -O0 -g -Wall
# for development and internal release: -O2 -g -Wall
# for release outside the company: -O2 -Wall

ifeq ($(CFG),debug)
CXXFLAGS += -g -Wall -DNDEBUG
else
CXXFLAGS += -O2 -Wall
endif

all: dirs cfgcheck $(OUT)

.PHONY: clean cleanall all $(DIRS)

cfgcheck:
ifneq ($(CFG),release)
ifneq ($(CFG),debug)
	@echo "Error: Invalid CFG '$(CFG)'' (options: debug,release)"
	@exit 1
endif
endif
	@echo "Making '$(CURDIR)' CFG="$(CFG)

$(OUT): $(OBJS)
ifeq ($(TYPE),lib)
	$(AR) rcs $(OUT) $(OBJS)
endif
ifeq ($(TYPE),exe)
	$(CXX) -o $@ $^ ${LDFLAGS} $(LIBS)
endif

-include $(OBJS:.o=.d)

%.o: %.cpp
	$(CXX) -c $(INCLUDES) $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

dirs: $(DIRS)

clean:
	rm -f $(OUT) *.o *.d

cleanall: TARG:=clean
cleanall: $(DIRS)

$(DIRS):
	@$(MAKE) -C $@ $(TARG)