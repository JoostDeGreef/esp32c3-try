
all: font_data.cpp font_data.h Makefile
	
font_data.cpp font_data.h: tmp Makefile tool.py
	python3 tool.py
	
clean:
	rm -rf *~ tmp *.cpp *.h

tmp: font2bitmap.zip Makefile
	mkdir -p tmp
	unzip -d tmp -o font2bitmap.zip
