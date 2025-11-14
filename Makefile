OUT := out
SRC := src
BIN := main
SOURCES := $(SRC)/*.c

.PHONY: clean always build
always:
	mkdir -p $(OUT)
clean:
	rm -r $(OUT)
build: always $(OUT)/$(BIN)

$(OUT)/$(BIN): $(SOURCES)
	$(CC) -o $@ $^
