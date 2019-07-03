CC=gcc
STD=-std=gnu99
FOLDERTINY=tinySTM
LIBTINY = lib/libstm.a

DEFINES += -D_GNU_SOURCE

arraySum: arraySum.c
	#compila a lib da tinySTM
	cd $(FOLDERTINY) && $(MAKE) $(LIBTINY)
        #compila o projeto usando a lib da tiny compilada
	$(CC) -g -O0 $(DEFINES) arraySum.c $(STD) -o arraySum.out -lpthread $(FOLDERTINY)/$(LIBTINY)
#	clear
#	@echo -----------------------------------------
#	@echo Compilado. Utilize make run para executar

clean:
	cd $(FOLDERTINY) && $(MAKE) clean
	rm -f *.out
#	@echo -----------------------------------------
#	@echo Arquivos compilados apagados.

run: 
	./arraySum.out
