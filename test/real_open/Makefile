TOP = $(shell pwd)/../../

XML_TEST_SRC = $(shell find xml -name '*.xml')
XML_POST_TESTS = $(subst :,\:,$(patsubst xml/%.xml, input/%.xml.post, $(XML_TEST_SRC)))
TEMPORARY_ENGINE_CONFICS = cnf/observation_open.conf cnf/geonames.conf cnf/contour.conf cnf/gis.conf

foo:
	echo $(XML_TEST_SRC)
	echo $(XML_POST_TESTS)

all: 

clean:
	rm -fv $(shell find input -type f -a -name '*.xml.post')
	rm -rf failures
	rm -f $(TEMPORARY_ENGINE_CONFICS)

test:	../PluginTest input-files $(TEMPORARY_ENGINE_CONFICS) stations-cache
ifeq "$(XML_POST_TESTS)" ""
	@echo "ERROR: Empty XML_POST_TESTS"
else
	@mkdir -p failures
	@../PluginTest cnf/wfs_plugin_test.conf
endif

input-files: $(XML_POST_TESTS)

input/%.xml.post : xml/%.xml ; @mkdir -p $(shell dirname $@)
	@perl ../MakeXMLPOST.pl $< $@ /wfs

../PluginTest:
	$(MAKE) -C .. PluginTest

stations-cache:
	$(MAKE) -C .. stations.xml

cnf/observation_open.conf: ../base/cnf/observation.conf
	awk '{ if (/serializedStationsFile/) { printf("serializedStationsFile = \"../stations.xml\";\n"); } else { print; }}' $< | \
	awk '{ if (/disableUpdates/) { printf("disableUpdates = true;\n"); } else { print; }}' >$@

cnf/geonames.conf:	../../../../engines/geonames/test/cnf/geonames.conf
	awk '{ if (/mock/) { printf("mock = true;\n"); } else { print; }}' $< >$@

cnf/contour.conf:	../../../../engines/contour/test/cnf/contour.conf
	cp -fv $< $@

cnf/gis.conf:	../../../../engines/gis/test/cnf/gis.conf
	awk '{ if (/crsDefinitionDir/) { printf("crsDefinitionDir = \"../../../../engines/gis/cnf/crs\";\n"); } else { print; }}' $< >$@
