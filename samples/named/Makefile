
source := $(wildcard *.la16)
files=$(source:.la16=.mixal)

.phony: build
build: $(files)
	
%.mixal: %.la16
	-../../compiler $< $@

.phony: clean
clean:
	rm -f *.mixal
