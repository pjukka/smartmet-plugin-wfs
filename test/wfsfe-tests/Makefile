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
	rm -f cnf/observation.conf

test:	../PluginTest input-files cnf/observation.conf stations-cache
	@mkdir -p failures
	@../PluginTest

test-open:	../PluginTest input-files cnf/observation_open.conf stations-cache
	@mkdir -p failures
	@../PluginTest cnf/wfs_plugin_test_open.conf

valgrind: ../PluginTest input-files cnf/observation.conf stations-cache
	@mkdir -p failures
	@$(VALGRIND) ../PluginTest

test-installed:	../PluginTest input-files cnf/observation.conf
	@mkdir -p failures
	@../PluginTest cnf/wfs_plugin_test_installed.conf

test-installed-open:	../PluginTest input-files cnf/observation_open.conf
	@mkdir -p failures
	@../PluginTest cnf/wfs_plugin_test_installed_open.conf

input-files: $(XML_POST_TESTS)

input/%.xml.post : xml/%.xml ; @mkdir -p $(shell dirname $@)
	@perl ../MakeXMLPOST.pl $< $@ /wfs

../PluginTest:
	$(MAKE) -C .. PluginTest

stations-cache:
	$(MAKE) -C .. stations.xml

cnf/observation.conf:	../base/cnf/observation.conf.base Makefile
	awk '{ if (/serializedStationsFile/) { printf("serializedStationsFile = \"../stations.xml\";\n"); } else { print; }}' \
		$< >$@

cnf/observation_open.conf:	../base/cnf/observation.conf.base Makefile
	awk '{ if (/serializedStationsFile/) { printf("serializedStationsFile = \"../stations.xml\";\n"); } else { print; }}' \
		$< >$@
