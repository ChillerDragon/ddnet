#!/bin/bash

backup_file=''
if [ "$(uname)" = Darwin ]
then
	backup_file="''"
fi

find . \
        -type f \
        \( -name "*.cpp" -o -name "*.h" -o -name "*.py"  \) \
        \( \
                -path "./src/*" -o \
                -path "./datasrc/*" -o \
                -path "./scripts/*" \
        \) \
        -print | while IFS=$'\n' read -r src_file
do
        sed -i $backup_file \
        -e 's/\([a-z]_\?\)ID/\1Id/g' \
        -e 's/\([^ ]\)\<UI\>/\1Ui/g' \
        -e 's/UI()/Ui()/g' \
        -e 's/\<CUI\>/CUi/g' \
        -e 's/\([\ta-z.(&]\|[,=|] \)ID\>/\1Id/g' \
        -e 's/\<ID\>\([^ ").]\)/Id\1/g' \
        -e 's/\<ID\([0-9]\)/Id\1/g' \
        -e 's/\<ID\>\( [<=>:+*/-]\)/Id\1/g' \
        -e 's/int ID/int Id/g' \
        -e 's/\([a-z]_\?\)GPU/\1Gpu/g' \
        -e 's/\([a-z]_\?\)IP/\1Ip/g' \
        -e 's/\([a-z]_\?\)CID/\1Cid/g' \
        -e 's/\([a-z]_\?\)MySQL/\1Mysql/g' \
        -e 's/MySql/Mysql/g' \
        -e 's/\([a-xz]_\?\)SQL/\1Sql/g' \
        -e 's/DPMode/DpMode/g' \
        -e 's/TTWGraphics/TTwGraphics/g' \
        \
        -e 's/Ipointer/IPointer/g' \
        -e 's/\.vendorId/.vendorID/g' \
        -e 's/\.windowId/.windowID/g' \
        -e 's/SDL_GetWindowFromId/SDL_GetWindowFromID/g' \
        -e 's/SDL_AudioDeviceId/SDL_AudioDeviceID/g' \
        -e 's/SDL_JoystickId/SDL_JoystickID/g' \
        -e 's/SDL_JoystickInstanceId/SDL_JoystickInstanceID/g' \
        -e 's/AVCodecId/AVCodecID/g' \
	"$src_file"
done

