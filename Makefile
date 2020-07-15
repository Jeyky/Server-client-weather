TARGET := server
TARGET_CLIENT := client
TARGET_CLIENT_GUI := client_gui
CLIENT_DIR := ./client
SERVER_DIR := ./server
CLIENT_DIR_GUI := ./client_gui
INCDIR := /inc
SRCDIR := /src
LIBS = gtk+-3.0
BUILDDIR := .build

SRCS = $(TARGET).c

SRCS_CLIENT = $(TARGET_CLIENT).c
SRCS_CLIENT_GUI = $(TARGET_CLIENT_GUI).c

CFLAGS = -O2 -Wall -Wextra -Wpedantic $(shell pkg-config --cflags $(LIBS))

LDFLAGS = -lcurl -ljson-c -pthread $(shell pkg-config --libs $(LIBS))

CC := gcc

.PHONY: all clean tidy

all: $(TARGET) $(TARGET_CLIENT) $(TARGET_CLIENT_GUI)
###################MAKES server.o & server###################
$(TARGET): $(BUILDDIR)/$(TARGET)
	cp $< $@

$(BUILDDIR)/$(TARGET): $(addprefix $(BUILDDIR)/, $(SRCS:.c=.o))
	$(CC) $^ $(LDFLAGS) $(CFLAGS)  -o $@

$(BUILDDIR)/$(TARGET).o: $(SERVER_DIR)/$(SRCDIR)/$(TARGET).c $(BUILDDIR)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(addprefix -I,$(INCDIR)) -c  -o $@

###################MAKES client.o & client###################
$(TARGET_CLIENT): $(BUILDDIR)/$(TARGET_CLIENT)
	cp $< $@

$(BUILDDIR)/$(TARGET_CLIENT): $(addprefix $(BUILDDIR)/,$(SRCS_CLIENT:.c=.o))
	$(CC) $^ $(LDFLAGS) $(CFLAGS)  -o $@


$(BUILDDIR)/$(TARGET_CLIENT).o: $(CLIENT_DIR)$(SRCDIR)/$(TARGET_CLIENT).c $(BUILDDIR)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(addprefix -I,$(INCDIR)) -c  -o $@

###################MAKES client_gui.o & client_gui###################
$(TARGET_CLIENT_GUI): $(BUILDDIR)/$(TARGET_CLIENT_GUI)
	cp $< $@

$(BUILDDIR)/$(TARGET_CLIENT_GUI): $(addprefix $(BUILDDIR)/,$(SRCS_CLIENT_GUI:.c=.o))
	$(CC) $^ $(LDFLAGS) $(CFLAGS)  -o $@


$(BUILDDIR)/$(TARGET_CLIENT_GUI).o: $(CLIENT_DIR_GUI)$(SRCDIR)/$(TARGET_CLIENT_GUI).c $(BUILDDIR)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(addprefix -I,$(INCDIR)) -c  -o $@


$(BUILDDIR): 
	mkdir -p $@

clean:
	@echo Cleaning up
	-rm -rf $(BUILDDIR)/*.o

tidy: clean
	-rm -f $(BUILDDIR)/$(TARGET)
	-rm -f $(BUILDDIR)/$(TARGET_CLIENT)
	-rm -f $(BUILDDIR)/$(TARGET_CLIENT_GUI)


