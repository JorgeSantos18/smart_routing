CROSS_COMPILE ?=

all: smart_routing

smart_routing: smart_routing.c
	$(CROSS_COMPILE)gcc -o $@ $^

smart_routing_tests: smart_routing_tests.c smart_routing.c
	$(CROSS_COMPILE)gcc -DTEST_MODE -I. -o $@ $^

test: smart_routing_tests
	./smart_routing_tests

clean:
	rm -rf smart_routing_tests

.PHONY: all clean