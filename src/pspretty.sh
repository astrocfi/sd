#! /bin/sh
# Time-stamp: <93/06/18 19:19:17 gildea>
# pspretty.sh - pretty up the PS file DSC comments:
# better title
# Unix date to RFC 822
# add %%For line
# move %%Creator line down, since it is less important
# nicer page labels for front matter
sed -e '/^%%Creator: /h' \
    -e '/^%%Creator: /d' \
    -e '/^%%Title: /s/sd_doc.*\.dvi/Sd/' \
    -e '/^%%CreationDate: /s/[A-Z][a-z]* \([A-Z][a-z]*\) \([ 0-9][0-9]\) \([:0-9]*\) \([0-9]*\)/\2 \1 \4 \3/' \
    -e '/^%%CreationDate: /a\
%%For: '"$LOGNAME"\
    -e '/^%%CreationDate: /G' \
    -e '/^%%Page: 1 1$/s/1 /title /' \
    -e '/^%%Page: 1 2$/s/1 /copyr /' \
    -e '/^%%Page: 2 2$/s/2 /copyr /' \
    -e '/^%%Page: -1 /s/-1/i/' \
    -e '/^%%Page: -2 /s/-2/ii/' \
    -e '/^%%Page: -3 /s/-3/iii/' \
    -e '/^%%Page: -4 /s/-4/iv/' \
  "$@"
