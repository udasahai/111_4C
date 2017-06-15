.SILENT:


default:
	gcc -g -lmraa -pthread -std=c99 edison.c -o lab4c_tcp -lm -lssl -lcrypto
	gcc -g -lmraa -pthread -std=c99 edison2.c -o lab4c_tls -lm -lssl -lcrypto
check: default first second third
	rm -rf LOGFILE

first:
	chmod u+x checkScript
	./checkScript
	if [[ $$? -eq 0 ]]; then \
	echo "Test passed...runs and talks to sensors." ; \
	else \
	echo "FAILED ...stdin --> stdout" ;\
	fi
	

second:
	./lab4b --period=b &> /dev/null;\
	if [[ $$? -eq 1 ]]; then \
	echo "Test passed ...detects non number argument to period" ;\
	else \
	echo "FAILED ...Does not detect non-number argument" ;\
	fi

third:
	./lab4b --scale=z  &> /dev/null; \
	if [[ $$? -eq 1 ]]; then \
	echo "Test passed ...detects bad argument to scale" ;\
	else \
	echo "FAILED ...doesnt detect bad argument to period." ;\
	fi


dist: default 
	tar -cvzf lab4c-504646937.tar.gz edison.c edison2.c README Makefile

clean: 
	rm -rf *.tar.gz lab4c_*
