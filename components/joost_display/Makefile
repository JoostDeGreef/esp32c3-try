
all: font_data.cpp font_data.h Makefile
	
font_data.cpp font_data.h: tmp Makefile font_conversion.py
	python3 font_conversion.py
	
clean:
	rm -rf *~ tmp font_data.*

tmp: data/font2bitmap.zip Makefile
	mkdir -p tmp
	unzip -d tmp -o data/font2bitmap.zip
