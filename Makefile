view: viewf viewb

viewf: difference/front.jpg
	gwenview $<

viewb: difference/back.jpg
	gwenview $<

difference/%.jpg: drawings/%.jpg
	composite $< ./photo/$*.jpg -compose difference  ./difference/$*.jpg || true

drawings/front.jpg drawings/back.jpg: drawings.kibot.yaml
	kibot -c ./drawings.kibot.yaml
