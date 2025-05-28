    ELEMENT sdsi.c @the_libe =
         DEPENDS_SOURCE
                '/usr/include/stdio.h';
                '/usr/include/stdlib.h';
                '/usr/include/time.h';
                paths.h @ the_libe;
    END OF sdsi.c;

    ELEMENT sdmain.c @the_libe =
         DEPENDS_SOURCE
                paths.h @ the_libe;
    END OF sdmain.c;

    ELEMENT sdinit.c @the_libe =
    END OF sdinit.c;

    ELEMENT sdgetout.c @the_libe =
    END OF sdgetout.c;

    ELEMENT sdutil.c @the_libe =
    END OF sdutil.c;

    ELEMENT sdtables.c @the_libe =
    END OF sdtables.c;

    ELEMENT sdctable.c @the_libe =
    END OF sdctable.c;

    ELEMENT sdpreds.c @the_libe =
    END OF sdpreds.c;

    ELEMENT sdconcpt.c @the_libe =
    END OF sdconcpt.c;

    ELEMENT sdmoves.c @the_libe =
    END OF sdmoves.c;

    ELEMENT sdbasic.c @the_libe =
    END OF sdbasic.c;

    ELEMENT sdtand.c @the_libe =
    END OF sdtand.c;

    ELEMENT sdconc.c @the_libe =
    END OF sdconc.c;

    ELEMENT sd12.c @the_libe =
    END OF sd12.c;

    ELEMENT sd16.c @the_libe =
    END OF sd16.c;

    ELEMENT sdistort.c @the_libe =
    END OF sdistort.c;

    ELEMENT sdtop.c @the_libe =
    END OF sdtop.c;
