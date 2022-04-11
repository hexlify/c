build: myinit.c
	@gcc myinit.c -o myinit

clean:
	@echo Cleaning...
	rm -f /tmp/program1.sh
	rm -f /tmp/program2.sh
	rm -f /tmp/program3.sh
	rm -f /tmp/myinit.log
	rm -rf /tmp/program_in
	rm -rf /tmp/program_out