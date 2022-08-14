TARGET=Undiscovered_Worlds
BUILD=obj
VPATH=$(BUILD) ../
INCLUDES=-I../ -I../../imgui -I../../imgui-sfml -I../../ImGuiFileDialog -I../../stb
CXXFLAGS=-g -Wall -O3 -fopenmp $(INCLUDES) -Wno-unused-parameter -Wno-implicit-fallthrough -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unknown-pragmas -std=c++17
OBJS=$(shell cat obj_list)

LINK=$(CPP) $(INCLUDES)
LDFLAGS=-lglfw -lsfml-window -lsfml-system -lsfml-graphics -lGL

# For creating dependencies files
DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD)/$*.Td
POSTCOMPILE = mv -f $(BUILD)/$*.Td $(BUILD)/$*.d

# Change the verbosity of the makefile.
ifeq ($(VERBOSE),1)
Q :=
else
Q := @
endif

# Compile
all: $(TARGET)

# Link the target
$(TARGET): $(OBJS)
	@echo "Linking $<"
	$(Q)cd $(BUILD) && $(CXX) $(INCLUDES) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files
%.o : %.cpp $(BUILD)/%.d
	@echo "Compiling $<"
	$(Q)$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $(abspath $<) -o $(abspath $(BUILD)/$@)
	@$(POSTCOMPILE)

# Delete compiled files
.PHONY: clean
clean:
	-rm -fr $(BUILD)

# Create the directory before compiling sources
$(OBJS): | $(BUILD)
$(BUILD):
	@mkdir -p $(BUILD)

# Create the dependency files
$(BUILD)/%.d: ;
.PRECIOUS: $(BUILD)/%.d

-include $(patsubst %,$(BUILD)/%.d,$(basename $(OBJS)))
