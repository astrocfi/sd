REM This tests 24 combinations of color.  The appearance should be as follows:
REM
REM   text   background    person glyphs
REM
REM    gry      blk         RB
REM    gry      blk         gry
REM    gry      blk         RGBY
REM    gry      blk         CM
REM    gry      blk         gry
REM    gry      blk         RGBY
REM
REM    wht      blk         RB
REM    wht      blk         wht
REM    wht      blk         RGBY
REM    wht      blk         CM
REM    wht      blk         wht
REM    wht      blk         RGBY
REM
REM    blk      gry         RB
REM    blk      gry         blk
REM    blk      gry         RGBY
REM    blk      gry         CM
REM    blk      gry         blk
REM    blk      gry         RGBY
REM
REM    blk      wht         RB
REM    blk      wht         blk
REM    blk      wht         RGB dark yel
REM    blk      wht         CM
REM    blk      wht         blk
REM    blk      wht         RGB dark yel

sdtty m -no_cursor -reverse_video -no_intensify -bold_color
sdtty m -no_cursor -reverse_video -no_intensify -bold_color   -no_color
sdtty m -no_cursor -reverse_video -no_intensify -bold_color   -color_by_corner
sdtty m -no_cursor -reverse_video -no_intensify -pastel_color
sdtty m -no_cursor -reverse_video -no_intensify -pastel_color -no_color
sdtty m -no_cursor -reverse_video -no_intensify -pastel_color -color_by_corner

sdtty m -no_cursor -reverse_video               -bold_color
sdtty m -no_cursor -reverse_video               -bold_color   -no_color
sdtty m -no_cursor -reverse_video               -bold_color   -color_by_corner
sdtty m -no_cursor -reverse_video               -pastel_color
sdtty m -no_cursor -reverse_video               -pastel_color -no_color
sdtty m -no_cursor -reverse_video               -pastel_color -color_by_corner

sdtty m -no_cursor -normal_video  -no_intensify -bold_color
sdtty m -no_cursor -normal_video  -no_intensify -bold_color   -no_color
sdtty m -no_cursor -normal_video  -no_intensify -bold_color   -color_by_corner
sdtty m -no_cursor -normal_video  -no_intensify -pastel_color
sdtty m -no_cursor -normal_video  -no_intensify -pastel_color -no_color
sdtty m -no_cursor -normal_video  -no_intensify -pastel_color -color_by_corner

sdtty m -no_cursor -normal_video                -bold_color
sdtty m -no_cursor -normal_video                -bold_color   -no_color
sdtty m -no_cursor -normal_video                -bold_color   -color_by_corner
sdtty m -no_cursor -normal_video                -pastel_color
sdtty m -no_cursor -normal_video                -pastel_color -no_color
sdtty m -no_cursor -normal_video                -pastel_color -color_by_corner
