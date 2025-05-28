REM This uses index1.htmli, index2.htmli, and the poster as source files.
del index.msg index.txt index.zip index.asc
copy index1.htmli index.html
type poster%1.htmli >> index.html
type index2.htmli >> index.html
