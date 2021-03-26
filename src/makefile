GCC_FLAGS = gcc -g -Wall --std=gnu99
H_FILES = densmora.smallshConstants.h densmora.commands.h densmora.smallshBuiltins.h densmora.signalHandlers.h \
densmora.childProcesses.h
C_FILES = densmora.smallshMain.c densmora.commands.c densmora.smallshBuiltins.c densmora.signalHandlers.c \
densmora.childProcesses.c
EXE_FILE = smallsh

smallsh: ${C_FILES} ${H_FILES}
	${GCC_FLAGS} ${C_FILES} -o ${EXE_FILE}

clean:
	rm ${EXE_FILE}

cleanZip:
	rm smallsh.zip

zip:
	zip -D smallsh.zip makefile readme.txt ${H_FILES} ${C_FILES}

runTest:
	p3testscript 1>testresults.txt 2>&1

cleanTest:
	rm -f testresults.txt jun*
	rmdir ~/testdir*
