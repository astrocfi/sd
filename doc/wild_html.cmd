ren %1_toc.html %1_toc.htmlsave
for %%f in (%1_*.html) do call make_text %%f %2
ren %1_toc.htmlsave %1_toc.html
