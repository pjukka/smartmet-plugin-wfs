TOP = $(shell pwd)/../../

XML_TEST_SRC = $(shell find xml -name '*.xml')
XML_POST_TESTS = $(subst :,\:,$(patsubst xml/%.xml, input/%.xml.post, $(XML_TEST_SRC)))

VALGRIND = valgrind \
	--num-callers=30 \
	--gen-suppressions=all \
	--suppressions=../../../../data/valgrind/valgrind_libclntsh.supp \
	--suppressions=../../../../data/valgrind/valgrind_brainstorm.supp

foo:
	echo $(XML_TEST_SRC)
	echo $(XML_POST_TESTS)

all:

clean:
	rm -fv $(shell find input -type f -a -name '*.xml.post')
	rm -rf failures

test:	../PluginTest input-files
	@mkdir -p failures
	@../PluginTest

valgrind: ../PluginTest input-files
	@mkdir -p failures
	@$(VALGRIND) ../PluginTest

input-files: $(XML_POST_TESTS)

input/%.xml.post : xml/%.xml ; @mkdir -p $(shell dirname $@)
	@perl ../MakeXMLPOST.pl $< $@ /wfs

../PluginTest:
	$(MAKE) -C .. PluginTest
