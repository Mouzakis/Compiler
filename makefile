all :	calc

calc : al.c parser.c  symboltable.c quads.c terminal.c 
	gcc -o calc al.c parser.c symboltable.c quads.c terminal.c 

al.c : al.l
	flex --outfile=al.c al.l

parser.c :	parser.y
	bison --yacc --defines --output=parser.c parser.y

clean:	calc al.c parser.c parser.h 
	rm -f al.c calc parser.c parser.h 

vm : virtual_machine.c 
	gcc -o vm virtual_machine.c