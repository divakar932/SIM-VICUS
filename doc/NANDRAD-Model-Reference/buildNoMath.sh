#!/bin/bash

ADOC=NANDRAD-Model-Reference

echo '*** Generating html ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py html . &&
asciidoctor -a lang=en $ADOC.adoc &&

echo &&
echo '*** Generating pdf ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py pdf . &&
asciidoctor-pdf -a lang=en  -a pdf-theme=../adoc_utils/pdf-theme.yml  -r ../adoc_utils/rouge_theme.rb -a pdf-fontsdir="../adoc_utils/fonts;GEM_FONTS_DIR" $ADOC.adoc &&

# restore html-type image files
echo &&
echo '*** Restoring html ***' &&
python ../adoc_utils/scripts/adoc-image-prep.py html . &&

echo &&
echo '*** Copying files to ../../docs directory' &&

if [ ! -d "../../docs/$ADOC" ]; then
	mkdir ../../docs/$ADOC
fi &&
mv $ADOC.html ../../docs/$ADOC/index.html &&


imgFiles=(./images/*.png) &&
if [ ${#imgFiles[@]} -gt 0 ]; then
	echo 'Copying '${#imgFiles[@]}' images to ../../docs/'$ADOC'/images' &&
	cp -r ./images/*.png ../../docs/$ADOC/images
fi

