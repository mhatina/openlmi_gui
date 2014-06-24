RUN_FILE=ui/run

all:
	make -C logger all
	make -C ui all
	make -C plugin/accountProvider all
	make -C plugin/serviceProvider all

	echo "#!/bin/bash" > $(RUN_FILE)
	echo "LD_LIBRARY_PATH=../logger ./openLMI > /dev/null 2> /dev/null &" >> $(RUN_FILE)
	chmod +x $(RUN_FILE)

clean:
	make -C logger clean
	make -C ui clean
	make -C plugin/accountProvider clean
	make -C plugin/serviceProvider clean
	rm -rf $(RUN_FILE) logger/libLogger.so* plugin/lib*.so ui/openLMI
