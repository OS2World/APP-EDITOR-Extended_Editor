CC= gcc -Zomf

.SUFFIXES: .c .obj

exedit.exe: child.obj main.obj procs.obj search.obj settings.obj status.obj toolbar.obj exedit.res
	$(CC) child.obj main.obj procs.obj search.obj settings.obj status.obj toolbar.obj exedit.def exedit.res -o exedit.exe

clean: 
	del *.obj *.res *.exe

exedit.res: exedit.rc res.h
	rc -r exedit.rc

.c.obj: 
	$(CC) -c $<

