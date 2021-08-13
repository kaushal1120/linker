IDIR=../include
CC=g++-9.2
CPPFLAGS=-I$(IDIR)

ODIR=obj

_DEPS=tokenizer.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=linker.o tokenizer.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ)) 

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

linker: $(OBJ) 
	$(CC) -o $@ $^ $(CPPFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~