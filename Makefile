$(info  Starting building process)

# include configurations

include Makefile.conf

$(info  - Makefile.conf loaded)

# find project files

H_FILES   :=    $(shell find -L ./$(INC_DIRECTORY) -name '*.h' -exec dirname {} \; | sed 's/ /\\ /g' | uniq)

C_FILES   :=    $(shell find ./$(SRC_DIRECTORY) -name '*.c' -type f | sed 's/ /\\ /g' | uniq)

CXX_FILES :=    $(shell find ./$(SRC_DIRECTORY) -name '*.cpp' -type f | sed 's/ /\\ /g' | uniq)

O_FILES   :=    $(C_FILES:.c=.o)

O_FILES   +=    $(CXX_FILES:.cpp=.o)

H_FILES   :=    $(notdir  $(H_FILES))

C_FILES   :=    $(notdir  $(C_FILES))

CXX_FILES :=    $(notdir  $(CXX_FILES))

INCLUDES  :=    $(H_FILES:%=-I%)

$(info  - Project Files Loaded)


ifeq ($(DEBUG),yes)

   $(info  - Debug flag added [makefile.conf DEBUG = yes])

   CFLAGS := -g $(CFLAGS)

endif


ifeq ($(IS_LIBRARY),yes)

   $(info  - Set Parameters for Shared Library build process)

   ALL_PARAMETERS = lib$(PROJECT_NAME).so.$(PROJECT_VERSION) clean

   ALL_TYPE = lib$(PROJECT_NAME).so.$(PROJECT_VERSION): $(O_FILES)
   
   LIBFLAGS = -shared -Wl,-soname,lib$(PROJECT_NAME).so
   
   CFLAGS :=  -fPIC $(CFLAGS)
   
   CXXFLAGS := -fPIC $(CXXFLAGS)
else

   $(info  - Set Parameters for Application build process)

   ALL_PARAMETERS = $(PROJECT_NAME) clean

   ALL_TYPE = $(PROJECT_NAME): $(O_FILES)
   
   LIBFLAGS =

endif

# Build Process

all: $(ALL_PARAMETERS)

$(ALL_TYPE)
	@echo -  [OUTPUT][CXX] $@ @[$(BIN_DIRECTORY)]
	@$(CXX) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LIBFLAGS) -o $(BIN_DIRECTORY)/$@ $^ $(LDLIBS)

%.o: %.c
	@echo -  [CC] $@
	@$(CC) $(CFLAGS) -c $(INCLUDES) -o $@ $< $(LFLAGS)

%.o: %.cpp
	@echo -  [CXX] $@
	@$(CXX) $(CXXFLAGS) -c $(INCLUDES) -o $@ $< $(LFLAGS)
	
# Clear Objects
	
clean:
	$(info  - Remove all .o [object] files)
	@find . -name \*.o -type f -delete
	
	
# Clear Objects & Executables 
	
cleanall:	
	$(info  - Remove all .o [object] files)
	@find . -name \*.o -type f -delete
	$(info  - Remove all files in $(BIN_DIRECTORY))
	@find $(BIN_DIRECTORY) -name \*.* -type f -delete
