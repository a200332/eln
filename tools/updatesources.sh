#!/bin/sh

SUBDIRS="App ToolScene Dialogs Gui Book Data File Items Scenes AutoNote"

cd `dirname $0`
cd ../src

echo "# Automatically generated by updatesources.sh" > eln.pri
echo "" >> eln.pri
echo "sourcedirs = \\" >> eln.pri

for DIR in $SUBDIRS; do
    echo "    $DIR \\" >> eln.pri

    echo "# Automatically generated by updatesources.sh" > $DIR/$DIR.pri
    echo "" >> $DIR/$DIR.pri
    echo "HEADERS += \\" >> $DIR/$DIR.pri
    find $DIR -name \*.h -exec echo "    " '{}' " \\" ';' | sort -df >> $DIR/$DIR.pri
    echo "" >> $DIR/$DIR.pri

    echo "SOURCES += \\" >> $DIR/$DIR.pri
    find $DIR -name \*.cpp -exec echo "    " '{}' " \\" ';' | sort -df >> $DIR/$DIR.pri
    echo "" >> $DIR/$DIR.pri

    echo "RESOURCES += \\" >> $DIR/$DIR.pri
    find $DIR -name \*.qrc -exec echo "    " '{}' " \\" ';' | sort -df >> $DIR/$DIR.pri
    echo "" >> $DIR/$DIR.pri

    echo "FORMS += \\" >> $DIR/$DIR.pri
    find $DIR -name \*.ui -exec echo "    " '{}' " \\" ';' | sort -df >> $DIR/$DIR.pri
    echo "" >> $DIR/$DIR.pri
done

echo "" >> eln.pri
