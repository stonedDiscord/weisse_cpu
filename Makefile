view: viewf viewb

viewf: difference/front.jpg
	gwenview $<

viewb: difference/back.jpg
	gwenview $<

difference/%.jpg: drawings/%-drawn.jpg
	composite $< ./photo/$*.jpg -compose difference  ./difference/$*.jpg || true

drawings/front-drawn.jpg drawings/back-drawn.jpg: weisse_cpu.kicad_pcb drawings.kibot.yaml
	kibot -c ./drawings.kibot.yaml
