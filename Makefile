all: fs code

code:
	platformio remote -a pizero2w run --target upload --environment rpipicow -v            

fs: html
	platformio remote run --target uploadfs --environment rpipicow --force-remote

html:
	mustache web/index.yaml web/index.mustache > data/index.html