CC := gcc
CFLAGS := `pkg-config --cflags gtk4` -Iinclude -Isrc -g
LIBS := `pkg-config --libs gtk4`

TARGET   := main
SRC 	 := src/main.c src/gui.c src/ops.c src/app.c
OBJDIR   := objs
OBJS 	 := $(SRC:src/%.c=$(OBJDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean
