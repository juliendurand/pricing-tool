pandoc --template template.latex -s -o book.pdf chap*.md && open book.pdf
pandoc -s --reference-doc=reference.docx -o book.docx chap*.md
