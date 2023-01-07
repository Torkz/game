@echo off

SET shared_compiler_flags=-MT -nologo -GR- -Oi -WX -W4 -wd4100 -wd4505 -wd4189 -DGAME_SLOWMODE -DGAME_INTERNAL -FC -Z7
SET shared_linker_flags=-opt:ref
SET libs=user32.lib gdi32.lib
SET game_pdb_name=game_%date%_%time:~0,2%%time:~3,2%-%time:~6,2%%time:~9,2%.pdb

IF NOT EXIST build mkdir build
pushd build

del *.pdb

cl /LD %shared_compiler_flags% %additional_compiler_flags% -Fmgame.map ../code/game.cpp /link %shared_linker_flags% /PDB:%game_pdb_name% /EXPORT:update_and_render /EXPORT:fill_audio_output
cl %shared_compiler_flags% %additional_compiler_flags% -Fmwin_game.map ../code/win_main.cpp /link %shared_linker_flags% %libs%
popd