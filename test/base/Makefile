TOP = $(shell pwd)/../../

XML_TEST_SRC = $(shell find xml -name '*.xml')
XML_POST_TESTS = $(subst :,\:,$(patsubst xml/%.xml, input/%.xml.post, $(XML_TEST_SRC)))
TEMPORARY_ENGINE_CONFICS = cnf/observation.conf cnf/geonames.conf cnf/contour.conf cnf/gis.conf

VALGRIND = valgrind \
	--num-callers=30 \
	--suppressions=../../../../data/valgrind/valgrind_libclntsh.supp \
	--suppressions=../../../../data/valgrind/valgrind_brainstorm.supp

all:

clean:
	rm -f $(shell find input -type f -a -name '*.xml.post')
	rm -rf failures
	rm -f $(TEMPORARY_ENGINE_CONFICS)

test:	../PluginTest $(XML_POST_TESTS) $(TEMPORARY_ENGINE_CONFICS)
	@mkdir -p failures
	@../PluginTest

valgrind: ../PluginTest input-files $(TEMPORARY_ENGINE_CONFICS)
	@mkdir -p failures
	@$(VALGRIND) ../PluginTest

input-files: $(XML_POST_TESTS)

input/%.xml.post : xml/%.xml ; @mkdir -p $(shell dirname $@)
	@perl ../MakeXMLPOST.pl $< $@ /wfs

../PluginTest:
	$(MAKE) -C .. PluginTest

cnf/observation.conf:	cnf/observation.conf.base
	awk '{ if (/serializedStationsFile/) { printf("serializedStationsFile = \"../../../../data/stations/stations.xml\";\n"); } else { print; }}' $< | \
	awk '{ if (/spatialiteFile/) { printf("spatialiteFile = \"../../../../data/stations/stations.sqlite\";\n"); } else { print; }}' | \
	awk '{ if (/disableUpdates/) { printf("disableUpdates = true;\n"); } else { print; }}' >$@

cnf/geonames.conf:	../../../../engines/geonames/test/cnf/geonames.conf
	awk '{ if (/mock/) { printf("mock = true;\n"); } else { print; }}' $< >$@

cnf/contour.conf:	../../../../engines/contour/test/cnf/contour.conf
	cp -fv $< $@

cnf/gis.conf:	../../../../engines/gis/test/cnf/gis.conf
	awk '{ if (/crsDefinitionDir/) { printf("crsDefinitionDir = \"../../../../engines/gis/cnf/crs\";\n"); } else { print; }}' $< >$@
